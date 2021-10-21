/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021 Raymond Wright
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

static bool phrase_contains_pos(const StarPower& phrase, int position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

std::vector<SpData::BeatRate>
SpData::form_beat_rates(int resolution, const SyncTrack& sync_track,
                        const std::vector<int>& od_beats, const Engine& engine)
{
    constexpr double DEFAULT_BEAT_RATE = 4.0;

    std::vector<BeatRate> beat_rates;
    if (!engine.uses_beat_track()) {
        beat_rates.reserve(sync_track.time_sigs().size());

        for (const auto& ts : sync_track.time_sigs()) {
            const auto pos = static_cast<double>(ts.position) / resolution;
            const auto measure_rate
                = ts.numerator * DEFAULT_BEAT_RATE / ts.denominator;
            const auto drain_rate
                = engine.sp_gain_rate() - 1 / (MEASURES_PER_BAR * measure_rate);
            beat_rates.push_back({Beat(pos), drain_rate});
        }
    } else if (!od_beats.empty()) {
        beat_rates.reserve(od_beats.size() - 1);

        for (auto i = 0U; i < od_beats.size() - 1; ++i) {
            const auto pos = static_cast<double>(od_beats[i]) / resolution;
            const auto next_marker
                = static_cast<double>(od_beats[i + 1]) / resolution;
            const auto drain_rate = engine.sp_gain_rate()
                - 1 / (DEFAULT_BEATS_PER_BAR * (next_marker - pos));
            beat_rates.push_back({Beat(pos), drain_rate});
        }
    } else {
        beat_rates.push_back(
            {Beat(0.0), engine.sp_gain_rate() - 1 / DEFAULT_BEATS_PER_BAR});
    }

    return beat_rates;
}

void SpData::initialise(
    const std::vector<std::tuple<int, int, Second>>& note_spans,
    const std::vector<StarPower>& phrases, int resolution, Second lazy_whammy,
    Second video_lag)
{
    // Elements are (whammy start, whammy end, note).
    std::vector<std::tuple<Beat, Beat, Beat>> ranges;
    for (const auto& [position, length, early_timing_window] : note_spans) {
        if (length == 0) {
            continue;
        }
        const int pos_copy = position;
        const auto phrase = std::find_if(
            phrases.cbegin(), phrases.cend(),
            [&](const auto& p) { return phrase_contains_pos(p, pos_copy); });
        if (phrase == phrases.cend()) {
            continue;
        }

        const Beat note {static_cast<double>(position) / resolution};
        auto second_start = m_converter.beats_to_seconds(note);
        second_start -= early_timing_window;
        second_start += lazy_whammy;
        second_start += video_lag;
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
    auto p = std::lower_bound(
        m_beat_rates.cbegin(), m_beat_rates.cend(), start,
        [](const auto& ts, const auto& y) { return ts.position < y; });
    if (p != m_beat_rates.cbegin()) {
        --p;
    } else {
        auto subrange_end = std::min(end, p->position);
        sp_bar_amount
            += (subrange_end - start).value() * m_default_net_sp_gain_rate;
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        start = subrange_end;
    }
    while (start < end) {
        auto subrange_end = end;
        if (std::next(p) != m_beat_rates.cend()) {
            subrange_end = std::min(end, std::next(p)->position);
        }
        sp_bar_amount += (subrange_end - start).value() * p->net_sp_gain_rate;
        if (sp_bar_amount < 0.0) {
            return -1.0;
        }
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        start = subrange_end;
        ++p;
    }

    return sp_bar_amount;
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

Position SpData::activation_end_point(Position start, Position end,
                                      double sp_bar_amount) const
{
    auto p = first_whammy_range_after(start.beat);
    while ((p != m_whammy_ranges.cend()) && (p->start.beat < end.beat)) {
        if (p->start.beat > start.beat) {
            auto meas_diff = p->start.measure - start.measure;
            auto sp_deduction = meas_diff.value() / MEASURES_PER_BAR;
            if (sp_bar_amount < sp_deduction) {
                auto end_meas
                    = start.measure + Measure(sp_bar_amount * MEASURES_PER_BAR);
                auto end_beat = m_converter.measures_to_beats(end_meas);
                return {end_beat, end_meas};
            }
            sp_bar_amount -= sp_deduction;
            start = p->start;
        }
        auto range_end = std::min(end.beat, p->end.beat);
        auto new_sp_bar_amount
            = propagate_over_whammy_range(start.beat, range_end, sp_bar_amount);
        if (new_sp_bar_amount < 0.0) {
            auto end_beat = whammy_propagation_endpoint(start.beat, end.beat,
                                                        sp_bar_amount);
            auto end_meas = m_converter.beats_to_measures(end_beat);
            return {end_beat, end_meas};
        }
        sp_bar_amount = new_sp_bar_amount;
        if (p->end.beat >= end.beat) {
            return end;
        }
        start = p->end;
        ++p;
    }

    auto meas_diff = end.measure - start.measure;
    auto sp_deduction = meas_diff.value() / MEASURES_PER_BAR;
    if (sp_bar_amount < sp_deduction) {
        auto end_meas
            = start.measure + Measure(sp_bar_amount * MEASURES_PER_BAR);
        auto end_beat = m_converter.measures_to_beats(end_meas);
        return {end_beat, end_meas};
    }
    return end;
}

// Return the point whammy runs out if all of the range [start, end) is
// whammied.
Beat SpData::whammy_propagation_endpoint(Beat start, Beat end,
                                         double sp_bar_amount) const
{
    auto p = std::lower_bound(
        m_beat_rates.cbegin(), m_beat_rates.cend(), start,
        [](const auto& ts, const auto& y) { return ts.position < y; });
    if (p != m_beat_rates.cbegin()) {
        --p;
    } else {
        auto subrange_end = std::min(end, p->position);
        auto sp_gain
            = (subrange_end - start).value() * m_default_net_sp_gain_rate;
        sp_bar_amount += sp_gain;
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        start = subrange_end;
    }
    while (start < end) {
        auto subrange_end = end;
        if (std::next(p) != m_beat_rates.cend()) {
            subrange_end = std::min(end, std::next(p)->position);
        }
        auto sp_gain = (subrange_end - start).value() * p->net_sp_gain_rate;
        if (sp_bar_amount + sp_gain < 0.0) {
            return start + Beat(-sp_bar_amount / p->net_sp_gain_rate);
        }
        sp_bar_amount += sp_gain;
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        start = subrange_end;
        ++p;
    }

    return end;
}
