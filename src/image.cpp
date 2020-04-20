/*
 * chopt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020 Raymond Wright
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <array>
#include <cmath>

#include "image.hpp"

using namespace cimg_library;

constexpr int BEAT_WIDTH = 60;
constexpr int LEFT_MARGIN = 31;
constexpr int MARGIN = 32;
constexpr int MAX_BEATS_PER_LINE = 16;
constexpr int MEASURE_HEIGHT = 61;
constexpr float OPEN_NOTE_OPACITY = 0.5F;
constexpr int DIST_BETWEEN_MEASURES = MEASURE_HEIGHT + MARGIN;

static double get_beat_rate(const SyncTrack& sync_track, int resolution,
                            double beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return BASE_BEAT_RATE;
    }
    --ts;
    return BASE_BEAT_RATE * ts->numerator / ts->denominator;
}

static int get_numer(const SyncTrack& sync_track, int resolution, double beat)
{
    constexpr int BASE_NUMERATOR = 4;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return BASE_NUMERATOR;
    }
    --ts;
    return ts->numerator;
}

static double get_denom(const SyncTrack& sync_track, int resolution,
                        double beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return 1.0;
    }
    --ts;
    return BASE_BEAT_RATE / ts->denominator;
}

static std::vector<DrawnNote> drawn_notes(const NoteTrack& track,
                                          int resolution)
{
    std::vector<DrawnNote> notes;

    for (const auto& note : track.notes()) {
        auto beat = note.position / static_cast<double>(resolution);
        auto length = note.length / static_cast<double>(resolution);
        auto is_sp_note = false;
        for (const auto& phrase : track.sp_phrases()) {
            if (note.position >= phrase.position
                && note.position < phrase.position + phrase.length) {
                is_sp_note = true;
            }
        }
        notes.push_back({beat, length, note.colour, is_sp_note});
    }

    return notes;
}

static std::vector<DrawnRow> drawn_rows(const NoteTrack& track, int resolution,
                                        const SyncTrack& sync_track)
{
    int max_pos = 0;
    for (const auto& note : track.notes()) {
        auto length = note.position + note.length;
        if (length > max_pos) {
            max_pos = length;
        }
    }

    const auto max_beat = max_pos / static_cast<double>(resolution);
    auto current_beat = 0.0;
    std::vector<DrawnRow> rows;

    while (current_beat <= max_beat) {
        auto row_length = 0.0;
        while (true) {
            auto contribution = get_beat_rate(sync_track, resolution,
                                              current_beat + row_length);
            if (contribution > MAX_BEATS_PER_LINE && row_length == 0.0) {
                // Break up a measure that spans more than a full row.
                while (contribution > MAX_BEATS_PER_LINE) {
                    rows.push_back(
                        {current_beat, current_beat + MAX_BEATS_PER_LINE});
                    current_beat += MAX_BEATS_PER_LINE;
                    contribution -= MAX_BEATS_PER_LINE;
                }
            }
            if (contribution + row_length > MAX_BEATS_PER_LINE) {
                break;
            }
            row_length += contribution;
            if (current_beat + row_length > max_beat) {
                break;
            }
        }
        rows.push_back({current_beat, current_beat + row_length});
        current_beat += row_length;
    }

    return rows;
}

DrawingInstructions create_instructions(const NoteTrack& track, int resolution,
                                        const SyncTrack& sync_track)
{
    auto notes = drawn_notes(track, resolution);
    auto rows = drawn_rows(track, resolution, sync_track);

    std::vector<double> half_beat_lines;
    std::vector<double> beat_lines;
    std::vector<double> measure_lines;
    constexpr double HALF_BEAT = 0.5;
    for (const auto& row : rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length = get_beat_rate(sync_track, resolution, start);
            auto numer = get_numer(sync_track, resolution, start);
            auto denom = get_denom(sync_track, resolution, start);
            measure_lines.push_back(start);
            half_beat_lines.push_back(start + HALF_BEAT * denom);
            for (int i = 1; i < numer; ++i) {
                beat_lines.push_back(start + i * denom);
                half_beat_lines.push_back(start + (i + HALF_BEAT) * denom);
            }
            start += meas_length;
        }
    }

    std::vector<std::tuple<double, double>> green_ranges;
    for (const auto& phrase : track.sp_phrases()) {
        auto start = phrase.position / static_cast<double>(resolution);
        auto end = (phrase.position + phrase.length)
            / static_cast<double>(resolution);
        green_ranges.emplace_back(start, end);
    }

    return {rows,  half_beat_lines, beat_lines, measure_lines,
            notes, green_ranges,    {}};
}

static void draw_vertical_lines(Image& image,
                                const DrawingInstructions& instructions,
                                const std::vector<double>& positions,
                                const std::array<unsigned char, 3> colour)
{
    for (auto pos : positions) {
        auto row
            = std::find_if(instructions.rows.cbegin(), instructions.rows.cend(),
                           [=](const auto x) { return x.end > pos; });
        auto x
            = LEFT_MARGIN + static_cast<int>(BEAT_WIDTH * (pos - row->start));
        auto y = MARGIN
            + DIST_BETWEEN_MEASURES
                * static_cast<int>(
                    std::distance(instructions.rows.cbegin(), row));
        image.draw_line(x, y, x, y + MEASURE_HEIGHT, colour.data());
    }
}

static void draw_measures(Image& image, const DrawingInstructions& instructions)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr std::array<unsigned char, 3> grey {160, 160, 160};
    constexpr std::array<unsigned char, 3> light_grey {224, 224, 224};
    constexpr int COLOUR_DISTANCE = 15;

    draw_vertical_lines(image, instructions, instructions.beat_lines, grey);
    draw_vertical_lines(image, instructions, instructions.half_beat_lines,
                        light_grey);

    auto current_row = 0;
    for (const auto& row : instructions.rows) {
        auto y = DIST_BETWEEN_MEASURES * current_row + MARGIN;
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (row.end - row.start));
        for (int i = 1; i < 4; ++i) {
            image.draw_line(LEFT_MARGIN, y + COLOUR_DISTANCE * i, x_max,
                            y + COLOUR_DISTANCE * i, grey.data());
        }
        image.draw_rectangle(LEFT_MARGIN, y, x_max, y + MEASURE_HEIGHT,
                             black.data(), 1.0, ~0U);
        ++current_row;
    }

    // We do measure lines after the boxes because we want the measure lines to
    // lie over the horizontal grey fretboard lines.
    draw_vertical_lines(image, instructions, instructions.measure_lines, black);
}

static std::array<unsigned char, 3> note_colour_to_colour(NoteColour colour)
{
    constexpr std::array<unsigned char, 3> GREEN {0, 255, 0};
    constexpr std::array<unsigned char, 3> RED {255, 0, 0};
    constexpr std::array<unsigned char, 3> YELLOW {255, 255, 0};
    constexpr std::array<unsigned char, 3> BLUE {0, 0, 255};
    constexpr std::array<unsigned char, 3> ORANGE {255, 165, 0};
    constexpr std::array<unsigned char, 3> PURPLE {128, 0, 128};

    switch (colour) {
    case NoteColour::Green:
        return GREEN;
    case NoteColour::Red:
        return RED;
    case NoteColour::Yellow:
        return YELLOW;
    case NoteColour::Blue:
        return BLUE;
    case NoteColour::Orange:
        return ORANGE;
    case NoteColour::Open:
        return PURPLE;
    }

    throw std::invalid_argument("Invalid colour to note_colour_to_colour");
}

static int note_colour_to_offset(NoteColour colour)
{
    constexpr int GREEN_OFFSET = 0;
    constexpr int RED_OFFSET = 15;
    constexpr int YELLOW_OFFSET = 30;
    constexpr int BLUE_OFFSET = 45;
    constexpr int ORANGE_OFFSET = 60;

    switch (colour) {
    case NoteColour::Green:
        return GREEN_OFFSET;
    case NoteColour::Red:
        return RED_OFFSET;
    case NoteColour::Yellow:
        return YELLOW_OFFSET;
    case NoteColour::Blue:
        return BLUE_OFFSET;
    case NoteColour::Orange:
        return ORANGE_OFFSET;
    case NoteColour::Open:
        return YELLOW_OFFSET;
    }

    throw std::invalid_argument("Invalid colour to note_colour_to_offset");
}

static void draw_note_sustain(Image& image,
                              const DrawingInstructions& instructions,
                              const DrawnNote& note)
{
    auto colour = note_colour_to_colour(note.colour);
    auto offset = note_colour_to_offset(note.colour);

    auto start = note.beat;
    const auto end = note.beat + note.length;
    auto row_iter
        = std::find_if(instructions.rows.cbegin(), instructions.rows.cend(),
                       [&](const auto& r) { return r.end > note.beat; });
    auto row
        = static_cast<int>(std::distance(instructions.rows.cbegin(), row_iter));

    while (start < end) {
        auto sust_end = std::min(row_iter->end, end);
        auto x_min = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (start - row_iter->start));
        // -1 is so sustains that cross rows do not go over the ending line of a
        // row.
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (sust_end - row_iter->start)) - 1;
        if (x_min <= x_max) {
            auto y = MARGIN + DIST_BETWEEN_MEASURES * row + offset;
            if (note.colour == NoteColour::Open) {
                constexpr int OPEN_SUST_RADIUS = 23;
                image.draw_rectangle(x_min, y - OPEN_SUST_RADIUS, x_max,
                                     y + OPEN_SUST_RADIUS, colour.data(),
                                     OPEN_NOTE_OPACITY);
            } else {
                image.draw_rectangle(x_min, y - 3, x_max, y + 3, colour.data());
            }
        }
        start = sust_end;
        ++row_iter;
        ++row;
    }
}

static void draw_note_circle(Image& image, int x, int y, NoteColour note_colour)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr int RADIUS = 5;

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    if (note_colour == NoteColour::Open) {
        image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                             colour.data(), OPEN_NOTE_OPACITY);
        image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                             black.data(), 1.0, ~0U);
    } else {
        image.draw_circle(x, y + offset, RADIUS, colour.data());
        image.draw_circle(x, y + offset, RADIUS, black.data(), 1.0, ~0U);
    }
}

static void draw_note_star(Image& image, int x, int y, NoteColour note_colour)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    constexpr unsigned int POINTS_IN_STAR_POLYGON = 10;
    auto points = CImg<int>(POINTS_IN_STAR_POLYGON, 2);
    constexpr std::array<int, 20> coords {0, -6, 1, -2, 5, -2, 2,  1,  3, 5, 0,
                                          2, -3, 5, -2, 1, -5, -2, -1, -2};
    for (auto i = 0U; i < POINTS_IN_STAR_POLYGON; ++i) {
        points(i, 0) = coords[2 * i] + x; // NOLINT
        points(i, 1) = coords[2 * i + 1] + y + offset; // NOLINT
    }

    if (note_colour == NoteColour::Open) {
        image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                             colour.data(), OPEN_NOTE_OPACITY);
        image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                             black.data(), 1.0, ~0U);
    } else {
        image.draw_polygon(points, colour.data());
        image.draw_polygon(points, black.data(), 1.0, ~0U);
    }
}

static void draw_note(Image& image, const DrawingInstructions& instructions,
                      const DrawnNote& note)
{
    if (note.length > 0.0) {
        draw_note_sustain(image, instructions, note);
    }

    auto row_iter
        = std::find_if(instructions.rows.cbegin(), instructions.rows.cend(),
                       [&](const auto& r) { return r.end > note.beat; });
    auto row
        = static_cast<int>(std::distance(instructions.rows.cbegin(), row_iter));
    auto beats_along_row = note.beat - row_iter->start;
    auto x = LEFT_MARGIN + static_cast<int>(beats_along_row * BEAT_WIDTH);
    auto y = MARGIN + DIST_BETWEEN_MEASURES * row;
    if (note.is_sp_note) {
        draw_note_star(image, x, y, note.colour);
    } else {
        draw_note_circle(image, x, y, note.colour);
    }
}

static void draw_range(Image& image, const DrawingInstructions& instructions,
                       std::array<unsigned char, 3> colour,
                       std::tuple<double, double> range)
{
    constexpr float RANGE_OPACITY = 0.25F;

    auto start = std::get<0>(range);
    auto end = std::get<1>(range);
    auto row_iter
        = std::find_if(instructions.rows.cbegin(), instructions.rows.cend(),
                       [=](const auto& r) { return r.end > start; });
    auto row
        = static_cast<int>(std::distance(instructions.rows.cbegin(), row_iter));

    while (start < end) {
        auto block_end = std::min(row_iter->end, end);
        auto x_min = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (start - row_iter->start));
        // -1 is so blocks that cross rows do not go over the ending line of a
        // row.
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (block_end - row_iter->start)) - 1;
        if (x_min <= x_max) {
            auto y = MARGIN + DIST_BETWEEN_MEASURES * row;
            image.draw_rectangle(x_min, y, x_max, y + MEASURE_HEIGHT,
                                 colour.data(), RANGE_OPACITY);
        }
        start = block_end;
        ++row_iter;
        ++row;
    }
}

Image create_path_image(const DrawingInstructions& instructions)
{
    constexpr std::array<unsigned char, 3> green {0, 255, 0};
    constexpr std::array<unsigned char, 3> blue {0, 0, 255};

    constexpr unsigned int IMAGE_WIDTH = 1024;
    constexpr unsigned char WHITE = 255;

    auto height = static_cast<unsigned int>(
        MARGIN + DIST_BETWEEN_MEASURES * instructions.rows.size());

    CImg<unsigned char> image(IMAGE_WIDTH, height, 1, 3, WHITE);
    draw_measures(image, instructions);

    for (const auto& note : instructions.notes) {
        draw_note(image, instructions, note);
    }

    for (const auto& range : instructions.green_ranges) {
        draw_range(image, instructions, green, range);
    }
    for (const auto& range : instructions.blue_ranges) {
        draw_range(image, instructions, blue, range);
    }

    return image;
}
