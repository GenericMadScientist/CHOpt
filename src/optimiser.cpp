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
#include <iterator>
#include <numeric>

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
    *points++ = {Beat(pos / resolution), NOTE_VALUE * chord_size, false,
                 is_note_sp_ender};
    while (chord_length > 0) {
        pos += static_cast<uint32_t>(tick_gap);
        chord_length -= tick_gap;
        *points++ = {Beat(pos / resolution), 1, true, false};
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

    auto combo = 0U;
    for (auto& point : points) {
        if (!point.is_hold_point) {
            ++combo;
        }
        const auto multiplier = 1 + std::min(combo / 10, 3U);
        point.value *= multiplier;
    }

    return points;
}

std::vector<ProcessedTrack::BeatRate>
ProcessedTrack::form_beat_rates(const SongHeader& header,
                                const SyncTrack& sync_track)
{
    constexpr double DEFAULT_BEAT_RATE = 4.0;
    constexpr double MEASURES_PER_BAR = 8.0;

    std::vector<BeatRate> beat_rates;
    beat_rates.reserve(sync_track.time_sigs().size());

    for (const auto& ts : sync_track.time_sigs()) {
        const auto pos = static_cast<double>(ts.position) / header.resolution();
        const auto measure_rate
            = ts.numerator * DEFAULT_BEAT_RATE / ts.denominator;
        const auto drain_rate
            = SP_GAIN_RATE - 1 / (MEASURES_PER_BAR * measure_rate);
        beat_rates.push_back({Beat(pos), drain_rate});
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
            m_whammy_ranges.push_back({Beat(start), Beat(end)});
        }
    }
}

double ProcessedTrack::propagate_sp_over_whammy(Beat start, Beat end,
                                                double sp_bar_amount) const
{
    constexpr double MEASURES_PER_BAR = 8.0;

    auto p = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                          [=](const auto& x) { return x.end_beat > start; });
    while ((p != m_whammy_ranges.cend()) && (p->start_beat < end)) {
        if (p->start_beat > start) {
            auto meas_diff = m_converter.beats_to_measures(p->start_beat)
                - m_converter.beats_to_measures(start);
            sp_bar_amount -= meas_diff.value() / MEASURES_PER_BAR;
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
    sp_bar_amount -= meas_diff.value() / MEASURES_PER_BAR;
    if (sp_bar_amount < 0.0) {
        return -1.0;
    }
    return sp_bar_amount;
}

bool ProcessedTrack::is_candidate_valid(
    const ActivationCandidate& activation) const
{
    constexpr double MEASURES_PER_BAR = 8.0;

    if (activation.max_sp_bar_amount < MINIMUM_SP_AMOUNT) {
        return false;
    }

    auto current_position = activation.act_start->beat_position;
    auto min_sp = std::max(activation.min_sp_bar_amount, MINIMUM_SP_AMOUNT);
    auto max_sp = activation.max_sp_bar_amount;
    auto starting_meas_diff = m_converter.beats_to_measures(current_position)
        - m_converter.beats_to_measures(activation.earliest_activation_point);
    min_sp -= starting_meas_diff.value() / MEASURES_PER_BAR;
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
            min_sp
                = std::max(min_sp - meas_diff.value() / MEASURES_PER_BAR, 0.0);
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
    min_sp -= meas_diff.value() / MEASURES_PER_BAR;

    return min_sp < 0.0;
}

double ProcessedTrack::propagate_over_whammy_range(Beat start, Beat end,
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

std::tuple<double, double> ProcessedTrack::total_available_sp(
    Beat start, std::vector<Point>::const_iterator act_start) const
{
    auto min_sp = 0.0;
    auto p
        = std::find_if(m_points.cbegin(), m_points.cend(),
                       [=](const auto& x) { return x.beat_position >= start; });
    for (; p < act_start; ++p) {
        if (p->is_sp_granting_note) {
            min_sp += SP_PHRASE_AMOUNT;
        }
    }

    auto max_sp = min_sp;
    auto q = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                          [=](const auto& x) { return x.end_beat > start; });
    while (q < m_whammy_ranges.cend()) {
        if (q->start_beat >= act_start->beat_position) {
            break;
        }
        auto end = std::min(q->end_beat, act_start->beat_position);
        max_sp += (end - q->start_beat).value() * SP_GAIN_RATE;
        ++q;
    }

    min_sp = std::min(min_sp, 1.0);
    max_sp = std::min(max_sp, 1.0);

    return {min_sp, max_sp};
}

std::tuple<uint32_t, std::vector<Activation>> ProcessedTrack::get_partial_path(
    std::vector<Point>::const_iterator point,
    std::map<std::vector<Point>::const_iterator,
             std::tuple<uint32_t, std::vector<Activation>>>& partial_paths)
    const
{
    if (partial_paths.find(point) == partial_paths.end()) {
        add_point_to_partial_acts(point, partial_paths);
    }
    return partial_paths.at(point);
}

void ProcessedTrack::add_point_to_partial_acts(
    std::vector<Point>::const_iterator point,
    std::map<std::vector<Point>::const_iterator,
             std::tuple<uint32_t, std::vector<Activation>>>& partial_paths)
    const
{
    auto starting_beat = point->beat_position;
    std::vector<std::tuple<uint32_t, std::vector<Activation>>> paths;

    for (auto p = point; p < m_points.cend(); ++p) {
        auto [min_sp, max_sp] = total_available_sp(starting_beat, p);
        if (max_sp < MINIMUM_SP_AMOUNT) {
            continue;
        }
        std::vector<std::tuple<uint32_t, std::vector<Point>::const_iterator,
                               std::vector<Point>::const_iterator>>
            acts;
        for (auto start = p; start < m_points.cend(); ++start) {
            auto q = m_points.cend() - 1;
            while (true) {
                ActivationCandidate candidate {start, q, starting_beat, min_sp,
                                               max_sp};
                if (is_candidate_valid(candidate)) {
                    break;
                }
                --q;
            }
            assert(start <= q);
            auto act_score = std::accumulate(
                start, q + 1, 0U,
                [](const auto& sum, const auto& x) { return sum + x.value; });
            auto rest_of_path = get_partial_path(q + 1, partial_paths);
            auto score = act_score + std::get<0>(rest_of_path);
            auto act_set = std::get<1>(rest_of_path);
            act_set.insert(act_set.begin(), {start, q});
            paths.emplace_back(score, act_set);
        }
    }

    auto final_act = std::max_element(
        paths.cbegin(), paths.cend(), [](const auto& x, const auto& y) {
            return std::get<0>(x) < std::get<0>(y);
        });
    if (final_act != paths.cend()) {
        partial_paths[point] = *final_act;
    } else {
        partial_paths[point] = {0, {}};
    }
}

std::vector<Activation> ProcessedTrack::optimal_path() const
{
    std::map<std::vector<Point>::const_iterator,
             std::tuple<uint32_t, std::vector<Activation>>>
        partial_paths;
    partial_paths[m_points.cend()] = {0, {}};

    return std::get<1>(get_partial_path(m_points.cbegin(), partial_paths));
}

Beat front_end(const Point& point, const TimeConverter& converter)
{
    constexpr double FRONT_END = 0.07;

    if (point.is_hold_point) {
        return point.beat_position;
    }

    auto time = converter.beats_to_seconds(point.beat_position).value();
    time -= FRONT_END;
    return converter.seconds_to_beats(Second(time));
}

Beat back_end(const Point& point, const TimeConverter& converter)
{
    constexpr double BACK_END = 0.07;

    if (point.is_hold_point) {
        return point.beat_position;
    }

    auto time = converter.beats_to_seconds(point.beat_position).value();
    time += BACK_END;
    return converter.seconds_to_beats(Second(time));
}
