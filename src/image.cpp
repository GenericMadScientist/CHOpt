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
#include <cstdio>
#include <iterator>
#include <limits>

#include "cimg_wrapper.hpp"

#include "image.hpp"

using namespace cimg_library;

constexpr int BEAT_WIDTH = 60;
constexpr int FONT_HEIGHT = 13;
constexpr int LEFT_MARGIN = 31;
constexpr int MARGIN = 80;
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

ImageBuilder::ImageBuilder(const NoteTrack& track, int resolution,
                           const SyncTrack& sync_track)
    : m_rows {drawn_rows(track, resolution, sync_track)}
    , m_notes {drawn_notes(track, resolution)}
{
    constexpr double HALF_BEAT = 0.5;

    for (const auto& row : m_rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length = get_beat_rate(sync_track, resolution, start);
            auto numer = get_numer(sync_track, resolution, start);
            auto denom = get_denom(sync_track, resolution, start);
            m_measure_lines.push_back(start);
            m_half_beat_lines.push_back(start + HALF_BEAT * denom);
            for (int i = 1; i < numer; ++i) {
                m_beat_lines.push_back(start + i * denom);
                m_half_beat_lines.push_back(start + (i + HALF_BEAT) * denom);
            }
            start += meas_length;
        }
    }
    m_measure_lines.push_back(m_rows.back().end);
}

void ImageBuilder::add_bpms(const SyncTrack& sync_track, int resolution)
{
    constexpr double MS_PER_SECOND = 1000.0;

    m_bpms.clear();

    for (const auto& bpm : sync_track.bpms()) {
        auto pos = bpm.position / static_cast<double>(resolution);
        auto tempo = bpm.bpm / MS_PER_SECOND;
        if (pos < m_rows.back().end) {
            m_bpms.emplace_back(pos, tempo);
        }
    }
}

void ImageBuilder::add_measure_values(const PointSet& points, const Path& path)
{
    m_base_values.clear();
    m_base_values.resize(m_measure_lines.size() - 1);

    m_score_values.clear();
    m_score_values.resize(m_measure_lines.size() - 1);

    auto base_value_iter = m_base_values.begin();
    auto meas_iter = std::next(m_measure_lines.cbegin());
    auto score_value_iter = m_score_values.begin();
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        while (meas_iter != m_measure_lines.cend()
               && *meas_iter <= p->position.beat.value()) {
            ++meas_iter;
            ++base_value_iter;
            ++score_value_iter;
        }
        *base_value_iter += p->base_value;
        *score_value_iter += p->value;
    }

    meas_iter = std::next(m_measure_lines.cbegin());
    score_value_iter = m_score_values.begin();
    for (const auto& solo : points.solo_boosts()) {
        while (meas_iter != m_measure_lines.cend()
               && *meas_iter <= std::get<0>(solo).beat.value()) {
            ++meas_iter;
            ++score_value_iter;
        }
        *score_value_iter += std::get<1>(solo);
    }

    for (const auto& act : path.activations) {
        meas_iter = std::next(m_measure_lines.cbegin());
        score_value_iter = m_score_values.begin();
        for (auto p = act.act_start; p <= act.act_end; ++p) {
            while (meas_iter != m_measure_lines.cend()
                   && *meas_iter <= p->position.beat.value()) {
                ++meas_iter;
                ++score_value_iter;
            }
            *score_value_iter += p->value;
        }
    }

    int score_total = 0;
    for (auto& score : m_score_values) {
        score_total += score;
        score = score_total;
    }
}

void ImageBuilder::add_solo_sections(const NoteTrack& track, int resolution)
{
    for (const auto& solo : track.solos()) {
        auto start = solo.start / static_cast<double>(resolution);
        auto end = solo.end / static_cast<double>(resolution);
        m_solo_ranges.emplace_back(start, end);
    }
}

void ImageBuilder::add_sp_acts(const Path& path)
{
    for (const auto& act : path.activations) {
        auto start = act.act_start->position.beat.value();
        auto end = act.act_end->position.beat.value();
        m_blue_ranges.emplace_back(start, end);
    }
}

void ImageBuilder::add_sp_phrases(const NoteTrack& track, int resolution)
{
    for (const auto& phrase : track.sp_phrases()) {
        auto p = track.notes().cbegin();
        while (p->position < phrase.position) {
            ++p;
        }
        auto start = p->position / static_cast<double>(resolution);
        auto phrase_end = phrase.position + phrase.length;
        auto end = start;
        while (p < track.notes().cend() && p->position < phrase_end) {
            auto current_end
                = (p->position + p->length) / static_cast<double>(resolution);
            end = std::max(end, current_end);
            ++p;
        }
        m_green_ranges.emplace_back(start, end);
    }
}

void ImageBuilder::add_sp_values(const SpData& sp_data)
{
    constexpr double WHAMMY_BEATS_IN_BAR = 30.0;
    m_sp_values.clear();
    m_sp_values.resize(m_measure_lines.size() - 1);

    for (std::size_t i = 0; i < m_measure_lines.size() - 1; ++i) {
        Beat start {m_measure_lines[i]};
        Beat end {std::numeric_limits<double>::infinity()};
        if (i < m_measure_lines.size() - 1) {
            end = Beat {m_measure_lines[i + 1]};
        }
        m_sp_values[i]
            = WHAMMY_BEATS_IN_BAR * sp_data.available_whammy(start, end);
    }
}

void ImageBuilder::add_time_sigs(const SyncTrack& sync_track, int resolution)
{
    for (const auto& ts : sync_track.time_sigs()) {
        auto pos = ts.position / static_cast<double>(resolution);
        auto num = ts.numerator;
        auto denom = ts.denominator;
        if (pos < m_rows.back().end) {
            m_time_sigs.emplace_back(pos, num, denom);
        }
    }
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

class ImageImpl {
private:
    CImg<int> m_image;

    void draw_note_circle(int x, int y, NoteColour note_colour);
    void draw_note_star(int x, int y, NoteColour note_colour);
    void draw_note_sustain(const ImageBuilder& builder, const DrawnNote& note);
    void draw_quarter_note(int x, int y);
    void draw_text_backwards(int x, int y, const char* text,
                             const unsigned char* color, float opacity,
                             unsigned int font_height);
    void draw_vertical_lines(const ImageBuilder& builder,
                             const std::vector<double>& positions,
                             std::array<unsigned char, 3> colour);

public:
    ImageImpl(unsigned int size_x, unsigned int size_y, unsigned int size_z,
              unsigned int size_c, const int& value)
        : m_image {size_x, size_y, size_z, size_c, value}
    {
    }

    void colour_beat_range(const ImageBuilder& builder,
                           std::array<unsigned char, 3> colour,
                           std::tuple<double, double> x_range,
                           std::tuple<int, int> y_range, float opacity);
    void draw_measures(const ImageBuilder& builder);
    void draw_note(const ImageBuilder& builder, const DrawnNote& note);
    void draw_score_totals(const ImageBuilder& builder);
    void draw_tempos(const ImageBuilder& builder);
    void draw_time_sigs(const ImageBuilder& builder);

    void save(const char* filename) const { m_image.save(filename); }
};

static std::tuple<int, int> get_xy(const ImageBuilder& builder, double pos)
{
    auto row = std::find_if(builder.rows().cbegin(), builder.rows().cend(),
                            [=](const auto x) { return x.end > pos; });
    auto x = LEFT_MARGIN + static_cast<int>(BEAT_WIDTH * (pos - row->start));
    auto y = MARGIN
        + DIST_BETWEEN_MEASURES
            * static_cast<int>(std::distance(builder.rows().cbegin(), row));
    return {x, y};
}

void ImageImpl::draw_measures(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> BLACK {0, 0, 0};
    constexpr std::array<unsigned char, 3> GREY {160, 160, 160};
    constexpr std::array<unsigned char, 3> LIGHT_GREY {224, 224, 224};
    constexpr std::array<unsigned char, 3> RED {140, 0, 0};
    constexpr int COLOUR_DISTANCE = 15;
    constexpr int MEASURE_NUMB_GAP = 18;

    draw_vertical_lines(builder, builder.beat_lines(), GREY);
    draw_vertical_lines(builder, builder.half_beat_lines(), LIGHT_GREY);

    auto current_row = 0;
    for (const auto& row : builder.rows()) {
        auto y = DIST_BETWEEN_MEASURES * current_row + MARGIN;
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (row.end - row.start));
        for (int i = 1; i < 4; ++i) {
            m_image.draw_line(LEFT_MARGIN, y + COLOUR_DISTANCE * i, x_max,
                              y + COLOUR_DISTANCE * i, GREY.data());
        }
        m_image.draw_rectangle(LEFT_MARGIN, y, x_max, y + MEASURE_HEIGHT,
                               BLACK.data(), 1.0, ~0U);
        ++current_row;
    }

    // We do measure lines after the boxes because we want the measure lines to
    // lie over the horizontal grey fretboard lines. We make a copy of the
    // measures missing the last one because we don't want to draw the last
    // measure line, since that is already dealt with by drawing the boxes.
    std::vector<double> measure_copy {
        builder.measure_lines().cbegin(),
        std::prev(builder.measure_lines().cend())};
    draw_vertical_lines(builder, measure_copy, BLACK);

    for (std::size_t i = 0; i < builder.measure_lines().size() - 1; ++i) {
        auto pos = builder.measure_lines()[i];
        auto [x, y] = get_xy(builder, pos);
        y -= MEASURE_NUMB_GAP;
        m_image.draw_text(x, y, "%u", RED.data(), 0, 1.0, FONT_HEIGHT, i + 1);
    }
}

void ImageImpl::draw_vertical_lines(const ImageBuilder& builder,
                                    const std::vector<double>& positions,
                                    std::array<unsigned char, 3> colour)
{
    for (auto pos : positions) {
        auto [x, y] = get_xy(builder, pos);
        m_image.draw_line(x, y, x, y + MEASURE_HEIGHT, colour.data());
    }
}

void ImageImpl::draw_tempos(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> GREY {160, 160, 160};
    constexpr int TEMPO_OFFSET = 31;

    for (const auto& [pos, tempo] : builder.bpms()) {
        auto [x, y] = get_xy(builder, pos);
        y -= TEMPO_OFFSET;
        m_image.draw_text(x + 1, y, " =%.f", GREY.data(), 0, 1.0, FONT_HEIGHT,
                          tempo);
        draw_quarter_note(x, y);
    }
}

void ImageImpl::draw_quarter_note(int x, int y)
{
    constexpr int GREY_VALUE = 160;
    constexpr std::array<std::tuple<int, int>, 26> PIXELS {
        {{4, 0},  {4, 1},  {4, 2},  {4, 3},  {4, 4},  {4, 5},  {4, 6},
         {4, 7},  {4, 8},  {4, 9},  {4, 10}, {4, 11}, {3, 9},  {3, 10},
         {3, 11}, {3, 12}, {2, 9},  {2, 10}, {2, 11}, {2, 12}, {1, 9},
         {1, 10}, {1, 11}, {1, 12}, {0, 10}, {0, 11}}};

    for (const auto& [diff_x, diff_y] : PIXELS) {
        auto x_pos = static_cast<unsigned int>(x + diff_x);
        auto y_pos = static_cast<unsigned int>(y + diff_y);
        m_image(x_pos, y_pos, 0) = GREY_VALUE;
        m_image(x_pos, y_pos, 1) = GREY_VALUE;
        m_image(x_pos, y_pos, 2) = GREY_VALUE;
    }
}

void ImageImpl::draw_time_sigs(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> GREY {160, 160, 160};
    constexpr int TS_FONT_HEIGHT = 38;
    constexpr int TS_GAP = MEASURE_HEIGHT / 16;

    for (const auto& [pos, num, denom] : builder.time_sigs()) {
        auto [x, y] = get_xy(builder, pos);
        x += TS_GAP;
        y -= TS_GAP;
        m_image.draw_text(x, y, "%d", GREY.data(), 0, 1.0, TS_FONT_HEIGHT, num);
        m_image.draw_text(x, y + MEASURE_HEIGHT / 2, "%d", GREY.data(), 0, 1.0,
                          TS_FONT_HEIGHT, denom);
    }
}

void ImageImpl::draw_score_totals(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> CYAN {0, 160, 160};
    constexpr std::array<unsigned char, 3> GREEN {0, 100, 0};
    constexpr std::array<unsigned char, 3> GREY {160, 160, 160};

    constexpr int BASE_VALUE_MARGIN = 5;
    // This is enough room for the max double (below 10^309, a '.', two more
    // digits, then "SP").
    constexpr std::size_t BUFFER_SIZE = 315;
    constexpr int VALUE_GAP = 13;

    const auto& base_values = builder.base_values();
    const auto& score_values = builder.score_values();
    const auto& sp_values = builder.sp_values();
    const auto& measures = builder.measure_lines();

    std::array<char, BUFFER_SIZE> buffer {};

    for (std::size_t i = 0; i < base_values.size(); ++i) {
        // We need to go to the previous double because otherwise we have an OOB
        // when drawing the scores for the last measure.
        auto pos = std::nextafter(measures[i + 1], -1.0);
        auto [x, y] = get_xy(builder, pos);
        y += BASE_VALUE_MARGIN + MEASURE_HEIGHT;
        auto text = std::to_string(base_values[i]);
        draw_text_backwards(x, y, text.c_str(), GREY.data(), 1.0F, FONT_HEIGHT);
        text = std::to_string(score_values[i]);
        y += VALUE_GAP;
        draw_text_backwards(x, y, text.c_str(), GREEN.data(), 1.0F,
                            FONT_HEIGHT);
        if (sp_values[i] > 0) {
            y += VALUE_GAP;
            std::snprintf(buffer.data(), BUFFER_SIZE, "%.2fSP", sp_values[i]);
            draw_text_backwards(x, y, buffer.data(), CYAN.data(), 1.0F,
                                FONT_HEIGHT);
        }
    }
}

// CImg's normal draw_text method draws the text so the top left corner of the
// box is at (x, y). This method draws the text so that the top right corner of
// the box is at (x, y).
void ImageImpl::draw_text_backwards(int x, int y, const char* text,
                                    const unsigned char* color, float opacity,
                                    unsigned int font_height)
{
    // Hack for getting the text box width from the answer at
    // stackoverflow.com/questions/24190327.
    CImg<int> img_text;
    img_text.draw_text(0, 0, text, color, 0, opacity, font_height);
    x -= img_text.width();
    m_image.draw_text(x, y, text, color, 0, opacity, font_height);
}

void ImageImpl::draw_note(const ImageBuilder& builder, const DrawnNote& note)
{
    if (note.length > 0.0) {
        draw_note_sustain(builder, note);
    }

    const auto [x, y] = get_xy(builder, note.beat);
    if (note.is_sp_note) {
        draw_note_star(x, y, note.colour);
    } else {
        draw_note_circle(x, y, note.colour);
    }
}

void ImageImpl::draw_note_circle(int x, int y, NoteColour note_colour)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr int RADIUS = 5;

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    if (note_colour == NoteColour::Open) {
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               colour.data(), OPEN_NOTE_OPACITY);
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               black.data(), 1.0, ~0U);
    } else {
        m_image.draw_circle(x, y + offset, RADIUS, colour.data());
        m_image.draw_circle(x, y + offset, RADIUS, black.data(), 1.0, ~0U);
    }
}

void ImageImpl::draw_note_star(int x, int y, NoteColour note_colour)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    constexpr unsigned int POINTS_IN_STAR_POLYGON = 10;
    CImg<int> points {POINTS_IN_STAR_POLYGON, 2};
    constexpr std::array<int, 20> coords {0, -6, 1, -2, 5, -2, 2,  1,  3, 5, 0,
                                          2, -3, 5, -2, 1, -5, -2, -1, -2};
    for (auto i = 0U; i < POINTS_IN_STAR_POLYGON; ++i) {
        points(i, 0) = coords[2 * i] + x; // NOLINT
        points(i, 1) = coords[2 * i + 1] + y + offset; // NOLINT
    }

    if (note_colour == NoteColour::Open) {
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               colour.data(), OPEN_NOTE_OPACITY);
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               black.data(), 1.0, ~0U);
    } else {
        m_image.draw_polygon(points, colour.data());
        m_image.draw_polygon(points, black.data(), 1.0, ~0U);
    }
}

void ImageImpl::draw_note_sustain(const ImageBuilder& builder,
                                  const DrawnNote& note)
{
    constexpr std::tuple<int, int> OPEN_NOTE_Y_RANGE {7, 53};

    auto colour = note_colour_to_colour(note.colour);
    std::tuple<double, double> x_range {note.beat, note.beat + note.length};
    auto offset = note_colour_to_offset(note.colour);
    std::tuple<int, int> y_range {offset - 3, offset + 3};
    float opacity = 1.0F;
    if (note.colour == NoteColour::Open) {
        y_range = OPEN_NOTE_Y_RANGE;
        opacity = OPEN_NOTE_OPACITY;
    }
    colour_beat_range(builder, colour, x_range, y_range, opacity);
}

void ImageImpl::colour_beat_range(const ImageBuilder& builder,
                                  std::array<unsigned char, 3> colour,
                                  std::tuple<double, double> x_range,
                                  std::tuple<int, int> y_range, float opacity)
{
    auto start = std::get<0>(x_range);
    auto end = std::get<1>(x_range);
    auto row_iter = std::find_if(builder.rows().cbegin(), builder.rows().cend(),
                                 [=](const auto& r) { return r.end > start; });
    auto row
        = static_cast<int>(std::distance(builder.rows().cbegin(), row_iter));

    auto y_min = std::get<0>(y_range);
    auto y_max = std::get<1>(y_range);

    while (start < end) {
        auto block_end = std::min(row_iter->end, end);
        auto x_min = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (start - row_iter->start));
        // -1 is so regions that cross rows do not go over the ending line of a
        // row.
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (block_end - row_iter->start)) - 1;
        if (x_min <= x_max) {
            auto y = MARGIN + DIST_BETWEEN_MEASURES * row;
            m_image.draw_rectangle(x_min, y + y_min, x_max, y + y_max,
                                   colour.data(), opacity);
        }
        start = block_end;
        ++row_iter;
        ++row;
    }
}

Image::Image(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> green {0, 255, 0};
    constexpr std::array<unsigned char, 3> blue {0, 0, 255};
    constexpr std::array<unsigned char, 3> solo_blue {0, 51, 128};

    constexpr unsigned int IMAGE_WIDTH = 1024;
    constexpr float RANGE_OPACITY = 0.25F;
    constexpr int SOLO_HEIGHT = 10;
    constexpr unsigned char WHITE = 255;

    auto height = static_cast<unsigned int>(
        MARGIN + DIST_BETWEEN_MEASURES * builder.rows().size());

    m_impl = std::make_unique<ImageImpl>(IMAGE_WIDTH, height, 1, 3, WHITE);
    m_impl->draw_measures(builder);
    m_impl->draw_tempos(builder);
    m_impl->draw_time_sigs(builder);

    for (const auto& range : builder.solo_ranges()) {
        m_impl->colour_beat_range(builder, solo_blue, range,
                                  {-SOLO_HEIGHT, MEASURE_HEIGHT + SOLO_HEIGHT},
                                  RANGE_OPACITY / 2);
    }

    for (const auto& note : builder.notes()) {
        m_impl->draw_note(builder, note);
    }

    m_impl->draw_score_totals(builder);

    for (const auto& range : builder.green_ranges()) {
        m_impl->colour_beat_range(builder, green, range, {0, MEASURE_HEIGHT},
                                  RANGE_OPACITY);
    }
    for (const auto& range : builder.blue_ranges()) {
        m_impl->colour_beat_range(builder, blue, range, {0, MEASURE_HEIGHT},
                                  RANGE_OPACITY);
    }
}

Image::~Image() = default;

Image::Image(Image&& image) noexcept = default;

Image& Image::operator=(Image&& image) noexcept = default;

void Image::save(const char* filename) const { m_impl->save(filename); }
