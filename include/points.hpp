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

#ifndef CHOPT_POINTS_HPP
#define CHOPT_POINTS_HPP

#include <tuple>
#include <vector>

#include "chart.hpp"
#include "time.hpp"

struct Point {
    Position position;
    Position hit_window_start;
    Position hit_window_end;
    int value;
    int base_value;
    bool is_hold_point;
    bool is_sp_granting_note;
};

using PointPtr = std::vector<Point>::const_iterator;

class PointSet {
private:
    std::vector<Point> m_points;
    std::vector<std::tuple<Position, int>> m_solo_boosts;

public:
    PointSet(const NoteTrack<NoteColour>& track, int resolution,
             const TimeConverter& converter, double squeeze);
    PointSet(const NoteTrack<GHLNoteColour>& track, int resolution,
             const TimeConverter& converter, double squeeze);
    [[nodiscard]] PointPtr cbegin() const { return m_points.cbegin(); }
    [[nodiscard]] PointPtr cend() const { return m_points.cend(); }
    [[nodiscard]] const std::vector<std::tuple<Position, int>>&
    solo_boosts() const
    {
        return m_solo_boosts;
    }
};

#endif
