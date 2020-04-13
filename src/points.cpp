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

#include "points.hpp"

static bool phrase_contains_pos(const StarPower& phrase, std::uint32_t position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

template <class InputIt, class OutputIt>
static void append_note_points(InputIt first, InputIt last, OutputIt points,
                               std::int32_t resolution, bool is_note_sp_ender,
                               const TimeConverter& converter)
{
    constexpr auto NOTE_VALUE = 50U;
    const double float_res = resolution;
    const auto tick_gap = std::max(resolution / 25, 1);

    const auto chord_size
        = static_cast<std::uint32_t>(std::distance(first, last));
    auto chord_length = static_cast<std::int32_t>(
        std::max_element(first, last, [](const auto& x, const auto& y) {
            return x.length < y.length;
        })->length);
    auto pos = first->position;
    auto beat = Beat(pos / float_res);
    auto meas = converter.beats_to_measures(beat);
    *points++
        = {{beat, meas}, NOTE_VALUE * chord_size, false, is_note_sp_ender};
    while (chord_length > 0) {
        pos += static_cast<std::uint32_t>(tick_gap);
        chord_length -= tick_gap;
        beat = Beat(pos / float_res);
        meas = converter.beats_to_measures(beat);
        *points++ = {{beat, meas}, 1, true, false};
    }
}

PointSet::PointSet(const NoteTrack& track, std::int32_t resolution,
                   const TimeConverter& converter)
{
    const auto& notes = track.notes();

    auto current_phrase = track.sp_phrases().cbegin();
    for (auto p = notes.cbegin(); p != notes.cend();) {
        const auto q = std::find_if_not(p, notes.cend(), [=](const auto& x) {
            return x.position == p->position;
        });
        auto is_note_sp_ender = false;
        if (current_phrase != track.sp_phrases().cend()
            && phrase_contains_pos(*current_phrase, p->position)
            && ((q == notes.cend())
                || !phrase_contains_pos(*current_phrase, q->position))) {
            is_note_sp_ender = true;
            ++current_phrase;
        }
        append_note_points(p, q, std::back_inserter(m_points), resolution,
                           is_note_sp_ender, converter);
        p = q;
    }

    std::stable_sort(m_points.begin(), m_points.end(),
                     [](const auto& x, const auto& y) {
                         return x.position.beat < y.position.beat;
                     });

    auto combo = 0U;
    for (auto& point : m_points) {
        if (!point.is_hold_point) {
            ++combo;
        }
        const auto multiplier = 1 + std::min(combo / 10, 3U);
        point.value *= multiplier;
    }
}

Beat hit_window_start(const Point& point, const TimeConverter& converter)
{
    constexpr double FRONT_END = 0.07;

    if (point.is_hold_point) {
        return point.position.beat;
    }

    auto time = converter.beats_to_seconds(point.position.beat).value();
    time -= FRONT_END;
    return converter.seconds_to_beats(Second(time));
}

Beat hit_window_end(const Point& point, const TimeConverter& converter)
{
    constexpr double BACK_END = 0.07;

    if (point.is_hold_point) {
        return point.position.beat;
    }

    auto time = converter.beats_to_seconds(point.position.beat).value();
    time += BACK_END;
    return converter.seconds_to_beats(Second(time));
}
