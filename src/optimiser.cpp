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
#include <cassert>

#include "optimiser.hpp"

TimeConverter::TimeConverter(const SyncTrack& sync_track,
                             const SongHeader& header)
{
    auto last_tick = 0U;
    auto last_bpm = DEFAULT_BPM;
    auto last_time = 0.0;

    for (const auto& bpm : sync_track.bpms()) {
        last_time += ((bpm.position - last_tick) * MS_PER_MINUTE)
            / (static_cast<uint32_t>(header.resolution()) * last_bpm);
        const auto beat
            = static_cast<double>(bpm.position) / header.resolution();
        m_beat_timestamps.push_back({beat, last_time});
        last_bpm = bpm.bpm;
        last_tick = bpm.position;
    }

    m_last_bpm = last_bpm;

    assert(!m_beat_timestamps.empty());
}

double TimeConverter::beats_to_seconds(double beats) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), beats,
        [](const auto& x, const auto& y) { return x.beat < y; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return back.time + ((beats - back.beat) * MS_PER_MINUTE) / m_last_bpm;
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return pos->time - ((pos->beat - beats) * MS_PER_MINUTE) / DEFAULT_BPM;
    }
    const auto prev = pos - 1;
    return prev->time
        + (pos->time - prev->time) * (beats - prev->beat)
        / (pos->beat - prev->beat);
}

double TimeConverter::seconds_to_beats(double seconds) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), seconds,
        [](const auto& x, const auto& y) { return x.time < y; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return back.beat + ((seconds - back.time) * m_last_bpm) / MS_PER_MINUTE;
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return pos->beat
            - ((pos->time - seconds) * DEFAULT_BPM) / MS_PER_MINUTE;
    }
    const auto prev = pos - 1;
    return prev->beat
        + (pos->beat - prev->beat) * (seconds - prev->time)
        / (pos->time - prev->time);
}

std::vector<Point> notes_to_points(const NoteTrack& track,
                                   const SongHeader& header)
{
    constexpr auto NOTE_VALUE = 50U;
    const auto tick_gap = std::max(header.resolution() / 25, 1);
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