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

#include <iterator>

#include "sp.hpp"

static bool phrase_contains_pos(const StarPower& phrase, std::uint32_t position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

std::vector<SpData::BeatRate>
SpData::form_beat_rates(std::int32_t resolution, const SyncTrack& sync_track)
{
    constexpr double DEFAULT_BEAT_RATE = 4.0;
    constexpr double MEASURES_PER_BAR = 8.0;

    std::vector<BeatRate> beat_rates;
    beat_rates.reserve(sync_track.time_sigs().size());

    for (const auto& ts : sync_track.time_sigs()) {
        const auto pos = static_cast<double>(ts.position) / resolution;
        const auto measure_rate
            = ts.numerator * DEFAULT_BEAT_RATE / ts.denominator;
        const auto drain_rate
            = SP_GAIN_RATE - 1 / (MEASURES_PER_BAR * measure_rate);
        beat_rates.push_back({Beat(pos), drain_rate});
    }

    return beat_rates;
}

SpData::SpData(const NoteTrack& track, std::int32_t resolution,
               const SyncTrack& sync_track)
    : m_converter {TimeConverter(sync_track, resolution)}
    , m_beat_rates {form_beat_rates(resolution, sync_track)}
{
    std::vector<std::tuple<std::uint32_t, std::uint32_t>> ranges_as_ticks;
    for (const auto& note : track.notes()) {
        if (note.length == 0) {
            continue;
        }
        auto phrase
            = std::find_if(track.sp_phrases().cbegin(),
                           track.sp_phrases().cend(), [&](const auto& p) {
                               return phrase_contains_pos(p, note.position);
                           });
        if (phrase == track.sp_phrases().cend()) {
            continue;
        }
        ranges_as_ticks.emplace_back(note.position,
                                     note.position + note.length);
    }
    std::sort(ranges_as_ticks.begin(), ranges_as_ticks.end());

    if (!ranges_as_ticks.empty()) {
        std::vector<std::tuple<std::uint32_t, std::uint32_t>> merged_ranges;
        auto pair = ranges_as_ticks[0];
        for (auto p = std::next(ranges_as_ticks.cbegin());
             p < ranges_as_ticks.cend(); ++p) {
            if (std::get<0>(*p) <= std::get<1>(pair)) {
                std::get<1>(pair)
                    = std::max(std::get<1>(pair), std::get<1>(*p));
            } else {
                merged_ranges.push_back(pair);
                pair = *p;
            }
        }
        merged_ranges.push_back(pair);

        for (const auto& range : merged_ranges) {
            auto start = static_cast<double>(std::get<0>(range)) / resolution;
            auto end = static_cast<double>(std::get<1>(range)) / resolution;
            auto start_meas = m_converter.beats_to_measures(Beat(start));
            auto end_meas = m_converter.beats_to_measures(Beat(end));
            m_whammy_ranges.push_back(
                {{Beat(start), start_meas}, {Beat(end), end_meas}});
        }
    }
}

SpBar SpData::propagate_sp_over_whammy(Position start, Position end,
                                       SpBar sp_bar) const
{
    constexpr double MEASURES_PER_BAR = 8.0;

    sp_bar.min() -= (end.measure - start.measure).value() / MEASURES_PER_BAR;
    sp_bar.min() = std::max(sp_bar.min(), 0.0);

    auto p
        = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                       [=](const auto& x) { return x.end.beat > start.beat; });
    while ((p != m_whammy_ranges.cend()) && (p->start.beat < end.beat)) {
        if (p->start.beat > start.beat) {
            auto meas_diff = p->start.measure - start.measure;
            sp_bar.max() -= meas_diff.value() / MEASURES_PER_BAR;
            if (sp_bar.max() < 0.0) {
                return sp_bar;
            }
            start = p->start;
        }
        auto range_end = std::min(end.beat, p->end.beat);
        sp_bar.max()
            = propagate_over_whammy_range(start.beat, range_end, sp_bar.max());
        if (sp_bar.max() < 0.0) {
            return sp_bar;
        }
        if (p->end.beat >= end.beat) {
            return sp_bar;
        }
        start = p->end;
        ++p;
    }

    auto meas_diff = end.measure - start.measure;
    sp_bar.max() -= meas_diff.value() / MEASURES_PER_BAR;
    return sp_bar;
}

double SpData::propagate_over_whammy_range(Beat start, Beat end,
                                           double sp_bar_amount) const
{
    constexpr double DEFAULT_NET_SP_GAIN_RATE = 1 / 480.0;

    auto p = std::find_if(m_beat_rates.cbegin(), m_beat_rates.cend(),
                          [=](const auto& ts) { return ts.position >= start; });
    if (p != m_beat_rates.cbegin()) {
        --p;
    } else {
        auto subrange_end = std::min(end, p->position);
        sp_bar_amount
            += (subrange_end - start).value() * DEFAULT_NET_SP_GAIN_RATE;
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        if (sp_bar_amount < 0.0) {
            return -1.0;
        }
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
    auto p = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                          [=](const auto& x) { return x.end.beat >= beat; });
    if (p == m_whammy_ranges.cend()) {
        return false;
    }
    return p->start.beat <= beat;
}

double SpData::available_whammy(Beat start, Beat end) const
{
    double total_whammy {0.0};

    auto p = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                          [=](const auto& x) { return x.end.beat > start; });
    for (; p < m_whammy_ranges.cend(); ++p) {
        if (p->start.beat >= end) {
            break;
        }
        auto whammy_start = std::max(p->start.beat, start);
        auto whammy_end = std::min(p->end.beat, end);
        total_whammy += (whammy_end - whammy_start).value() * SP_GAIN_RATE;
    }

    return total_whammy;
}
