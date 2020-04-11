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

#ifndef CHOPT_IMAGE_HPP
#define CHOPT_IMAGE_HPP

#include <cstdint>
#include <vector>

#include "cimg_wrapper.hpp"

#include "chart.hpp"

struct DrawnNote {
    double beat;
    NoteColour colour;
};

struct DrawingInstructions {
    std::vector<DrawnNote> notes;
    int number_of_rows;
};

DrawingInstructions create_instructions(const NoteTrack& track,
                                        std::int32_t resolution);

using Image = cimg_library::CImg<unsigned char>;

Image create_path_image(const DrawingInstructions& instructions);

#endif
