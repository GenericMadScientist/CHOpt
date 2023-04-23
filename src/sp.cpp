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
#include <iterator>

#include "sp.hpp"

namespace {
bool phrase_contains_pos(const StarPower& phrase, int position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

double sp_deduction(Position start, Position end)
{
    constexpr double MEASURES_PER_BAR = 8.0;

    const auto meas_diff = end.measure - start.measure;
    return meas_diff.value() / MEASURES_PER_BAR;
}
}

std::vector<SpData::BeatRate>
SpData::form_beat_rates(const TempoMap& tempo_map,
                        const std::vector<Tick>& od_beats, const Engine& engine)
{
    constexpr double DEFAULT_BEAT_RATE = 4.0;

    std::vector<BeatRate> beat_rates;
    if (!engine.uses_beat_track()) {
        beat_rates.reserve(tempo_map.time_sigs().size());

        for (const auto& ts : tempo_map.time_sigs()) {
            const auto pos = tempo_map.to_beat(ts.position);
            const auto measure_rate
                = ts.numerator * DEFAULT_BEAT_RATE / ts.denominator;
            const auto drain_rate
                = engine.sp_gain_rate() - 1 / (MEASURES_PER_BAR * measure_rate);
            beat_rates.push_back({pos, drain_rate});
        }
    } else if (!od_beats.empty()) {
        beat_rates.reserve(od_beats.size() - 1);

        for (auto i = 0U; i < od_beats.size() - 1; ++i) {
            const auto pos = tempo_map.to_beat(od_beats[i]);
            const auto next_marker = tempo_map.to_beat(od_beats[i + 1]);
            const auto drain_rate = engine.sp_gain_rate()
                - 1 / (DEFAULT_BEATS_PER_BAR * (next_marker - pos).value());
            beat_rates.push_back({pos, drain_rate});
        }
    } else {
        beat_rates.push_back(
            {Beat(0.0), engine.sp_gain_rate() - 1 / DEFAULT_BEATS_PER_BAR});
    }

    return beat_rates;
}

std::vector<std::tuple<int, int, Second>>
SpData::note_spans(const NoteTrack& track, double early_whammy,
                   const Engine& engine)
{
    const auto resolution
        = static_cast<double>(track.global_data().resolution());
    std::vector<std::tuple<int, int, Second>> spans;
    for (auto note = track.notes().cbegin(); note < track.notes().cend();
         ++note) {
        auto early_gap = std::numeric_limits<double>::infinity();
        auto late_gap = std::numeric_limits<double>::infinity();
        const auto current_note_time
            = m_converter.beats_to_seconds(Beat {note->position / resolution})
                  .value();
        if (note != track.notes().cbegin()) {
            early_gap = current_note_time
                - m_converter
                      .beats_to_seconds(
                          Beat {std::prev(note)->position / resolution})
                      .value();
        }
        if (std::next(note) < track.notes().cend()) {
            late_gap = m_converter
                           .beats_to_seconds(
                               Beat {std::next(note)->position / resolution})
                           .value()
                - current_note_time;
        }
        for (auto length : note->lengths) {
            if (length != -1) {
                spans.emplace_back(
                    note->position, length,
                    Second {engine.early_timing_window(early_gap, late_gap)}
                        * early_whammy);
            }
        }
    }
    return spans;
}

SpData::SpData(const NoteTrack& track, const TempoMap& tempo_map,
               const std::vector<Tick>& od_beats,
               const SqueezeSettings& squeeze_settings, const Engine& engine)
    : m_converter {tempo_map, engine, od_beats}
    , m_beat_rates {form_beat_rates(tempo_map, od_beats, engine)}
    , m_sp_gain_rate {engine.sp_gain_rate()}
    , m_default_net_sp_gain_rate {m_sp_gain_rate - 1 / DEFAULT_BEATS_PER_BAR}
{
    const double resolution = track.global_data().resolution();

    // Elements are (whammy start, whammy end, note).
    std::vector<std::tuple<Beat, Beat, Beat>> ranges;
    for (const auto& [position, length, early_timing_window] :
         note_spans(track, squeeze_settings.early_whammy, engine)) {
        if (length == 0) {
            continue;
        }
        const int pos_copy = position;
        const auto phrase = std::find_if(
            track.sp_phrases().cbegin(), track.sp_phrases().cend(),
            [&](const auto& p) { return phrase_contains_pos(p, pos_copy); });
        if (phrase == track.sp_phrases().cend()) {
            continue;
        }

        const Beat note {static_cast<double>(position) / resolution};
        auto second_start = m_converter.beats_to_seconds(note);
        second_start -= early_timing_window;
        second_start += squeeze_settings.lazy_whammy;
        second_start += squeeze_settings.video_lag;
        const auto beat_start = m_converter.seconds_to_beats(second_start);
        Beat beat_end {static_cast<double>(position + length) / resolution};
        if (beat_start < beat_end) {
            ranges.emplace_back(beat_start, beat_end, note);
        }
    }

    if (ranges.empty()) {
        return;
    }

    std::sort(ranges.begin(), ranges.end());

    std::vector<std::tuple<Beat, Beat, Beat>> merged_ranges;
    auto pair = ranges[0];
    for (auto p = std::next(ranges.cbegin()); p < ranges.cend(); ++p) {
        if (std::get<0>(*p) <= std::get<1>(pair)) {
            std::get<1>(pair) = std::max(std::get<1>(pair), std::get<1>(*p));
        } else {
            merged_ranges.push_back(pair);
            pair = *p;
        }
    }
    merged_ranges.push_back(pair);

    for (auto [start, end, note] : merged_ranges) {
        const auto start_meas = m_converter.beats_to_measures(start);
        const auto end_meas = m_converter.beats_to_measures(end);
        m_whammy_ranges.push_back({{start, start_meas}, {end, end_meas}, note});
    }

    if (m_whammy_ranges.empty()) {
        return;
    }

    m_last_whammy_point = m_whammy_ranges.back().end.beat;
    auto p = m_whammy_ranges.cbegin();
    for (auto pos = 0; pos < m_last_whammy_point.value(); ++pos) {
        p = std::find_if_not(p, m_whammy_ranges.cend(), [=](const auto& x) {
            return x.end.beat.value() <= pos;
        });
        m_initial_guesses.push_back(p);
    }
}

std::vector<SpData::WhammyRange>::const_iterator
SpData::first_whammy_range_after(Beat pos) const
{
    if (m_last_whammy_point <= pos) {
        return m_whammy_ranges.cend();
    }
    const auto index = static_cast<std::size_t>(pos.value());
    const auto begin = (pos < Beat(0.0)) ? m_whammy_ranges.cbegin()
                                         : m_initial_guesses[index];

    return std::find_if_not(begin, m_whammy_ranges.cend(),
                            [=](const auto& x) { return x.end.beat <= pos; });
}

SpData::WhammyPropagationState
SpData::initial_whammy_prop_state(Beat start, Beat end,
                                  double sp_bar_amount) const
{
    auto p = std::lower_bound(
        m_beat_rates.cbegin(), m_beat_rates.cend(), start,
        [](const auto& ts, const auto& y) { return ts.position < y; });
    if (p != m_beat_rates.cbegin()) {
        --p;
    } else {
        const auto subrange_end = std::min(end, p->position);
        const auto sp_gain
            = (subrange_end - start).value() * m_default_net_sp_gain_rate;
        sp_bar_amount += sp_gain;
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        start = subrange_end;
    }
    return {p, start, sp_bar_amount};
}

double SpData::propagate_sp_over_whammy_max(Position start, Position end,
                                            double sp) const
{
    assert(start.beat <= end.beat); // NOLINT
    auto p = first_whammy_range_after(start.beat);
    while ((p != m_whammy_ranges.cend()) && (p->start.beat < end.beat)) {
        if (p->start.beat > start.beat) {
            const auto meas_diff = p->start.measure - start.measure;
            sp -= meas_diff.value() / MEASURES_PER_BAR;
            if (sp < 0.0) {
                return sp;
            }
            start = p->start;
        }
        const auto range_end = std::min(end.beat, p->end.beat);
        sp = propagate_over_whammy_range(start.beat, range_end, sp);
        if (sp < 0.0 || p->end.beat >= end.beat) {
            return sp;
        }
        start = p->end;
        ++p;
    }

    const auto meas_diff = end.measure - start.measure;
    sp -= meas_diff.value() / MEASURES_PER_BAR;
    return sp;
}

double SpData::propagate_sp_over_whammy_min(Position start, Position end,
                                            double sp,
                                            Position required_whammy_end) const
{
    assert(start.beat <= end.beat); // NOLINT
    if (required_whammy_end.beat > start.beat) {
        auto whammy_end = end;
        if (required_whammy_end.beat < end.beat) {
            whammy_end = required_whammy_end;
        }
        sp = propagate_sp_over_whammy_max(start, whammy_end, sp);
        start = required_whammy_end;
    }
    if (start.beat < end.beat) {
        const auto meas_diff = end.measure - start.measure;
        sp -= meas_diff.value() / MEASURES_PER_BAR;
    }

    sp = std::max(sp, 0.0);
    return sp;
}

double SpData::propagate_over_whammy_range(Beat start, Beat end,
                                           double sp_bar_amount) const
{
    auto state = initial_whammy_prop_state(start, end, sp_bar_amount);
    while (state.current_position < end) {
        auto subrange_end = end;
        if (std::next(state.current_beat_rate) != m_beat_rates.cend()) {
            subrange_end
                = std::min(end, std::next(state.current_beat_rate)->position);
        }
        state.current_sp += (subrange_end - state.current_position).value()
            * state.current_beat_rate->net_sp_gain_rate;
        if (state.current_sp < 0.0) {
            return -1.0;
        }
        state.current_sp = std::min(state.current_sp, 1.0);
        state.current_position = subrange_end;
        ++state.current_beat_rate;
    }

    return state.current_sp;
}

bool SpData::is_in_whammy_ranges(Beat beat) const
{
    const auto p = first_whammy_range_after(beat);
    if (p == m_whammy_ranges.cend()) {
        return false;
    }
    return p->start.beat <= beat;
}

double SpData::available_whammy(Beat start, Beat end) const
{
    double total_whammy {0.0};

    for (auto p = first_whammy_range_after(start); p < m_whammy_ranges.cend();
         ++p) {
        if (p->start.beat >= end) {
            break;
        }
        const auto whammy_start = std::max(p->start.beat, start);
        const auto whammy_end = std::min(p->end.beat, end);
        total_whammy += (whammy_end - whammy_start).value() * m_sp_gain_rate;
    }

    return total_whammy;
}

double SpData::available_whammy(Beat start, Beat end, Beat note_pos) const
{
    double total_whammy {0.0};

    for (auto p = first_whammy_range_after(start); p < m_whammy_ranges.cend();
         ++p) {
        if (p->start.beat >= end || p->note >= note_pos) {
            break;
        }
        auto whammy_start = std::max(p->start.beat, start);
        auto whammy_end = std::min(p->end.beat, end);
        total_whammy += (whammy_end - whammy_start).value() * m_sp_gain_rate;
    }

    return total_whammy;
}

Position SpData::sp_drain_end_point(Position start, double sp_bar_amount) const
{
    const auto end_meas
        = start.measure + Measure(sp_bar_amount * MEASURES_PER_BAR);
    const auto end_beat = m_converter.measures_to_beats(end_meas);
    return {end_beat, end_meas};
}

Position SpData::activation_end_point(Position start, Position end,
                                      double sp_bar_amount) const
{
    auto p = first_whammy_range_after(start.beat);
    while ((p != m_whammy_ranges.cend()) && (p->start.beat < end.beat)) {
        if (p->start.beat > start.beat) {
            const auto sp_drop = sp_deduction(start, p->start);
            if (sp_bar_amount < sp_drop) {
                return sp_drain_end_point(start, sp_bar_amount);
            }
            sp_bar_amount -= sp_drop;
            start = p->start;
        }
        const auto range_end = std::min(end.beat, p->end.beat);
        const auto new_sp_bar_amount
            = propagate_over_whammy_range(start.beat, range_end, sp_bar_amount);
        if (new_sp_bar_amount < 0.0) {
            const auto end_beat = whammy_propagation_endpoint(
                start.beat, end.beat, sp_bar_amount);
            const auto end_meas = m_converter.beats_to_measures(end_beat);
            return {end_beat, end_meas};
        }
        sp_bar_amount = new_sp_bar_amount;
        if (p->end.beat >= end.beat) {
            return end;
        }
        start = p->end;
        ++p;
    }

    const auto sp_drop = sp_deduction(start, end);
    if (sp_bar_amount < sp_drop) {
        return sp_drain_end_point(start, sp_bar_amount);
    }
    return end;
}

// Return the point whammy runs out if all of the range [start, end) is
// whammied.
Beat SpData::whammy_propagation_endpoint(Beat start, Beat end,
                                         double sp_bar_amount) const
{
    auto state = initial_whammy_prop_state(start, end, sp_bar_amount);
    while (state.current_position < end) {
        auto subrange_end = end;
        if (std::next(state.current_beat_rate) != m_beat_rates.cend()) {
            subrange_end
                = std::min(end, std::next(state.current_beat_rate)->position);
        }
        auto sp_gain = (subrange_end - state.current_position).value()
            * state.current_beat_rate->net_sp_gain_rate;
        if (state.current_sp + sp_gain < 0.0) {
            return state.current_position
                + Beat(-state.current_sp
                       / state.current_beat_rate->net_sp_gain_rate);
        }
        state.current_sp += sp_gain;
        state.current_sp = std::min(state.current_sp, 1.0);
        state.current_position = subrange_end;
        ++state.current_beat_rate;
    }

    return end;
}
