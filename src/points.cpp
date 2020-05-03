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

static bool phrase_contains_pos(const StarPower& phrase, int position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

template <class InputIt, class OutputIt>
static void append_note_points(InputIt first, InputIt last, OutputIt points,
                               int resolution, bool is_note_sp_ender,
                               const TimeConverter& converter, double squeeze)
{
    constexpr double HALF_RES_OFFSET = 0.5;
    constexpr int NOTE_VALUE = 50;
    const auto EARLY_WINDOW = Second(0.07 * squeeze);
    const auto LATE_WINDOW = Second(0.07 * squeeze);

    const double float_res = resolution;
    const auto tick_gap = std::max(resolution / 25, 1);

    const auto chord_size = static_cast<int>(std::distance(first, last));
    auto chord_length
        = std::max_element(first, last, [](const auto& x, const auto& y) {
              return x.length < y.length;
          })->length;
    auto pos = first->position;
    auto beat = Beat(pos / float_res);
    auto meas = converter.beats_to_measures(beat);
    auto note_seconds = converter.beats_to_seconds(beat);
    auto early_beat = converter.seconds_to_beats(note_seconds - EARLY_WINDOW);
    auto early_meas = converter.beats_to_measures(early_beat);
    auto late_beat = converter.seconds_to_beats(note_seconds + LATE_WINDOW);
    auto late_meas = converter.beats_to_measures(late_beat);
    *points++ = {{beat, meas},
                 {early_beat, early_meas},
                 {late_beat, late_meas},
                 NOTE_VALUE * chord_size,
                 NOTE_VALUE * chord_size,
                 false,
                 is_note_sp_ender};
    auto sust_ticks = (chord_length + tick_gap - 1) / tick_gap;
    while (chord_length > (resolution / 4)) {
        pos += tick_gap;
        chord_length -= tick_gap;
        beat = Beat((pos - HALF_RES_OFFSET) / float_res);
        meas = converter.beats_to_measures(beat);
        --sust_ticks;
        *points++
            = {{beat, meas}, {beat, meas}, {beat, meas}, 1, 1, true, false};
    }
    if (sust_ticks > 0) {
        beat = Beat((pos + HALF_RES_OFFSET) / float_res);
        meas = converter.beats_to_measures(beat);
        *points++ = {{beat, meas}, {beat, meas}, {beat, meas}, sust_ticks,
                     sust_ticks,   true,         false};
    }
}

PointSet::PointSet(const NoteTrack& track, int resolution,
                   const TimeConverter& converter, double squeeze)
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
                           is_note_sp_ender, converter, squeeze);
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

    m_solo_boosts.reserve(track.solos().size());
    for (const auto& solo : track.solos()) {
        Beat end_beat {solo.end / static_cast<double>(resolution)};
        Measure end_meas = converter.beats_to_measures(end_beat);
        Position end_pos {end_beat, end_meas};
        m_solo_boosts.emplace_back(end_pos, solo.value);
    }
}
