/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023 Raymond Wright
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

#include <cassert>

#include "timeconverter.hpp"

TimeConverter::TimeConverter(const TempoMap& tempo_map, const Engine& engine,
                             const std::vector<Tick>& od_beats)
    : m_tempo_map {tempo_map}
{
    constexpr double MS_PER_MINUTE = 60000.0;

    Tick last_tick {0};
    auto last_bpm = DEFAULT_BPM;
    auto last_time = 0.0;

    for (const auto& bpm : tempo_map.bpms()) {
        last_time += tempo_map.to_beats(bpm.position - last_tick).value()
            * (MS_PER_MINUTE / static_cast<double>(last_bpm));
        const auto beat = tempo_map.to_beats(bpm.position);
        m_beat_timestamps.push_back({beat, Second(last_time)});
        last_bpm = bpm.bpm;
        last_tick = bpm.position;
    }

    m_last_bpm = last_bpm;

    if (!engine.uses_beat_track()) {
        last_tick = Tick {0};
        auto last_beat_rate = DEFAULT_BEAT_RATE;
        auto last_measure = 0.0;

        for (const auto& ts : tempo_map.time_sigs()) {
            last_measure += tempo_map.to_beats(ts.position - last_tick).value()
                / static_cast<double>(last_beat_rate);
            const auto beat = tempo_map.to_beats(ts.position);
            m_measure_timestamps.push_back({Measure(last_measure), beat});
            last_beat_rate
                = (ts.numerator * DEFAULT_BEAT_RATE) / ts.denominator;
            last_tick = ts.position;
        }

        m_last_beat_rate = last_beat_rate;
    } else if (!od_beats.empty()) {
        for (auto i = 0U; i < od_beats.size(); ++i) {
            const auto beat = tempo_map.to_beats(od_beats[i]);
            m_measure_timestamps.push_back(
                {Measure(i / DEFAULT_BEAT_RATE), beat});
        }

        m_last_beat_rate = DEFAULT_BEAT_RATE;
    } else {
        m_measure_timestamps.push_back({Measure(0.0), Beat(0.0)});
        m_last_beat_rate = DEFAULT_BEAT_RATE;
    }

    assert(!m_beat_timestamps.empty()); // NOLINT
    assert(!m_measure_timestamps.empty()); // NOLINT
}

Measure TimeConverter::beats_to_measures(Beat beats) const
{
    const auto pos = std::lower_bound(
        m_measure_timestamps.cbegin(), m_measure_timestamps.cend(), beats,
        [](const auto& x, const auto& y) { return x.beat < y; });
    if (pos == m_measure_timestamps.cend()) {
        const auto& back = m_measure_timestamps.back();
        return back.measure + (beats - back.beat).to_measure(m_last_beat_rate);
    }
    if (pos == m_measure_timestamps.cbegin()) {
        return pos->measure - (pos->beat - beats).to_measure(DEFAULT_BEAT_RATE);
    }
    const auto prev = pos - 1;
    return prev->measure
        + (pos->measure - prev->measure)
        * ((beats - prev->beat) / (pos->beat - prev->beat));
}

Beat TimeConverter::measures_to_beats(Measure measures) const
{
    const auto pos = std::lower_bound(
        m_measure_timestamps.cbegin(), m_measure_timestamps.cend(), measures,
        [](const auto& x, const auto& y) { return x.measure < y; });
    if (pos == m_measure_timestamps.cend()) {
        const auto& back = m_measure_timestamps.back();
        return back.beat + (measures - back.measure).to_beat(m_last_beat_rate);
    }
    if (pos == m_measure_timestamps.cbegin()) {
        return pos->beat - (pos->measure - measures).to_beat(DEFAULT_BEAT_RATE);
    }
    const auto prev = pos - 1;
    return prev->beat
        + (pos->beat - prev->beat)
        * ((measures - prev->measure) / (pos->measure - prev->measure));
}

Second TimeConverter::measures_to_seconds(Measure measures) const
{
    return m_tempo_map.to_seconds(measures_to_beats(measures));
}

Measure TimeConverter::seconds_to_measures(Second seconds) const
{
    return beats_to_measures(m_tempo_map.to_beats(seconds));
}
