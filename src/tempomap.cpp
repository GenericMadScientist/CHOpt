/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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

#include "tempomap.hpp"

TempoMap::TempoMap(std::vector<TimeSignature> time_sigs, std::vector<BPM> bpms,
                   int resolution)
    : m_resolution {resolution}
{
    constexpr double MS_PER_MINUTE = 60000.0;

    if (resolution <= 0) {
        throw std::invalid_argument("Resolution must be positive");
    }

    for (const auto& bpm : bpms) {
        if (bpm.bpm <= 0) {
            throw ParseError("BPMs must be positive");
        }
    }
    for (const auto& ts : time_sigs) {
        if (ts.numerator <= 0 || ts.denominator <= 0) {
            throw ParseError("Time signatures must be positive/positive");
        }
    }

    std::stable_sort(
        bpms.begin(), bpms.end(),
        [](const auto& x, const auto& y) { return x.position < y.position; });
    BPM prev_bpm {Tick {0}, DEFAULT_BPM};
    for (auto p = bpms.cbegin(); p < bpms.cend(); ++p) {
        if (p->position != prev_bpm.position) {
            m_bpms.push_back(prev_bpm);
        }
        prev_bpm = *p;
    }
    m_bpms.push_back(prev_bpm);

    std::stable_sort(
        time_sigs.begin(), time_sigs.end(),
        [](const auto& x, const auto& y) { return x.position < y.position; });
    TimeSignature prev_ts {Tick {0}, 4, 4};
    for (auto p = time_sigs.cbegin(); p < time_sigs.cend(); ++p) {
        if (p->position != prev_ts.position) {
            m_time_sigs.push_back(prev_ts);
        }
        prev_ts = *p;
    }
    m_time_sigs.push_back(prev_ts);

    Tick last_tick {0};
    auto last_bpm = DEFAULT_BPM;
    auto last_time = 0.0;

    for (const auto& bpm : m_bpms) {
        last_time += to_beat(bpm.position - last_tick).value()
            * (MS_PER_MINUTE / static_cast<double>(last_bpm));
        const auto beat = to_beat(bpm.position);
        m_beat_timestamps.push_back({beat, Second(last_time)});
        last_bpm = bpm.bpm;
        last_tick = bpm.position;
    }

    m_last_bpm = last_bpm;
}

TempoMap TempoMap::speedup(int speed) const
{
    constexpr auto DEFAULT_SPEED = 100;

    TempoMap speedup {m_time_sigs, m_bpms, m_resolution};
    for (auto& bpm : speedup.m_bpms) {
        bpm.bpm = (bpm.bpm * speed) / DEFAULT_SPEED;
    }

    const auto speed_ratio = static_cast<double>(speed) / DEFAULT_SPEED;
    for (auto& timestamp : speedup.m_beat_timestamps) {
        timestamp.time *= speed_ratio;
    }
    speedup.m_last_bpm = (speedup.m_last_bpm * speed) / DEFAULT_SPEED;

    return speedup;
}

Beat TempoMap::to_beats(Second seconds) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), seconds,
        [](const auto& x, const auto& y) { return x.time < y; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return back.beat + (seconds - back.time).to_beat(m_last_bpm);
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return pos->beat - (pos->time - seconds).to_beat(DEFAULT_BPM);
    }
    const auto prev = pos - 1;
    return prev->beat
        + (pos->beat - prev->beat)
        * ((seconds - prev->time) / (pos->time - prev->time));
}
