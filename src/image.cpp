/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include <climits>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <set>
#include <stdexcept>

#include "cimg_wrapper.hpp"

#include "image.hpp"
#include "optimiser.hpp"

using namespace cimg_library;

constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 6;
constexpr int VERSION_PATCH = 1;

constexpr int BEAT_WIDTH = 60;
constexpr int FONT_HEIGHT = 13;
constexpr int LEFT_MARGIN = 31;
constexpr int MARGIN = 80;
constexpr int MEASURE_HEIGHT = 61;
constexpr float OPEN_NOTE_OPACITY = 0.5F;
constexpr int TOP_MARGIN = 125;
constexpr int DIST_BETWEEN_MEASURES = MEASURE_HEIGHT + MARGIN;

namespace {
const char* diff_to_str(Difficulty difficulty)
{
    switch (difficulty) {
    case Difficulty::Easy:
        return "Easy";
    case Difficulty::Medium:
        return "Medium";
    case Difficulty::Hard:
        return "Hard";
    case Difficulty::Expert:
        return "Expert";
    default:
        throw std::runtime_error("Invalid difficulty to diff_to_str");
    }
}

std::array<unsigned char, 3> note_colour_to_colour(NoteColour colour)
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

std::array<unsigned char, 3> note_colour_to_colour(DrumNoteColour colour)
{
    constexpr std::array<unsigned char, 3> RED {255, 0, 0};
    constexpr std::array<unsigned char, 3> YELLOW {255, 255, 0};
    constexpr std::array<unsigned char, 3> BLUE {0, 0, 255};
    constexpr std::array<unsigned char, 3> GREEN {0, 255, 0};
    constexpr std::array<unsigned char, 3> ORANGE {255, 165, 0};

    switch (colour) {
    case DrumNoteColour::Red:
    case DrumNoteColour::RedGhost:
    case DrumNoteColour::RedAccent:
        return RED;
    case DrumNoteColour::Yellow:
    case DrumNoteColour::YellowCymbal:
    case DrumNoteColour::YellowGhost:
    case DrumNoteColour::YellowCymbalGhost:
    case DrumNoteColour::YellowAccent:
    case DrumNoteColour::YellowCymbalAccent:
        return YELLOW;
    case DrumNoteColour::Blue:
    case DrumNoteColour::BlueCymbal:
    case DrumNoteColour::BlueGhost:
    case DrumNoteColour::BlueCymbalGhost:
    case DrumNoteColour::BlueAccent:
    case DrumNoteColour::BlueCymbalAccent:
        return BLUE;
    case DrumNoteColour::Green:
    case DrumNoteColour::GreenCymbal:
    case DrumNoteColour::GreenGhost:
    case DrumNoteColour::GreenCymbalGhost:
    case DrumNoteColour::GreenAccent:
    case DrumNoteColour::GreenCymbalAccent:
        return GREEN;
    case DrumNoteColour::Kick:
    case DrumNoteColour::DoubleKick:
        return ORANGE;
    }

    throw std::invalid_argument("Invalid colour to note_colour_to_colour");
}

int note_colour_to_offset(NoteColour colour, bool is_lefty_flip)
{
    constexpr int GREEN_OFFSET = 0;
    constexpr int RED_OFFSET = 15;
    constexpr int YELLOW_OFFSET = 30;
    constexpr int BLUE_OFFSET = 45;
    constexpr int ORANGE_OFFSET = 60;

    int offset = 0;
    switch (colour) {
    case NoteColour::Green:
        offset = GREEN_OFFSET;
        break;
    case NoteColour::Red:
        offset = RED_OFFSET;
        break;
    case NoteColour::Yellow:
        offset = YELLOW_OFFSET;
        break;
    case NoteColour::Blue:
        offset = BLUE_OFFSET;
        break;
    case NoteColour::Orange:
        offset = ORANGE_OFFSET;
        break;
    case NoteColour::Open:
        offset = YELLOW_OFFSET;
        break;
    default:
        throw std::invalid_argument("Invalid colour to note_colour_to_offset");
    }

    if (is_lefty_flip) {
        offset = ORANGE_OFFSET - offset;
    }
    return offset;
}

int note_colour_to_offset(GHLNoteColour colour, bool is_lefty_flip)
{
    constexpr int LOW_OFFSET = 0;
    constexpr int MID_OFFSET = 30;
    constexpr int HIGH_OFFSET = 60;

    int offset = 0;
    switch (colour) {
    case GHLNoteColour::BlackLow:
    case GHLNoteColour::WhiteLow:
        offset = LOW_OFFSET;
        break;
    case GHLNoteColour::BlackMid:
    case GHLNoteColour::WhiteMid:
    case GHLNoteColour::Open:
        offset = MID_OFFSET;
        break;
    case GHLNoteColour::BlackHigh:
    case GHLNoteColour::WhiteHigh:
        offset = HIGH_OFFSET;
        break;
    default:
        throw std::invalid_argument("Invalid colour to note_colour_to_offset");
    }

    if (is_lefty_flip) {
        offset = HIGH_OFFSET - offset;
    }
    return offset;
}

int note_colour_to_offset(DrumNoteColour colour, bool is_lefty_flip)
{
    constexpr int RED_OFFSET = 0;
    constexpr int YELLOW_OFFSET = 20;
    constexpr int BLUE_OFFSET = 40;
    constexpr int GREEN_OFFSET = 60;
    constexpr int KICK_OFFSET = 30;

    int offset = 0;
    switch (colour) {
    case DrumNoteColour::Red:
    case DrumNoteColour::RedGhost:
    case DrumNoteColour::RedAccent:
        offset = RED_OFFSET;
        break;
    case DrumNoteColour::Yellow:
    case DrumNoteColour::YellowCymbal:
    case DrumNoteColour::YellowGhost:
    case DrumNoteColour::YellowCymbalGhost:
    case DrumNoteColour::YellowAccent:
    case DrumNoteColour::YellowCymbalAccent:
        offset = YELLOW_OFFSET;
        break;
    case DrumNoteColour::Blue:
    case DrumNoteColour::BlueCymbal:
    case DrumNoteColour::BlueGhost:
    case DrumNoteColour::BlueCymbalGhost:
    case DrumNoteColour::BlueAccent:
    case DrumNoteColour::BlueCymbalAccent:
        offset = BLUE_OFFSET;
        break;
    case DrumNoteColour::Green:
    case DrumNoteColour::GreenCymbal:
    case DrumNoteColour::GreenGhost:
    case DrumNoteColour::GreenCymbalGhost:
    case DrumNoteColour::GreenAccent:
    case DrumNoteColour::GreenCymbalAccent:
        offset = GREEN_OFFSET;
        break;
    case DrumNoteColour::Kick:
    case DrumNoteColour::DoubleKick:
        offset = KICK_OFFSET;
        break;
    default:
        throw std::invalid_argument("Invalid colour to note_colour_to_offset");
    }

    if (is_lefty_flip) {
        offset = GREEN_OFFSET - offset;
    }
    return offset;
}

std::tuple<int, int> get_xy(const ImageBuilder& builder, double pos)
{
    auto row = std::find_if(builder.rows().cbegin(), builder.rows().cend(),
                            [=](const auto x) { return x.end > pos; });
    auto x = LEFT_MARGIN + static_cast<int>(BEAT_WIDTH * (pos - row->start));
    auto y = TOP_MARGIN + MARGIN
        + DIST_BETWEEN_MEASURES
            * static_cast<int>(std::distance(builder.rows().cbegin(), row));
    return {x, y};
}

int numb_of_fret_lines(TrackType track_type)
{
    switch (track_type) {
    case TrackType::FiveFret:
        return 4;
    case TrackType::SixFret:
        return 2;
    case TrackType::Drums:
        return 3;
    }

    throw std::invalid_argument("Invalid TrackType");
}

enum class DrumSpriteShape { Kick, Cymbal, Tom };

DrumSpriteShape drum_colour_to_shape(DrumNoteColour colour)
{
    switch (colour) {
    case DrumNoteColour::Kick:
    case DrumNoteColour::DoubleKick:
        return DrumSpriteShape::Kick;
    case DrumNoteColour::YellowCymbal:
    case DrumNoteColour::BlueCymbal:
    case DrumNoteColour::GreenCymbal:
    case DrumNoteColour::YellowCymbalGhost:
    case DrumNoteColour::BlueCymbalGhost:
    case DrumNoteColour::GreenCymbalGhost:
    case DrumNoteColour::YellowCymbalAccent:
    case DrumNoteColour::BlueCymbalAccent:
    case DrumNoteColour::GreenCymbalAccent:
        return DrumSpriteShape::Cymbal;
    case DrumNoteColour::Red:
    case DrumNoteColour::Yellow:
    case DrumNoteColour::Blue:
    case DrumNoteColour::Green:
    case DrumNoteColour::RedGhost:
    case DrumNoteColour::YellowGhost:
    case DrumNoteColour::BlueGhost:
    case DrumNoteColour::GreenGhost:
    case DrumNoteColour::RedAccent:
    case DrumNoteColour::YellowAccent:
    case DrumNoteColour::BlueAccent:
    case DrumNoteColour::GreenAccent:
        return DrumSpriteShape::Tom;
    }

    throw std::invalid_argument("Invalid DrumNoteColour");
}

// Codes are
// 0 - No note
// 1 - White note
// 2 - Black note
// 3 - White and black note
std::array<int, 3>
ghl_note_colour_codes(const std::set<GHLNoteColour>& note_colours)
{
    std::array<int, 3> codes {0, 0, 0};
    for (const auto& colour : note_colours) {
        switch (colour) {
        case GHLNoteColour::Open:
            return {0, 0, 0};
        case GHLNoteColour::WhiteLow:
            codes[0] |= 1;
            break;
        case GHLNoteColour::WhiteMid:
            codes[1] |= 1;
            break;
        case GHLNoteColour::WhiteHigh:
            codes[2] |= 1;
            break;
        case GHLNoteColour::BlackLow:
            codes[0] |= 2;
            break;
        case GHLNoteColour::BlackMid:
            codes[1] |= 2;
            break;
        case GHLNoteColour::BlackHigh:
            codes[2] |= 2;
            break;
        }
    }
    return codes;
}

void validate_snprintf_rc(int snprintf_rc)
{
    if (snprintf_rc < 0) {
        throw std::runtime_error("Failure with snprintf");
    }
}
}

class ImageImpl {
private:
    CImg<unsigned char> m_image;

    void draw_note_circle(int x, int y, NoteColour note_colour,
                          bool is_lefty_flip);
    void draw_note_star(int x, int y, NoteColour note_colour,
                        bool is_lefty_flip);
    void draw_note_sustain(const ImageBuilder& builder,
                           const DrawnNote<NoteColour>& note);
    void draw_ghl_note(int x, int y,
                       const std::set<GHLNoteColour>& note_colours,
                       bool is_lefty_flip);
    void draw_ghl_note_sustain(const ImageBuilder& builder,
                               const DrawnNote<GHLNoteColour>& note);
    void draw_drum_note(int x, int y, DrumNoteColour note_colour,
                        bool is_lefty_flip);
    void draw_quarter_note(int x, int y);
    void draw_text_backwards(int x, int y, const char* text,
                             const unsigned char* color, float opacity,
                             unsigned int font_height);
    void draw_vertical_lines(const ImageBuilder& builder,
                             const std::vector<double>& positions,
                             std::array<unsigned char, 3> colour);

public:
    ImageImpl(unsigned int size_x, unsigned int size_y, unsigned int size_z,
              unsigned int size_c, const unsigned char& value)
        : m_image {size_x, size_y, size_z, size_c, value}
    {
    }

    ImageImpl(const ImageImpl&) = delete;
    ImageImpl& operator=(const ImageImpl&) = delete;
    ImageImpl(ImageImpl&&) = delete;
    ImageImpl& operator=(ImageImpl&&) = delete;
    ~ImageImpl() = default;

    void colour_beat_range(const ImageBuilder& builder,
                           std::array<unsigned char, 3> colour,
                           const std::tuple<double, double>& x_range,
                           const std::tuple<int, int>& y_range, float opacity);
    void draw_header(const ImageBuilder& builder);
    void draw_measures(const ImageBuilder& builder);
    void draw_notes(const ImageBuilder& builder);
    void draw_ghl_notes(const ImageBuilder& builder);
    void draw_drum_notes(const ImageBuilder& builder);
    void draw_score_totals(const ImageBuilder& builder);
    void draw_tempos(const ImageBuilder& builder);
    void draw_time_sigs(const ImageBuilder& builder);
    void draw_version();

    void save(const char* filename) const { m_image.save(filename); }
};

void ImageImpl::draw_header(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> BLACK {0, 0, 0};
    constexpr int HEADER_FONT_HEIGHT = 23;

    const auto x = LEFT_MARGIN;
    const auto y = LEFT_MARGIN;
    m_image.draw_text(x, y, "%s\n%s\n%s\n%s\nTotal score = %d", BLACK.data(), 0,
                      1.0, HEADER_FONT_HEIGHT, builder.song_name().c_str(),
                      builder.artist().c_str(), builder.charter().c_str(),
                      diff_to_str(builder.difficulty()), builder.total_score());
}

void ImageImpl::draw_measures(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> BLACK {0, 0, 0};
    constexpr std::array<unsigned char, 3> GREY {160, 160, 160};
    constexpr std::array<unsigned char, 3> LIGHT_GREY {224, 224, 224};
    constexpr std::array<unsigned char, 3> RED {140, 0, 0};
    constexpr int MEASURE_NUMB_GAP = 18;

    const int fret_lines = numb_of_fret_lines(builder.track_type());
    const int colour_distance = (MEASURE_HEIGHT - 1) / fret_lines;

    draw_vertical_lines(builder, builder.beat_lines(), GREY);
    draw_vertical_lines(builder, builder.half_beat_lines(), LIGHT_GREY);

    auto current_row = 0;
    for (const auto& row : builder.rows()) {
        auto y = TOP_MARGIN + DIST_BETWEEN_MEASURES * current_row + MARGIN;
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (row.end - row.start));
        for (int i = 1; i < fret_lines; ++i) {
            m_image.draw_line(LEFT_MARGIN, y + colour_distance * i, x_max,
                              y + colour_distance * i, GREY.data());
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
    constexpr double PERCENT_MULT = 100.0;

    const auto& base_values = builder.base_values();
    const auto& score_values = builder.score_values();
    const auto& sp_values = builder.sp_values();
    const auto& sp_percent_values = builder.sp_percent_values();
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
        if (sp_percent_values.empty() && sp_values[i] == 0) {
            continue;
        }
        y += VALUE_GAP;
        if (sp_percent_values.empty()) {
            const auto print_rc = std::snprintf(buffer.data(), BUFFER_SIZE,
                                                "%.2fSP", sp_values[i]);
            validate_snprintf_rc(print_rc);
        } else {
            const auto print_rc
                = std::snprintf(buffer.data(), BUFFER_SIZE, "%.2f%%%%",
                                PERCENT_MULT * sp_percent_values[i]);
            validate_snprintf_rc(print_rc);
        }
        draw_text_backwards(x, y, buffer.data(), CYAN.data(), 1.0F,
                            FONT_HEIGHT);
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

void ImageImpl::draw_notes(const ImageBuilder& builder)
{
    for (const auto& note : builder.notes()) {
        if (note.length > 0.0) {
            draw_note_sustain(builder, note);
        }

        const auto [x, y] = get_xy(builder, note.beat);
        if (note.is_sp_note) {
            draw_note_star(x, y, note.colour, builder.is_lefty_flip());
        } else {
            draw_note_circle(x, y, note.colour, builder.is_lefty_flip());
        }
    }
}

void ImageImpl::draw_ghl_notes(const ImageBuilder& builder)
{
    for (const auto& note : builder.ghl_notes()) {
        if (note.length > 0.0) {
            draw_ghl_note_sustain(builder, note);
        }
    }

    // double is beat, std::set is colours of note
    std::vector<std::tuple<double, std::set<GHLNoteColour>>> note_groups;
    for (const auto& note : builder.ghl_notes()) {
        if (note_groups.empty()
            || std::get<0>(note_groups.back()) < note.beat) {
            note_groups.push_back({note.beat, {note.colour}});
        }
        std::get<1>(note_groups.back()).insert(note.colour);
    }

    for (const auto& [pos, colours] : note_groups) {
        const auto [x, y] = get_xy(builder, pos);
        draw_ghl_note(x, y, colours, builder.is_lefty_flip());
    }
}

void ImageImpl::draw_drum_notes(const ImageBuilder& builder)
{
    // We draw all the kicks first because we want RYBG to lie on top of the
    // kicks, not underneath.
    for (const auto& note : builder.drum_notes()) {
        if (drum_colour_to_shape(note.colour) != DrumSpriteShape::Kick) {
            continue;
        }
        const auto [x, y] = get_xy(builder, note.beat);
        draw_drum_note(x, y, note.colour, builder.is_lefty_flip());
    }

    for (const auto& note : builder.drum_notes()) {
        if (drum_colour_to_shape(note.colour) == DrumSpriteShape::Kick) {
            continue;
        }
        const auto [x, y] = get_xy(builder, note.beat);
        draw_drum_note(x, y, note.colour, builder.is_lefty_flip());
    }
}

void ImageImpl::draw_note_circle(int x, int y, NoteColour note_colour,
                                 bool is_lefty_flip)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr int RADIUS = 5;

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour, is_lefty_flip);

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

void ImageImpl::draw_ghl_note(int x, int y,
                              const std::set<GHLNoteColour>& note_colours,
                              bool is_lefty_flip)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr std::array<unsigned char, 3> grey {30, 30, 30};
    constexpr std::array<unsigned char, 3> white {255, 255, 255};
    constexpr int FRET_GAP = 30;
    constexpr int RADIUS = 5;

    if (note_colours.count(GHLNoteColour::Open) != 0) {
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               white.data(), OPEN_NOTE_OPACITY);
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               black.data(), 1.0, ~0U);
        return;
    }

    const auto codes = ghl_note_colour_codes(note_colours);

    for (auto i = 0U; i < 3; ++i) {
        auto offset = FRET_GAP * static_cast<int>(i);
        if (is_lefty_flip) {
            offset = 2 * FRET_GAP - offset;
        }
        if (codes.at(i) == 0) {
            continue;
        }
        if (codes.at(i) == 1) {
            m_image.draw_circle(x, y + offset, RADIUS, white.data());
            m_image.draw_circle(x, y + offset, RADIUS, black.data(), 1.0, ~0U);
        } else if (codes.at(i) == 2) {
            m_image.draw_circle(x, y + offset, RADIUS, grey.data());
            m_image.draw_circle(x, y + offset, RADIUS, black.data(), 1.0, ~0U);
        } else if (codes.at(i) == 3) {
            m_image.draw_rectangle(x - RADIUS, y + offset - RADIUS, x + RADIUS,
                                   y + offset, grey.data());
            m_image.draw_rectangle(x - RADIUS, y + offset, x + RADIUS,
                                   y + offset + RADIUS, white.data());
            m_image.draw_rectangle(x - RADIUS, y + offset, x + RADIUS,
                                   y + offset + RADIUS, black.data(), 1.0, ~0U);
        }
    }
}

void ImageImpl::draw_drum_note(int x, int y, DrumNoteColour note_colour,
                               bool is_lefty_flip)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr int RADIUS = 5;

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour, is_lefty_flip);

    switch (drum_colour_to_shape(note_colour)) {
    case DrumSpriteShape::Kick:
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               colour.data(), OPEN_NOTE_OPACITY);
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               black.data(), 1.0, ~0U);
        break;
    case DrumSpriteShape::Cymbal:
        m_image.draw_triangle(x, y + offset - RADIUS, x - RADIUS,
                              y + offset + RADIUS, x + RADIUS,
                              y + offset + RADIUS, colour.data());
        m_image.draw_triangle(x, y + offset - RADIUS, x - RADIUS,
                              y + offset + RADIUS, x + RADIUS,
                              y + offset + RADIUS, black.data(), 1.0, ~0U);
        break;
    case DrumSpriteShape::Tom:
        m_image.draw_circle(x, y + offset, RADIUS, colour.data());
        m_image.draw_circle(x, y + offset, RADIUS, black.data(), 1.0, ~0U);
    }
}

void ImageImpl::draw_note_star(int x, int y, NoteColour note_colour,
                               bool is_lefty_flip)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour, is_lefty_flip);

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
                                  const DrawnNote<NoteColour>& note)
{
    constexpr std::tuple<int, int> OPEN_NOTE_Y_RANGE {7, 53};

    auto colour = note_colour_to_colour(note.colour);
    std::tuple<double, double> x_range {note.beat, note.beat + note.length};
    auto offset = note_colour_to_offset(note.colour, builder.is_lefty_flip());
    std::tuple<int, int> y_range {offset - 3, offset + 3};
    float opacity = 1.0F;
    if (note.colour == NoteColour::Open) {
        y_range = OPEN_NOTE_Y_RANGE;
        opacity = OPEN_NOTE_OPACITY;
    }
    colour_beat_range(builder, colour, x_range, y_range, opacity);
}

void ImageImpl::draw_ghl_note_sustain(const ImageBuilder& builder,
                                      const DrawnNote<GHLNoteColour>& note)
{
    constexpr std::tuple<int, int> OPEN_NOTE_Y_RANGE {7, 53};
    constexpr std::array<unsigned char, 3> SUST_COLOUR {150, 150, 150};

    std::tuple<double, double> x_range {note.beat, note.beat + note.length};
    auto offset = note_colour_to_offset(note.colour, builder.is_lefty_flip());
    std::tuple<int, int> y_range {offset - 3, offset + 3};
    float opacity = 1.0F;
    if (note.colour == GHLNoteColour::Open) {
        y_range = OPEN_NOTE_Y_RANGE;
        opacity = OPEN_NOTE_OPACITY;
    }
    colour_beat_range(builder, SUST_COLOUR, x_range, y_range, opacity);
}

void ImageImpl::colour_beat_range(const ImageBuilder& builder,
                                  std::array<unsigned char, 3> colour,
                                  const std::tuple<double, double>& x_range,
                                  const std::tuple<int, int>& y_range,
                                  float opacity)
{
    double start = std::numeric_limits<double>::quiet_NaN();
    double end = std::numeric_limits<double>::quiet_NaN();
    std::tie(start, end) = x_range;
    // Required if a beat range ends after the end of a song (e.g., a solo
    // section)
    end = std::min(end, std::nextafter(builder.rows().back().end, 0.0));
    auto row_iter = std::find_if(builder.rows().cbegin(), builder.rows().cend(),
                                 [=](const auto& r) { return r.end > start; });
    auto row
        = static_cast<int>(std::distance(builder.rows().cbegin(), row_iter));

    const auto& [y_min, y_max] = y_range;

    while (start < end) {
        auto block_end = std::min(row_iter->end, end);
        auto x_min = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (start - row_iter->start));
        // -1 is so regions that cross rows do not go over the ending line of a
        // row.
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (block_end - row_iter->start)) - 1;
        if (x_min <= x_max) {
            auto y = TOP_MARGIN + MARGIN + DIST_BETWEEN_MEASURES * row;
            m_image.draw_rectangle(x_min, y + y_min, x_max, y + y_max,
                                   colour.data(), opacity);
        }
        start = block_end;
        ++row_iter;
        ++row;
    }
}

void ImageImpl::draw_version()
{
    for (auto i = 0; i < CHAR_BIT; ++i) {
        m_image(i, 0, 0) ^= (VERSION_MAJOR >> (CHAR_BIT - i - 1)) & 1;
        m_image(i, 0, 1) ^= (VERSION_MINOR >> (CHAR_BIT - i - 1)) & 1;
        m_image(i, 0, 2) ^= (VERSION_PATCH >> (CHAR_BIT - i - 1)) & 1;
    }
}

Image::Image(const ImageBuilder& builder)
{
    constexpr std::array<unsigned char, 3> green {0, 255, 0};
    constexpr std::array<unsigned char, 3> blue {0, 0, 255};
    constexpr std::array<unsigned char, 3> yellow {255, 255, 0};
    constexpr std::array<unsigned char, 3> red {255, 0, 0};
    constexpr std::array<unsigned char, 3> solo_blue {0, 51, 128};
    constexpr std::array<unsigned char, 3> pink {127, 0, 0};

    constexpr unsigned int IMAGE_WIDTH = 1024;
    constexpr float RANGE_OPACITY = 0.33333F;
    constexpr int SOLO_HEIGHT = 10;
    constexpr unsigned char WHITE = 255;

    const auto height = static_cast<unsigned int>(
        TOP_MARGIN + MARGIN + DIST_BETWEEN_MEASURES * builder.rows().size());

    m_impl = std::make_unique<ImageImpl>(IMAGE_WIDTH, height, 1, 3, WHITE);
    m_impl->draw_version();
    m_impl->draw_header(builder);
    m_impl->draw_measures(builder);
    m_impl->draw_tempos(builder);
    m_impl->draw_time_sigs(builder);

    for (const auto& range : builder.solo_ranges()) {
        m_impl->colour_beat_range(builder, solo_blue, range,
                                  {-SOLO_HEIGHT, MEASURE_HEIGHT + SOLO_HEIGHT},
                                  RANGE_OPACITY / 2);
    }

    for (const auto& range : builder.bre_ranges()) {
        m_impl->colour_beat_range(builder, pink, range,
                                  {-SOLO_HEIGHT, MEASURE_HEIGHT + SOLO_HEIGHT},
                                  RANGE_OPACITY / 2);
    }

    for (const auto& range : builder.fill_ranges()) {
        m_impl->colour_beat_range(builder, pink, range,
                                  {-SOLO_HEIGHT, MEASURE_HEIGHT + SOLO_HEIGHT},
                                  RANGE_OPACITY / 2);
    }

    for (const auto& range : builder.unison_ranges()) {
        m_impl->colour_beat_range(builder, yellow, range, {-SOLO_HEIGHT, 0},
                                  RANGE_OPACITY / 2);
        m_impl->colour_beat_range(
            builder, yellow, range,
            {MEASURE_HEIGHT, MEASURE_HEIGHT + SOLO_HEIGHT}, RANGE_OPACITY / 2);
    }

    switch (builder.track_type()) {
    case TrackType::FiveFret:
        m_impl->draw_notes(builder);
        break;
    case TrackType::SixFret:
        m_impl->draw_ghl_notes(builder);
        break;
    case TrackType::Drums:
        m_impl->draw_drum_notes(builder);
        break;
    }

    m_impl->draw_score_totals(builder);

    for (const auto& range : builder.green_ranges()) {
        m_impl->colour_beat_range(builder, green, range, {0, MEASURE_HEIGHT},
                                  RANGE_OPACITY);
    }
    for (const auto& range : builder.yellow_ranges()) {
        m_impl->colour_beat_range(builder, yellow, range, {0, MEASURE_HEIGHT},
                                  RANGE_OPACITY);
    }
    for (const auto& range : builder.red_ranges()) {
        m_impl->colour_beat_range(builder, red, range, {0, MEASURE_HEIGHT},
                                  builder.activation_opacity());
    }
    for (const auto& range : builder.blue_ranges()) {
        m_impl->colour_beat_range(builder, blue, range, {0, MEASURE_HEIGHT},
                                  builder.activation_opacity());
    }
}

Image::~Image() = default;

Image::Image(Image&& image) noexcept = default;

Image& Image::operator=(Image&& image) noexcept = default;

void Image::save(const char* filename) const { m_impl->save(filename); }
