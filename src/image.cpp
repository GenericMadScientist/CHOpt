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
constexpr int BEATS_PER_LINE = 16;
constexpr int LEFT_MARGIN = 31;
constexpr int MARGIN = 32;
constexpr int MEASURE_HEIGHT = 61;
constexpr int DIST_BETWEEN_MEASURES = MEASURE_HEIGHT + MARGIN;

DrawingInstructions create_instructions(const NoteTrack& track,
                                        std::int32_t resolution)
{
    std::vector<DrawnNote> notes;
    int max_pos = 0;

    for (const auto& note : track.notes()) {
        auto beat = note.position / static_cast<double>(resolution);
        notes.push_back({beat, note.colour});
        max_pos = std::max(max_pos, static_cast<int>(note.position));
    }

    const auto res_per_row = BEATS_PER_LINE * resolution;
    const auto number_of_rows = 1 + max_pos / res_per_row;

    return {notes, number_of_rows};
}

static void draw_measures(Image& image, int num_of_lines)
{
    constexpr int LINE_WIDTH = 960;
    constexpr std::array<unsigned char, 3> black {0, 0, 0};

    for (auto i = 0; i < num_of_lines; ++i) {
        auto y = DIST_BETWEEN_MEASURES * i + MARGIN;
        image.draw_rectangle(LEFT_MARGIN, y, LEFT_MARGIN + LINE_WIDTH,
                             y + MEASURE_HEIGHT, black.data(), 1.0, ~0U);
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
        MARGIN + DIST_BETWEEN_MEASURES * instructions.number_of_rows);

    CImg<unsigned char> image(IMAGE_WIDTH, height, 1, 3, WHITE);
    draw_measures(image, instructions.number_of_rows);

    for (const auto& note : instructions.notes) {
        auto beats_along_row = std::fmod(note.beat, BEATS_PER_LINE);
        auto row = static_cast<int>(note.beat / BEATS_PER_LINE);
        auto x = LEFT_MARGIN + static_cast<int>(beats_along_row * BEAT_WIDTH);
        auto y = MARGIN + (DIST_BETWEEN_MEASURES * row);
        draw_note(image, x, y, note.colour);
    }

    return image;
}
