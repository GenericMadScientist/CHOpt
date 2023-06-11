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
                   std::vector<Tick> od_beats, int resolution)
    : m_od_beats {std::move(od_beats)}
    , m_resolution {resolution}
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
        last_time += to_beats(bpm.position - last_tick).value()
            * (MS_PER_MINUTE / static_cast<double>(last_bpm));
        const auto beat = to_beats(bpm.position);
        m_beat_timestamps.push_back({beat, Second(last_time)});
        last_bpm = bpm.bpm;
        last_tick = bpm.position;
    }

    m_last_bpm = last_bpm;

    last_tick = Tick {0};
    auto last_beat_rate = DEFAULT_BEAT_RATE;
    auto last_measure = 0.0;

    for (const auto& ts : m_time_sigs) {
        last_measure += to_beats(ts.position - last_tick).value()
            / static_cast<double>(last_beat_rate);
        const auto beat = to_beats(ts.position);
        m_measure_timestamps.push_back({Measure(last_measure), beat});
        last_beat_rate = (ts.numerator * DEFAULT_BEAT_RATE) / ts.denominator;
        last_tick = ts.position;
    }

    m_last_beat_rate = last_beat_rate;

    if (!m_od_beats.empty()) {
        for (auto i = 0U; i < m_od_beats.size(); ++i) {
            const auto beat = to_beats(m_od_beats[i]);
            m_od_beat_timestamps.push_back(
                {OdBeat(i / DEFAULT_BEAT_RATE), beat});
        }
    } else {
        m_od_beat_timestamps.push_back({OdBeat(0.0), Beat(0.0)});
    }

    m_last_od_beat_rate = DEFAULT_BEAT_RATE;
}

TempoMap TempoMap::speedup(int speed) const
{
    constexpr auto DEFAULT_SPEED = 100;

    TempoMap speedup {m_time_sigs, m_bpms, m_od_beats, m_resolution};
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

Beat TempoMap::to_beats(Measure measures) const
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

Beat TempoMap::to_beats(OdBeat od_beats) const
{
    const auto pos = std::lower_bound(
        m_od_beat_timestamps.cbegin(), m_od_beat_timestamps.cend(), od_beats,
        [](const auto& x, const auto& y) { return x.od_beat < y; });
    if (pos == m_od_beat_timestamps.cend()) {
        const auto& back = m_od_beat_timestamps.back();
        return back.beat
            + (od_beats - back.od_beat).to_beat(m_last_od_beat_rate);
    }
    if (pos == m_od_beat_timestamps.cbegin()) {
        return pos->beat - (pos->od_beat - od_beats).to_beat(DEFAULT_BEAT_RATE);
    }
    const auto prev = pos - 1;
    return prev->beat
        + (pos->beat - prev->beat)
        * ((od_beats - prev->od_beat) / (pos->od_beat - prev->od_beat));
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

Beat TempoMap::to_beats(SpMeasure measures) const
{
    if (m_use_od_beats) {
        return to_beats(OdBeat {measures.value()});
    }
    return to_beats(Measure {measures.value()});
}

Measure TempoMap::to_measures(Beat beats) const
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

OdBeat TempoMap::to_od_beats(Beat beats) const
{
    const auto pos = std::lower_bound(
        m_od_beat_timestamps.cbegin(), m_od_beat_timestamps.cend(), beats,
        [](const auto& x, const auto& y) { return x.beat < y; });
    if (pos == m_od_beat_timestamps.cend()) {
        const auto& back = m_od_beat_timestamps.back();
        return back.od_beat
            + OdBeat(
                   (beats - back.beat).to_measure(m_last_od_beat_rate).value());
    }
    if (pos == m_od_beat_timestamps.cbegin()) {
        return pos->od_beat
            - OdBeat((pos->beat - beats).to_measure(DEFAULT_BEAT_RATE).value());
    }
    const auto prev = pos - 1;
    return prev->od_beat
        + (pos->od_beat - prev->od_beat)
        * ((beats - prev->beat) / (pos->beat - prev->beat));
}

Second TempoMap::to_seconds(Beat beats) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), beats,
        [](const auto& x, const auto& y) { return x.beat < y; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return back.time + (beats - back.beat).to_second(m_last_bpm);
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return pos->time - (pos->beat - beats).to_second(DEFAULT_BPM);
    }
    const auto prev = pos - 1;
    return prev->time
        + (pos->time - prev->time)
        * ((beats - prev->beat) / (pos->beat - prev->beat));
}

Second TempoMap::to_seconds(SpMeasure measures) const
{
    return to_seconds(to_beats(measures));
}

Second TempoMap::to_seconds(Tick ticks) const
{
    return to_seconds(to_beats(ticks));
}

SpMeasure TempoMap::to_sp_measures(Beat beats) const
{
    if (m_use_od_beats) {
        return SpMeasure {to_od_beats(beats).value()};
    }
    return SpMeasure {to_measures(beats).value()};
}

SpMeasure TempoMap::to_sp_measures(Second seconds) const
{
    return to_sp_measures(to_beats(seconds));
}

Tick TempoMap::to_ticks(Second seconds) const
{
    return to_ticks(to_beats(seconds));
}