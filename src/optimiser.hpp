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

#ifndef CHOPT_OPTIMISER_HPP
#define CHOPT_OPTIMISER_HPP

#include <cstdint>
#include <vector>

#include "chart.hpp"

struct Point {
    uint32_t position;
    uint32_t value;

    friend bool operator==(const Point& lhs, const Point& rhs)
    {
        return std::tie(lhs.position, lhs.value)
            == std::tie(rhs.position, rhs.value);
    }
};

std::vector<Point> notes_to_points(const NoteTrack& track,
                                   const SongHeader& header);

#endif
