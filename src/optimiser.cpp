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
#include <iterator>

#include "optimiser.hpp"

static bool phrase_contains_pos(const StarPower& phrase, uint32_t position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

template <class InputIt, class OutputIt>
static void append_note_points(InputIt first, InputIt last, OutputIt points,
                               const SongHeader& header, bool is_note_sp_ender)
{
    constexpr auto NOTE_VALUE = 50U;
    const double resolution = header.resolution();
    const auto tick_gap = std::max(header.resolution() / 25, 1);

    const auto chord_size = static_cast<uint32_t>(std::distance(first, last));
    auto chord_length = static_cast<int32_t>(
        std::max_element(first, last, [](const auto& x, const auto& y) {
            return x.length < y.length;
        })->length);
    auto pos = first->position;
    *points++
        = {pos / resolution, NOTE_VALUE * chord_size, false, is_note_sp_ender};
    while (chord_length > 0) {
        pos += static_cast<uint32_t>(tick_gap);
        chord_length -= tick_gap;
        *points++ = {pos / resolution, 1, true, false};
    }
}

static std::vector<Point> notes_to_points(const NoteTrack& track,
                                          const SongHeader& header)
{
    std::vector<Point> points;

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
        append_note_points(p, q, std::back_inserter(points), header,
                           is_note_sp_ender);
        p = q;
    }

    std::stable_sort(points.begin(), points.end(),
                     [](const auto& x, const auto& y) {
                         return x.beat_position < y.beat_position;
                     });

    return points;
}

std::vector<ProcessedTrack::BeatRate>
ProcessedTrack::form_beat_rates(const SongHeader& header,
                                const SyncTrack& sync_track)
{
    constexpr double DEFAULT_BEAT_RATE = 4.0;
    constexpr double MEASURES_PER_BAR = 8.0;
    constexpr double SP_GAIN_RATE = 1 / 30.0;

    std::vector<BeatRate> beat_rates;
    beat_rates.reserve(sync_track.time_sigs().size());

    for (const auto& ts : sync_track.time_sigs()) {
        const auto pos = static_cast<double>(ts.position) / header.resolution();
        const auto measure_rate
            = ts.numerator * DEFAULT_BEAT_RATE / ts.denominator;
        const auto drain_rate
            = SP_GAIN_RATE - 1 / (MEASURES_PER_BAR * measure_rate);
        beat_rates.push_back({pos, drain_rate});
    }

    return beat_rates;
}

ProcessedTrack::ProcessedTrack(const NoteTrack& track, const SongHeader& header,
                               const SyncTrack& sync_track)
    : m_points {notes_to_points(track, header)}
    , m_converter {TimeConverter(sync_track, header)}
    , m_beat_rates {form_beat_rates(header, sync_track)}
{
    std::vector<std::tuple<uint32_t, uint32_t>> ranges_as_ticks;
    for (const auto& note : track.notes()) {
        if (note.length > 0) {
            ranges_as_ticks.emplace_back(note.position,
                                         note.position + note.length);
        }
    }
    std::sort(ranges_as_ticks.begin(), ranges_as_ticks.end());

    if (!ranges_as_ticks.empty()) {
        std::vector<std::tuple<uint32_t, uint32_t>> merged_ranges;
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
            auto start
                = static_cast<double>(std::get<0>(range)) / header.resolution();
            auto end
                = static_cast<double>(std::get<1>(range)) / header.resolution();
            m_whammy_ranges.push_back({start, end});
        }
    }
}

double ProcessedTrack::propagate_sp_over_whammy(double start, double end,
                                                double sp_bar_amount) const
{
    constexpr double MEASURES_PER_BAR = 8.0;

    auto p = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                          [=](const auto& x) { return x.end_beat > start; });
    while ((p != m_whammy_ranges.cend()) && (p->start_beat < end)) {
        if (p->start_beat > start) {
            auto meas_diff = m_converter.beats_to_measures(p->start_beat)
                - m_converter.beats_to_measures(start);
            sp_bar_amount -= meas_diff / MEASURES_PER_BAR;
            if (sp_bar_amount < 0.0) {
                return -1.0;
            }
            start = p->start_beat;
        }
        auto range_end = std::min(end, p->end_beat);
        sp_bar_amount
            = propagate_over_whammy_range(start, range_end, sp_bar_amount);
        if (sp_bar_amount < 0.0) {
            return -1.0;
        }
        start = p->end_beat;
        if (start >= end) {
            return sp_bar_amount;
        }
        ++p;
    }

    auto meas_diff = m_converter.beats_to_measures(end)
        - m_converter.beats_to_measures(start);
    sp_bar_amount -= meas_diff / MEASURES_PER_BAR;
    if (sp_bar_amount < 0.0) {
        return -1.0;
    }
    return sp_bar_amount;
}

bool ProcessedTrack::is_candidate_valid(
    const ActivationCandidate& activation) const
{
    constexpr double MEASURES_PER_BAR = 8.0;
    constexpr double MINIMUM_SP_AMOUNT = 0.5;
    constexpr double SP_PHRASE_AMOUNT = 0.25;

    if (activation.sp_bar_amount < MINIMUM_SP_AMOUNT) {
        return false;
    }

    auto current_position = activation.act_start->beat_position;
    auto min_sp = activation.sp_bar_amount;
    auto max_sp = activation.sp_bar_amount;
    auto starting_meas_diff = m_converter.beats_to_measures(current_position)
        - m_converter.beats_to_measures(activation.earliest_activation_point);
    min_sp -= starting_meas_diff / MEASURES_PER_BAR;
    min_sp = std::max(min_sp, 0.0);

    for (auto p = activation.act_start; p < activation.act_end; ++p) {
        if (p->is_sp_granting_note) {
            max_sp = propagate_sp_over_whammy(current_position,
                                              p->beat_position, max_sp);
            if (max_sp < 0.0) {
                return false;
            }
            auto meas_diff = m_converter.beats_to_measures(p->beat_position)
                - m_converter.beats_to_measures(current_position);
            min_sp = std::max(min_sp - meas_diff / MEASURES_PER_BAR, 0.0);
            min_sp = std::min(min_sp + SP_PHRASE_AMOUNT, 1.0);
            max_sp = std::min(max_sp + SP_PHRASE_AMOUNT, 1.0);
            current_position = p->beat_position;
        }
    }

    max_sp = propagate_sp_over_whammy(
        current_position, activation.act_end->beat_position, max_sp);
    if (max_sp < 0.0) {
        return false;
    }

    const auto next_point = std::next(activation.act_end);
    if (next_point == m_points.cend()) {
        return true;
    }

    auto meas_diff = m_converter.beats_to_measures(next_point->beat_position)
        - m_converter.beats_to_measures(current_position);
    min_sp -= meas_diff / MEASURES_PER_BAR;

    return min_sp < 0.0;
}

double ProcessedTrack::propagate_over_whammy_range(double start, double end,
                                                   double sp_bar_amount) const
{
    constexpr double DEFAULT_NET_SP_GAIN_RATE = 1 / 480.0;

    auto p = std::find_if(m_beat_rates.cbegin(), m_beat_rates.cend(),
                          [=](const auto& ts) { return ts.position >= start; });
    if (p != m_beat_rates.cbegin()) {
        --p;
    } else {
        double subrange_end = std::min(end, p->position);
        sp_bar_amount += (subrange_end - start) * DEFAULT_NET_SP_GAIN_RATE;
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        if (sp_bar_amount < 0.0) {
            return -1.0;
        }
        start = subrange_end;
    }
    while (start < end) {
        double subrange_end = end;
        if (std::next(p) != m_beat_rates.cend()) {
            subrange_end = std::min(end, std::next(p)->position);
        }
        sp_bar_amount += (subrange_end - start) * p->net_sp_gain_rate;
        if (sp_bar_amount < 0.0) {
            return -1.0;
        }
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        start = subrange_end;
        ++p;
    }

    return sp_bar_amount;
}

double front_end(const Point& point, const TimeConverter& converter)
{
    constexpr double FRONT_END = 0.07;

    if (point.is_hold_point) {
        return point.beat_position;
    }

    auto time = converter.beats_to_seconds(point.beat_position);
    time -= FRONT_END;
    return converter.seconds_to_beats(time);
}

double back_end(const Point& point, const TimeConverter& converter)
{
    constexpr double BACK_END = 0.07;

    if (point.is_hold_point) {
        return point.beat_position;
    }

    auto time = converter.beats_to_seconds(point.beat_position);
    time += BACK_END;
    return converter.seconds_to_beats(time);
}
