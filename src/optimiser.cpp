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

#include "optimiser.hpp"

std::vector<Point> notes_to_points(const NoteTrack& track, int32_t resolution)
{
    constexpr auto NOTE_VALUE = 50U;
    const auto tick_gap = std::max(resolution / 25, 1);
    std::vector<Point> points;

    const auto& notes = track.notes();

    if (!notes.empty()) {
        auto current_position = notes[0].position;
        auto chord_size = 1U;
        auto chord_length = static_cast<int32_t>(notes[0].length);

        for (auto p = notes.cbegin() + 1; p < notes.cend(); ++p) {
            if (p->position == current_position) {
                ++chord_size;
                chord_length
                    = std::max(chord_length, static_cast<int32_t>(p->length));
            } else {
                points.push_back({current_position, NOTE_VALUE * chord_size});
                while (chord_length > 0) {
                    current_position += static_cast<uint32_t>(tick_gap);
                    chord_length -= tick_gap;
                    points.push_back({current_position, 1});
                }
                chord_size = 1;
                chord_length = static_cast<int32_t>(p->length);
                current_position = p->position;
            }
        }

        points.push_back({current_position, NOTE_VALUE * chord_size});
        while (chord_length > 0) {
            current_position += static_cast<uint32_t>(tick_gap);
            chord_length -= tick_gap;
            points.push_back({current_position, 1});
        }
    }

    std::stable_sort(
        points.begin(), points.end(),
        [](const auto& x, const auto& y) { return x.position < y.position; });

    return points;
}
