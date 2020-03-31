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

#include "time.hpp"

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
        m_beat_timestamps.push_back({{beat}, {last_time}});
        last_bpm = bpm.bpm;
        last_tick = bpm.position;
    }

    m_last_bpm = last_bpm;

    last_tick = 0U;
    auto last_beat_rate = DEFAULT_BEAT_RATE;
    auto last_measure = 0.0;

    for (const auto& ts : sync_track.time_sigs()) {
        last_measure += (ts.position - last_tick)
            / (header.resolution() * last_beat_rate);
        const auto beat
            = static_cast<double>(ts.position) / header.resolution();
        m_measure_timestamps.push_back({{last_measure}, {beat}});
        last_beat_rate = (ts.numerator * DEFAULT_BEAT_RATE) / ts.denominator;
        last_tick = ts.position;
    }

    m_last_beat_rate = last_beat_rate;

    assert(!m_beat_timestamps.empty()); // NOLINT
    assert(!m_measure_timestamps.empty()); // NOLINT
}

Second TimeConverter::beats_to_seconds(Beat beats) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), beats,
        [](const auto& x, const auto& y) { return x.beat.value < y.value; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return {back.time.value
                + ((beats.value - back.beat.value) * MS_PER_MINUTE)
                    / m_last_bpm};
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return {pos->time.value
                - ((pos->beat.value - beats.value) * MS_PER_MINUTE)
                    / DEFAULT_BPM};
    }
    const auto prev = pos - 1;
    return {prev->time.value
            + (pos->time.value - prev->time.value)
                * (beats.value - prev->beat.value)
                / (pos->beat.value - prev->beat.value)};
}

Beat TimeConverter::seconds_to_beats(Second seconds) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), seconds,
        [](const auto& x, const auto& y) { return x.time.value < y.value; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return {back.beat.value
                + ((seconds.value - back.time.value) * m_last_bpm)
                    / MS_PER_MINUTE};
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return {pos->beat.value
                - ((pos->time.value - seconds.value) * DEFAULT_BPM)
                    / MS_PER_MINUTE};
    }
    const auto prev = pos - 1;
    return {prev->beat.value
            + (pos->beat.value - prev->beat.value)
                * (seconds.value - prev->time.value)
                / (pos->time.value - prev->time.value)};
}

Measure TimeConverter::beats_to_measures(Beat beats) const
{
    const auto pos = std::lower_bound(
        m_measure_timestamps.cbegin(), m_measure_timestamps.cend(), beats,
        [](const auto& x, const auto& y) { return x.beat.value < y.value; });
    if (pos == m_measure_timestamps.cend()) {
        const auto& back = m_measure_timestamps.back();
        return {back.measure.value
                + (beats.value - back.beat.value) / m_last_beat_rate};
    }
    if (pos == m_measure_timestamps.cbegin()) {
        return {pos->measure.value
                - (pos->beat.value - beats.value) / DEFAULT_BEAT_RATE};
    }
    const auto prev = pos - 1;
    return {prev->measure.value
            + (pos->measure.value - prev->measure.value)
                * (beats.value - prev->beat.value)
                / (pos->beat.value - prev->beat.value)};
}

Beat TimeConverter::measures_to_beats(Measure measures) const
{
    const auto pos = std::lower_bound(
        m_measure_timestamps.cbegin(), m_measure_timestamps.cend(), measures,
        [](const auto& x, const auto& y) { return x.measure.value < y.value; });
    if (pos == m_measure_timestamps.cend()) {
        const auto& back = m_measure_timestamps.back();
        return {back.beat.value
                + (measures.value - back.measure.value) * m_last_beat_rate};
    }
    if (pos == m_measure_timestamps.cbegin()) {
        return {pos->beat.value
                - (pos->measure.value - measures.value) * DEFAULT_BEAT_RATE};
    }
    const auto prev = pos - 1;
    return {prev->beat.value
            + (pos->beat.value - prev->beat.value)
                * (measures.value - prev->measure.value)
                / (pos->measure.value - prev->measure.value)};
}

Second TimeConverter::measures_to_seconds(Measure measures) const
{
    return beats_to_seconds(measures_to_beats(measures));
}

Measure TimeConverter::seconds_to_measures(Second seconds) const
{
    return beats_to_measures(seconds_to_beats(seconds));
}
