/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022 Raymond Wright
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

#include <boost/nowide/fstream.hpp>
#include <cairo/cairo.h>

#include "image.hpp"
#include "optimiser.hpp"

constexpr int BEAT_WIDTH = 60;
constexpr int FONT_HEIGHT = 13;
constexpr int LEFT_MARGIN = 31;
constexpr int MARGIN = 80;
constexpr int MEASURE_HEIGHT = 61;
constexpr float OPEN_NOTE_OPACITY = 0.5F;
constexpr int TOP_MARGIN = 100;
constexpr int DIST_BETWEEN_MEASURES = MEASURE_HEIGHT + MARGIN;
constexpr double HALF_PIXEL = 0.5; // Needed due to how Cairo aligns lines
constexpr double TWO_PI = 6.283185307179586;

struct Colour {
    double red;
    double green;
    double blue;
};

static Colour note_colour_to_colour(NoteColour colour)
{
    constexpr Colour GREEN {0.0, 1.0, 0.0};
    constexpr Colour RED {1.0, 0.0, 0.0};
    constexpr Colour YELLOW {1.0, 1.0, 0.0};
    constexpr Colour BLUE {0.0, 0.0, 1.0};
    constexpr Colour ORANGE {1.0, 0.647, 0.0};
    constexpr Colour PURPLE {0.5, 0, 0.5};

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

static Colour note_colour_to_colour(DrumNoteColour colour)
{
    constexpr Colour RED {1.0, 0.0, 0.0};
    constexpr Colour YELLOW {1.0, 1.0, 0.0};
    constexpr Colour BLUE {0.0, 0.0, 1.0};
    constexpr Colour GREEN {0.0, 1.0, 0.0};
    constexpr Colour ORANGE {1.0, 0.647, 0.0};

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

static int note_colour_to_offset(GHLNoteColour colour)
{
    constexpr int LOW_OFFSET = 0;
    constexpr int MID_OFFSET = 30;
    constexpr int HIGH_OFFSET = 60;

    switch (colour) {
    case GHLNoteColour::BlackLow:
    case GHLNoteColour::WhiteLow:
        return LOW_OFFSET;
    case GHLNoteColour::BlackMid:
    case GHLNoteColour::WhiteMid:
    case GHLNoteColour::Open:
        return MID_OFFSET;
    case GHLNoteColour::BlackHigh:
    case GHLNoteColour::WhiteHigh:
        return HIGH_OFFSET;
    }

    throw std::invalid_argument("Invalid colour to note_colour_to_offset");
}

static int note_colour_to_offset(DrumNoteColour colour)
{
    constexpr int RED_OFFSET = 0;
    constexpr int YELLOW_OFFSET = 20;
    constexpr int BLUE_OFFSET = 40;
    constexpr int GREEN_OFFSET = 60;
    constexpr int KICK_OFFSET = 30;

    switch (colour) {
    case DrumNoteColour::Red:
    case DrumNoteColour::RedGhost:
    case DrumNoteColour::RedAccent:
        return RED_OFFSET;
    case DrumNoteColour::Yellow:
    case DrumNoteColour::YellowCymbal:
    case DrumNoteColour::YellowGhost:
    case DrumNoteColour::YellowCymbalGhost:
    case DrumNoteColour::YellowAccent:
    case DrumNoteColour::YellowCymbalAccent:
        return YELLOW_OFFSET;
    case DrumNoteColour::Blue:
    case DrumNoteColour::BlueCymbal:
    case DrumNoteColour::BlueGhost:
    case DrumNoteColour::BlueCymbalGhost:
    case DrumNoteColour::BlueAccent:
    case DrumNoteColour::BlueCymbalAccent:
        return BLUE_OFFSET;
    case DrumNoteColour::Green:
    case DrumNoteColour::GreenCymbal:
    case DrumNoteColour::GreenGhost:
    case DrumNoteColour::GreenCymbalGhost:
    case DrumNoteColour::GreenAccent:
    case DrumNoteColour::GreenCymbalAccent:
        return GREEN_OFFSET;
    case DrumNoteColour::Kick:
    case DrumNoteColour::DoubleKick:
        return KICK_OFFSET;
    }

    throw std::invalid_argument("Invalid colour to note_colour_to_offset");
}

static void write_int(boost::nowide::ofstream& stream, int value)
{
    stream.write(reinterpret_cast<char*>(&value), sizeof value); // NOLINT
}

static void write_24_bit_bmp_header(boost::nowide::ofstream& stream, int width,
                                    int height)
{
    constexpr int BPP_AND_PLANES = 0x00180001;
    constexpr int DIB_HEADER_SIZE = 40;
    constexpr int HEADER_SIZE = 54;
    constexpr int PPM_RESOLUTION = 256;

    const auto row_padding
        = ((3 * width) % 4 == 0) ? 0 : (4 - ((3 * width) % 4));
    const auto size = (3 * width + row_padding) * height;

    stream << "BM";
    write_int(stream, size + HEADER_SIZE);
    write_int(stream, 0);
    write_int(stream, HEADER_SIZE);
    write_int(stream, DIB_HEADER_SIZE);
    write_int(stream, width);
    write_int(stream, height);
    write_int(stream, BPP_AND_PLANES);
    write_int(stream, 0);
    write_int(stream, size);
    write_int(stream, PPM_RESOLUTION);
    write_int(stream, PPM_RESOLUTION);
    write_int(stream, 0);
    write_int(stream, 0);
}

class ImageImpl {
private:
    cairo_surface_t* m_surface;
    cairo_t* m_cr;

    void draw_note_circle(int x, int y, NoteColour note_colour);
    void draw_note_star(int x, int y, NoteColour note_colour);
    void draw_note_sustain(const ImageBuilder& builder,
                           const DrawnNote<NoteColour>& note);
    void draw_ghl_note(int x, int y,
                       const std::set<GHLNoteColour>& note_colours);
    void draw_ghl_note_sustain(const ImageBuilder& builder,
                               const DrawnNote<GHLNoteColour>& note);
    void draw_drum_note(int x, int y, DrumNoteColour note_colour);
    void draw_text_backwards(int x, int y, const char* text,
                             const Colour& colour, unsigned int font_height);
    void draw_vertical_lines(const ImageBuilder& builder,
                             const std::vector<double>& positions,
                             const Colour& colour);

public:
    ImageImpl(int width, int height)
    {
        m_surface
            = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
        m_cr = cairo_create(m_surface);

        cairo_set_source_rgb(m_cr, 1.0, 1.0, 1.0);
        cairo_rectangle(m_cr, 0, 0, width, height);
        cairo_fill(m_cr);

        cairo_set_line_width(m_cr, 1.0);
        cairo_select_font_face(m_cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
    }

    ~ImageImpl()
    {
        cairo_destroy(m_cr);
        cairo_surface_destroy(m_surface);
    }

    ImageImpl(const ImageImpl&) = delete;
    ImageImpl& operator=(const ImageImpl&) = delete;
    ImageImpl(ImageImpl&&) = delete;
    ImageImpl& operator=(ImageImpl&&) = delete;

    void colour_beat_range(const ImageBuilder& builder, const Colour& colour,
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

    void save(const std::string& path) const
    {
        if (path.size() < 4) {
            throw std::runtime_error("Unsupported file type");
        }
        const auto file_type = path.substr(path.size() - 4, 4);
        if (file_type == ".bmp") {
            cairo_surface_flush(m_surface);
            const auto* data = reinterpret_cast<char*>( // NOLINT
                cairo_image_surface_get_data(m_surface));
            const auto width = cairo_image_surface_get_width(m_surface);
            const auto height = cairo_image_surface_get_height(m_surface);
            const auto stride = cairo_image_surface_get_stride(m_surface);
            const auto row_padding
                = ((3 * width) % 4 == 0) ? 0 : (4 - ((3 * width) % 4));
            boost::nowide::ofstream out(path, std::ios::binary);
            write_24_bit_bmp_header(out, width, height);
            constexpr std::array<char, 3> zeroes {{0, 0, 0}};
            for (int i = height - 1; i >= 0; --i) {
                for (int j = 0; j < width; ++j) {
                    const auto* pixel = data + stride * i + 4 * j; // NOLINT
                    out.write(pixel, 3);
                }
                out.write(zeroes.data(), row_padding);
            }
        } else if (file_type == ".png") {
            const auto status
                = cairo_surface_write_to_png(m_surface, path.c_str());
            if (status != CAIRO_STATUS_SUCCESS) {
                throw std::runtime_error("Unable to write to png");
            }
        } else {
            throw std::runtime_error("Unsupported file type");
        }
    }
};

static std::tuple<int, int> get_xy(const ImageBuilder& builder, double pos)
{
    auto row = std::find_if(builder.rows().cbegin(), builder.rows().cend(),
                            [=](const auto x) { return x.end > pos; });
    auto x = LEFT_MARGIN + static_cast<int>(BEAT_WIDTH * (pos - row->start));
    auto y = TOP_MARGIN + MARGIN
        + DIST_BETWEEN_MEASURES
            * static_cast<int>(std::distance(builder.rows().cbegin(), row));
    return {x, y};
}

void ImageImpl::draw_header(const ImageBuilder& builder)
{
    constexpr int HEADER_FONT_HEIGHT = 23;

    const auto x = LEFT_MARGIN;
    const auto y = LEFT_MARGIN + HEADER_FONT_HEIGHT;

    cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
    cairo_set_font_size(m_cr, HEADER_FONT_HEIGHT);

    cairo_move_to(m_cr, x, y);
    cairo_show_text(m_cr, builder.song_name().c_str());

    cairo_move_to(m_cr, x, y + HEADER_FONT_HEIGHT);
    cairo_show_text(m_cr, builder.artist().c_str());

    cairo_move_to(m_cr, x, y + 2 * HEADER_FONT_HEIGHT);
    cairo_show_text(m_cr, builder.charter().c_str());

    cairo_move_to(m_cr, x, y + 3 * HEADER_FONT_HEIGHT);
    cairo_show_text(m_cr, "Total score = ");
    cairo_show_text(m_cr, std::to_string(builder.total_score()).c_str());
}

static int numb_of_fret_lines(TrackType track_type)
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

void ImageImpl::draw_measures(const ImageBuilder& builder)
{
    constexpr Colour BLACK {0.0, 0.0, 0.0};
    constexpr Colour GREY {0.627, 0.627, 0.627};
    constexpr Colour LIGHT_GREY {0.878, 0.878, 0.878};
    constexpr Colour RED {0.549, 0, 0};
    constexpr int MEASURE_NUMB_GAP = 5;

    const int fret_lines = numb_of_fret_lines(builder.track_type());
    const int colour_distance = (MEASURE_HEIGHT - 1) / fret_lines;

    draw_vertical_lines(builder, builder.beat_lines(), GREY);
    draw_vertical_lines(builder, builder.half_beat_lines(), LIGHT_GREY);

    auto current_row = 0;
    for (const auto& row : builder.rows()) {
        auto y = TOP_MARGIN + DIST_BETWEEN_MEASURES * current_row + MARGIN;
        const auto measure_width = BEAT_WIDTH * (row.end - row.start);
        for (int i = 1; i < fret_lines; ++i) {
            cairo_set_source_rgb(m_cr, GREY.red, GREY.green, GREY.blue);
            cairo_move_to(m_cr, LEFT_MARGIN + HALF_PIXEL,
                          y + colour_distance * i + HALF_PIXEL);
            cairo_rel_line_to(m_cr, measure_width, 0);
            cairo_stroke(m_cr);
        }
        cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
        cairo_rectangle(m_cr, LEFT_MARGIN + HALF_PIXEL, y + HALF_PIXEL,
                        measure_width, MEASURE_HEIGHT);
        cairo_stroke(m_cr);
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

    cairo_set_source_rgb(m_cr, RED.red, RED.green, RED.blue);
    cairo_set_font_size(m_cr, FONT_HEIGHT);
    for (std::size_t i = 0; i < builder.measure_lines().size() - 1; ++i) {
        auto pos = builder.measure_lines()[i];
        auto [x, y] = get_xy(builder, pos);
        y -= MEASURE_NUMB_GAP;
        cairo_move_to(m_cr, x, y);
        cairo_show_text(m_cr, std::to_string(i + 1).c_str());
    }
}

void ImageImpl::draw_vertical_lines(const ImageBuilder& builder,
                                    const std::vector<double>& positions,
                                    const Colour& colour)
{
    cairo_set_source_rgb(m_cr, colour.red, colour.green, colour.blue);

    for (auto pos : positions) {
        auto [x, y] = get_xy(builder, pos);
        cairo_move_to(m_cr, x + HALF_PIXEL, y + HALF_PIXEL);
        cairo_rel_line_to(m_cr, 0, MEASURE_HEIGHT);
        cairo_stroke(m_cr);
    }
}

void ImageImpl::draw_tempos(const ImageBuilder& builder)
{
    constexpr Colour GREY {0.627, 0.627, 0.627};
    constexpr int TEMPO_OFFSET = 18;

    cairo_set_source_rgb(m_cr, GREY.red, GREY.green, GREY.blue);
    cairo_set_font_size(m_cr, FONT_HEIGHT);

    for (const auto& [pos, tempo] : builder.bpms()) {
        auto [x, y] = get_xy(builder, pos);
        const auto text = std::string("♩=")
            + std::to_string(static_cast<int>(std::round(tempo)));
        cairo_move_to(m_cr, x + 1, y - TEMPO_OFFSET);
        cairo_show_text(m_cr, text.c_str());
    }
}

void ImageImpl::draw_time_sigs(const ImageBuilder& builder)
{
    constexpr Colour GREY {0.627, 0.627, 0.627};
    constexpr int HALF_MEASURE_HEIGHT = MEASURE_HEIGHT / 2;
    constexpr int TS_FONT_HEIGHT = 36;
    constexpr int TS_X_GAP = 3;
    constexpr int TS_Y_GAP = 2;

    cairo_set_source_rgb(m_cr, GREY.red, GREY.green, GREY.blue);
    cairo_set_font_size(m_cr, TS_FONT_HEIGHT);

    for (const auto& [pos, num, denom] : builder.time_sigs()) {
        auto [x, y] = get_xy(builder, pos);
        cairo_move_to(m_cr, x + TS_X_GAP, y + HALF_MEASURE_HEIGHT - TS_Y_GAP);
        cairo_show_text(m_cr, std::to_string(num).c_str());
        cairo_move_to(m_cr, x + TS_X_GAP, y + MEASURE_HEIGHT - TS_Y_GAP);
        cairo_show_text(m_cr, std::to_string(denom).c_str());
    }
}

void ImageImpl::draw_score_totals(const ImageBuilder& builder)
{
    constexpr Colour CYAN {0.0, 0.627, 0.627};
    constexpr Colour GREEN {0, 0.392, 0};
    constexpr Colour GREY {0.627, 0.627, 0.627};

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
        draw_text_backwards(x, y, text.c_str(), GREY, FONT_HEIGHT);
        text = std::to_string(score_values[i]);
        y += VALUE_GAP;
        draw_text_backwards(x, y, text.c_str(), GREEN, FONT_HEIGHT);
        if (sp_values[i] > 0) {
            y += VALUE_GAP;
            std::snprintf(buffer.data(), BUFFER_SIZE, "%.2fSP", sp_values[i]);
            draw_text_backwards(x, y, buffer.data(), CYAN, FONT_HEIGHT);
        }
    }
}

void ImageImpl::draw_text_backwards(int x, int y, const char* text,
                                    const Colour& colour,
                                    unsigned int font_height)
{
    cairo_text_extents_t te;
    cairo_set_source_rgb(m_cr, colour.red, colour.green, colour.blue);
    cairo_set_font_size(m_cr, font_height);
    cairo_text_extents(m_cr, text, &te);
    cairo_move_to(m_cr, x - te.width, y + te.height);
    cairo_show_text(m_cr, text);
}

void ImageImpl::draw_notes(const ImageBuilder& builder)
{
    for (const auto& note : builder.notes()) {
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
        draw_ghl_note(x, y, colours);
    }
}

enum class DrumSpriteShape { Kick, Cymbal, Tom };

static DrumSpriteShape drum_colour_to_shape(DrumNoteColour colour)
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

void ImageImpl::draw_drum_notes(const ImageBuilder& builder)
{
    // We draw all the kicks first because we want RYBG to lie on top of the
    // kicks, not underneath.
    for (const auto& note : builder.drum_notes()) {
        if (drum_colour_to_shape(note.colour) != DrumSpriteShape::Kick) {
            continue;
        }
        const auto [x, y] = get_xy(builder, note.beat);
        draw_drum_note(x, y, note.colour);
    }

    for (const auto& note : builder.drum_notes()) {
        if (drum_colour_to_shape(note.colour) == DrumSpriteShape::Kick) {
            continue;
        }
        const auto [x, y] = get_xy(builder, note.beat);
        draw_drum_note(x, y, note.colour);
    }
}

// Codes are
// 0 - No note
// 1 - White note
// 2 - Black note
// 3 - White and black note
static std::array<int, 3>
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

void ImageImpl::draw_note_circle(int x, int y, NoteColour note_colour)
{
    constexpr int OPEN_WIDTH = 6;
    constexpr int RADIUS = 5;

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    if (note_colour == NoteColour::Open) {
        cairo_rectangle(m_cr, x - 3 + HALF_PIXEL, y - 3 + HALF_PIXEL,
                        OPEN_WIDTH, MEASURE_HEIGHT + OPEN_WIDTH);
        cairo_set_source_rgba(m_cr, colour.red, colour.green, colour.blue,
                              OPEN_NOTE_OPACITY);
    } else {
        cairo_new_sub_path(m_cr);
        cairo_arc(m_cr, x, y + offset, RADIUS, 0, TWO_PI);
        cairo_set_source_rgb(m_cr, colour.red, colour.green, colour.blue);
    }

    cairo_fill_preserve(m_cr);
    cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
    cairo_stroke(m_cr);
}

void ImageImpl::draw_ghl_note(int x, int y,
                              const std::set<GHLNoteColour>& note_colours)
{
    constexpr Colour DARK_GREY {0.118, 0.118, 0.118};
    constexpr int FRET_GAP = 30;
    constexpr int OPEN_WIDTH = 6;
    constexpr int RADIUS = 5;

    if (note_colours.count(GHLNoteColour::Open) != 0) {
        cairo_rectangle(m_cr, x - 3 + HALF_PIXEL, y - 3 + HALF_PIXEL,
                        OPEN_WIDTH, MEASURE_HEIGHT + OPEN_WIDTH);
        cairo_set_source_rgba(m_cr, 1.0, 1.0, 1.0, OPEN_NOTE_OPACITY);
        cairo_fill_preserve(m_cr);
        cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
        cairo_stroke(m_cr);
        return;
    }

    const auto codes = ghl_note_colour_codes(note_colours);

    for (auto i = 0U; i < 3; ++i) {
        auto offset = FRET_GAP * static_cast<int>(i);
        if (codes.at(i) == 0) {
            continue;
        }
        if (codes.at(i) == 1) {
            cairo_new_sub_path(m_cr);
            cairo_arc(m_cr, x, y + offset, RADIUS, 0, TWO_PI);
            cairo_set_source_rgb(m_cr, 1.0, 1.0, 1.0);
        } else if (codes.at(i) == 2) {
            cairo_new_sub_path(m_cr);
            cairo_arc(m_cr, x, y + offset, RADIUS, 0, TWO_PI);
            cairo_set_source_rgb(m_cr, DARK_GREY.red, DARK_GREY.green,
                                 DARK_GREY.blue);
        } else if (codes.at(i) == 3) {
            cairo_rectangle(m_cr, x - RADIUS + HALF_PIXEL,
                            y + offset - RADIUS + HALF_PIXEL, 2 * RADIUS,
                            RADIUS);
            cairo_set_source_rgb(m_cr, DARK_GREY.red, DARK_GREY.green,
                                 DARK_GREY.blue);
            cairo_fill(m_cr);
            cairo_rectangle(m_cr, x - RADIUS + HALF_PIXEL,
                            y + offset + HALF_PIXEL, 2 * RADIUS, RADIUS);
            cairo_set_source_rgb(m_cr, 1.0, 1.0, 1.0);
        }
        cairo_fill_preserve(m_cr);
        cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
        cairo_stroke(m_cr);
    }
}

void ImageImpl::draw_drum_note(int x, int y, DrumNoteColour note_colour)
{
    constexpr int OPEN_WIDTH = 6;
    constexpr int RADIUS = 5;

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    switch (drum_colour_to_shape(note_colour)) {
    case DrumSpriteShape::Kick:
        cairo_rectangle(m_cr, x - 3 + HALF_PIXEL, y - 3 + HALF_PIXEL,
                        OPEN_WIDTH, MEASURE_HEIGHT + OPEN_WIDTH);
        cairo_set_source_rgba(m_cr, colour.red, colour.green, colour.blue,
                              OPEN_NOTE_OPACITY);
        break;
    case DrumSpriteShape::Cymbal:
        cairo_move_to(m_cr, x + HALF_PIXEL, y + offset - RADIUS + HALF_PIXEL);
        cairo_rel_line_to(m_cr, -RADIUS, 2 * RADIUS);
        cairo_rel_line_to(m_cr, 2 * RADIUS, 0);
        cairo_close_path(m_cr);
        cairo_set_source_rgb(m_cr, colour.red, colour.green, colour.blue);
        break;
    case DrumSpriteShape::Tom:
        cairo_new_sub_path(m_cr);
        cairo_arc(m_cr, x, y + offset, RADIUS, 0, TWO_PI);
        cairo_set_source_rgb(m_cr, colour.red, colour.green, colour.blue);
    }

    cairo_fill_preserve(m_cr);
    cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
    cairo_stroke(m_cr);
}

void ImageImpl::draw_note_star(int x, int y, NoteColour note_colour)
{
    constexpr int OPEN_WIDTH = 6;
    constexpr int STAR_RADIUS = 6;

    const auto colour = note_colour_to_colour(note_colour);
    const auto offset = note_colour_to_offset(note_colour);

    if (note_colour == NoteColour::Open) {
        cairo_rectangle(m_cr, x - 3 + HALF_PIXEL, y - 3 + HALF_PIXEL,
                        OPEN_WIDTH, MEASURE_HEIGHT + OPEN_WIDTH);
        cairo_set_source_rgba(m_cr, colour.red, colour.green, colour.blue,
                              OPEN_NOTE_OPACITY);
    } else {
        cairo_move_to(m_cr, x + HALF_PIXEL,
                      y + offset - STAR_RADIUS + HALF_PIXEL);
        cairo_rel_line_to(m_cr, 1, 4);
        cairo_rel_line_to(m_cr, 4, 0);
        cairo_rel_line_to(m_cr, -3, 3);
        cairo_rel_line_to(m_cr, 1, 4);
        cairo_rel_line_to(m_cr, -3, -3);
        cairo_rel_line_to(m_cr, -3, 3);
        cairo_rel_line_to(m_cr, 1, -4);
        cairo_rel_line_to(m_cr, -3, -3);
        cairo_rel_line_to(m_cr, 4, 0);
        cairo_close_path(m_cr);
        cairo_set_source_rgb(m_cr, colour.red, colour.green, colour.blue);
    }

    cairo_fill_preserve(m_cr);
    cairo_set_source_rgb(m_cr, 0.0, 0.0, 0.0);
    cairo_stroke(m_cr);
}

void ImageImpl::draw_note_sustain(const ImageBuilder& builder,
                                  const DrawnNote<NoteColour>& note)
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

void ImageImpl::draw_ghl_note_sustain(const ImageBuilder& builder,
                                      const DrawnNote<GHLNoteColour>& note)
{
    constexpr std::tuple<int, int> OPEN_NOTE_Y_RANGE {7, 53};
    constexpr Colour SUST_COLOUR {0.588, 0.588, 0.588};

    std::tuple<double, double> x_range {note.beat, note.beat + note.length};
    auto offset = note_colour_to_offset(note.colour);
    std::tuple<int, int> y_range {offset - 3, offset + 3};
    float opacity = 1.0F;
    if (note.colour == GHLNoteColour::Open) {
        y_range = OPEN_NOTE_Y_RANGE;
        opacity = OPEN_NOTE_OPACITY;
    }
    colour_beat_range(builder, SUST_COLOUR, x_range, y_range, opacity);
}

void ImageImpl::colour_beat_range(const ImageBuilder& builder,
                                  const Colour& colour,
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
        const auto block_end = std::min(row_iter->end, end);
        const auto x_min = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (start - row_iter->start));
        // -1 is so regions that cross rows do not go over the ending line of a
        // row.
        const auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (block_end - row_iter->start));
        if (x_min <= x_max) {
            const auto y = TOP_MARGIN + MARGIN + DIST_BETWEEN_MEASURES * row;
            cairo_rectangle(m_cr, x_min, y + y_min, x_max - x_min,
                            y_max - y_min);
            cairo_set_source_rgba(m_cr, colour.red, colour.green, colour.blue,
                                  opacity);
            cairo_fill(m_cr);
        }
        start = block_end;
        ++row_iter;
        ++row;
    }
}

Image::Image(const ImageBuilder& builder)
{
    constexpr Colour green {0.0, 1.0, 0.0};
    constexpr Colour blue {0.0, 0.0, 1.0};
    constexpr Colour yellow {1.0, 1.0, 0};
    constexpr Colour red {1.0, 0, 0};
    constexpr Colour solo_blue {0, 0.2, 0.5};
    constexpr Colour pink {0.5, 0, 0};

    constexpr unsigned int IMAGE_WIDTH = 1024;
    constexpr float RANGE_OPACITY = 0.33333F;
    constexpr int SOLO_HEIGHT = 10;

    const auto height = static_cast<unsigned int>(
        TOP_MARGIN + MARGIN + DIST_BETWEEN_MEASURES * builder.rows().size());

    m_impl = std::make_unique<ImageImpl>(IMAGE_WIDTH, height);
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
