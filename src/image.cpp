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
constexpr int DIST_BETWEEN_MEASURES = MEASURE_HEIGHT + MARGIN;

static double get_beat_rate(const SyncTrack& sync_track,
                            std::int32_t resolution, double beat)
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

DrawingInstructions create_instructions(const NoteTrack& track,
                                        std::int32_t resolution,
                                        const SyncTrack& sync_track)
{
    std::vector<DrawnNote> notes;
    int max_pos = 0;

    for (const auto& note : track.notes()) {
        auto beat = note.position / static_cast<double>(resolution);
        notes.push_back({beat, note.colour});
        max_pos = std::max(max_pos, static_cast<int>(note.position));
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

    return {rows, notes};
}

static void draw_measures(Image& image, const DrawingInstructions& instructions)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};

    auto current_row = 0;
    for (const auto& row : instructions.rows) {
        auto y = DIST_BETWEEN_MEASURES * current_row + MARGIN;
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (row.end - row.start));
        image.draw_rectangle(LEFT_MARGIN, y, x_max, y + MEASURE_HEIGHT,
                             black.data(), 1.0, ~0U);
        ++current_row;
    }
}

static void draw_note(Image& image, int x, int y, NoteColour colour)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr std::array<unsigned char, 3> green {0, 255, 0};
    constexpr std::array<unsigned char, 3> red {255, 0, 0};
    constexpr std::array<unsigned char, 3> yellow {255, 255, 0};
    constexpr std::array<unsigned char, 3> blue {0, 0, 255};
    constexpr std::array<unsigned char, 3> orange {255, 165, 0};
    constexpr std::array<unsigned char, 3> purple {128, 0, 128};
    constexpr int RADIUS = 5;
    constexpr int RED_OFFSET = 15;
    constexpr int YELLOW_OFFSET = 30;
    constexpr int BLUE_OFFSET = 45;
    constexpr int ORANGE_OFFSET = 60;

    switch (colour) {
    case NoteColour::Green:
        image.draw_circle(x, y, RADIUS, green.data());
        image.draw_circle(x, y, RADIUS, black.data(), 1.0, ~0U);
        break;
    case NoteColour::Red:
        image.draw_circle(x, y + RED_OFFSET, RADIUS, red.data());
        image.draw_circle(x, y + RED_OFFSET, RADIUS, black.data(), 1.0, ~0U);
        break;
    case NoteColour::Yellow:
        image.draw_circle(x, y + YELLOW_OFFSET, RADIUS, yellow.data());
        image.draw_circle(x, y + YELLOW_OFFSET, RADIUS, black.data(), 1.0, ~0U);
        break;
    case NoteColour::Blue:
        image.draw_circle(x, y + BLUE_OFFSET, RADIUS, blue.data());
        image.draw_circle(x, y + BLUE_OFFSET, RADIUS, black.data(), 1.0, ~0U);
        break;
    case NoteColour::Orange:
        image.draw_circle(x, y + ORANGE_OFFSET, RADIUS, orange.data());
        image.draw_circle(x, y + ORANGE_OFFSET, RADIUS, black.data(), 1.0, ~0U);
        break;
    case NoteColour::Open:
        image.draw_rectangle(x - RADIUS, y - RADIUS, x + RADIUS,
                             y + MEASURE_HEIGHT + RADIUS, purple.data());
        image.draw_rectangle(x - RADIUS, y - RADIUS, x + RADIUS,
                             y + MEASURE_HEIGHT + RADIUS, black.data(), 1.0,
                             ~0U);
        break;
    }
}

Image create_path_image(const DrawingInstructions& instructions)
{
    constexpr unsigned int IMAGE_WIDTH = 1024;
    constexpr unsigned char WHITE = 255;

    auto height = static_cast<unsigned int>(
        MARGIN + DIST_BETWEEN_MEASURES * instructions.rows.size());

    CImg<unsigned char> image(IMAGE_WIDTH, height, 1, 3, WHITE);
    draw_measures(image, instructions);

    for (const auto& note : instructions.notes) {
        auto row
            = std::find_if(instructions.rows.cbegin(), instructions.rows.cend(),
                           [&](const auto& r) { return r.end > note.beat; });
        auto beats_along_row = note.beat - row->start;
        auto x = LEFT_MARGIN + static_cast<int>(beats_along_row * BEAT_WIDTH);
        auto y = MARGIN
            + (DIST_BETWEEN_MEASURES
               * static_cast<int>(
                   std::distance(instructions.rows.cbegin(), row)));
        draw_note(image, x, y, note.colour);
    }

    return image;
}
