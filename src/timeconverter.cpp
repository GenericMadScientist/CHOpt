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
    if (!engine.uses_beat_track()) {
        Tick last_tick {0};
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

    assert(!m_measure_timestamps.empty()); // NOLINT
}

Second TimeConverter::measures_to_seconds(Measure measures) const
{
    return m_tempo_map.to_seconds(m_tempo_map.to_beats(measures));
}

Measure TimeConverter::seconds_to_measures(Second seconds) const
{
    return m_tempo_map.to_measures(m_tempo_map.to_beats(seconds));
}
