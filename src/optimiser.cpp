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
#include <set>

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

static std::vector<Measure>
form_point_measures(const std::vector<Point>& points,
                    const TimeConverter& converter)
{
    std::vector<Measure> locations;
    locations.reserve(points.size());

    for (const auto& point : points) {
        locations.push_back(converter.beats_to_measures(point.beat_position));
    }

    return locations;
}

ProcessedTrack::ProcessedTrack(const NoteTrack& track, const SongHeader& header,
                               const SyncTrack& sync_track)
    : m_points {notes_to_points(track, header)}
    , m_converter {TimeConverter(sync_track, header)}
    , m_beat_rates {form_beat_rates(header, sync_track)}
{
    std::vector<std::tuple<uint32_t, uint32_t>> ranges_as_ticks;
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
            auto start_meas = m_converter.beats_to_measures(Beat(start));
            auto end_meas = m_converter.beats_to_measures(Beat(end));
            m_whammy_ranges.push_back(
                {Beat(start), Beat(end), start_meas, end_meas});
        }
    }

    m_point_measures = form_point_measures(m_points, m_converter);
}

double ProcessedTrack::propagate_sp_over_whammy(Beat start, Beat end,
                                                Measure start_meas,
                                                Measure end_meas,
                                                double sp_bar_amount) const
{
    constexpr double MEASURES_PER_BAR = 8.0;

    auto p = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                          [=](const auto& x) { return x.end_beat > start; });
    while ((p != m_whammy_ranges.cend()) && (p->start_beat < end)) {
        if (p->start_beat > start) {
            auto meas_diff = p->start_meas - start_meas;
            sp_bar_amount -= meas_diff.value() / MEASURES_PER_BAR;
            if (sp_bar_amount < 0.0) {
                return -1.0;
            }
            start = p->start_beat;
            start_meas = p->start_meas;
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
        start_meas = p->end_meas;
        ++p;
    }

    auto meas_diff = end_meas - start_meas;
    sp_bar_amount -= meas_diff.value() / MEASURES_PER_BAR;
    if (sp_bar_amount < 0.0) {
        return -1.0;
    }
    return sp_bar_amount;
}

static Measure point_to_measure(const std::vector<Point>& points,
                                const std::vector<Measure>& point_meas,
                                PointPtr point)
{
    return point_meas[static_cast<size_t>(
        std::distance(points.cbegin(), point))];
}

PointPtr ProcessedTrack::furthest_reachable_point(PointPtr point,
                                                  double sp) const
{
    auto current_position = point->beat_position;
    auto current_meas_position
        = point_to_measure(m_points, m_point_measures, point);

    for (auto p = point; p < m_points.cend(); ++p) {
        if (p->is_sp_granting_note) {
            auto p_meas_position
                = point_to_measure(m_points, m_point_measures, p);
            sp = propagate_sp_over_whammy(current_position, p->beat_position,
                                          current_meas_position,
                                          p_meas_position, sp);
            if (sp < 0.0) {
                return p - 1;
            }
            sp = std::min(sp + SP_PHRASE_AMOUNT, 1.0);
            current_position = p->beat_position;
            current_meas_position = p_meas_position;
        }
    }

    return m_points.cend() - 1;
}

bool ProcessedTrack::is_candidate_valid(
    const ActivationCandidate& activation) const
{
    constexpr double MEASURES_PER_BAR = 8.0;

    if (activation.max_sp_bar_amount < MINIMUM_SP_AMOUNT) {
        return false;
    }

    auto current_position = activation.act_start->beat_position;
    auto current_meas_position
        = point_to_measure(m_points, m_point_measures, activation.act_start);

    auto min_sp = std::max(activation.min_sp_bar_amount, MINIMUM_SP_AMOUNT);
    auto max_sp = activation.max_sp_bar_amount;

    auto starting_meas_diff = current_meas_position
        - m_converter.beats_to_measures(activation.earliest_activation_point);
    min_sp -= starting_meas_diff.value() / MEASURES_PER_BAR;
    min_sp = std::max(min_sp, 0.0);

    for (auto p = activation.act_start; p < activation.act_end; ++p) {
        if (p->is_sp_granting_note) {
            auto p_meas_position
                = point_to_measure(m_points, m_point_measures, p);
            max_sp = propagate_sp_over_whammy(
                current_position, p->beat_position, current_meas_position,
                p_meas_position, max_sp);
            if (max_sp < 0.0) {
                return false;
            }
            auto meas_diff = p_meas_position - current_meas_position;
            min_sp
                = std::max(min_sp - meas_diff.value() / MEASURES_PER_BAR, 0.0);
            min_sp = std::min(min_sp + SP_PHRASE_AMOUNT, 1.0);
            max_sp = std::min(max_sp + SP_PHRASE_AMOUNT, 1.0);
            current_position = p->beat_position;
            current_meas_position = p_meas_position;
        }
    }

    auto end_meas
        = point_to_measure(m_points, m_point_measures, activation.act_end);
    max_sp = propagate_sp_over_whammy(current_position,
                                      activation.act_end->beat_position,
                                      current_meas_position, end_meas, max_sp);
    if (max_sp < 0.0) {
        return false;
    }

    const auto next_point = std::next(activation.act_end);
    if (next_point == m_points.cend()) {
        return true;
    }

    auto meas_diff = point_to_measure(m_points, m_point_measures, next_point)
        - current_meas_position;
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

std::tuple<double, double>
ProcessedTrack::total_available_sp(Beat start, PointPtr act_start) const
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

bool ProcessedTrack::is_in_whammy_ranges(Beat beat) const
{
    auto p = std::find_if(m_whammy_ranges.cbegin(), m_whammy_ranges.cend(),
                          [=](const auto& x) { return x.end_beat >= beat; });
    if (p == m_whammy_ranges.cend()) {
        return false;
    }
    return p->start_beat <= beat;
}

PointPtr ProcessedTrack::next_candidate_point(PointPtr point) const
{
    while (point != m_points.cend()) {
        if (point->is_sp_granting_note) {
            return point;
        }
        if (point->is_hold_point && is_in_whammy_ranges(point->beat_position)) {
            return point;
        }
        ++point;
    }
    return m_points.cend();
}

Path ProcessedTrack::get_partial_path(
    PointPtr point, std::map<PointPtr, Path>& partial_paths) const
{
    point = next_candidate_point(point);
    if (partial_paths.find(point) == partial_paths.end()) {
        add_point_to_partial_acts(point, partial_paths);
    }
    return partial_paths.at(point);
}

void ProcessedTrack::add_point_to_partial_acts(
    PointPtr point, std::map<PointPtr, Path>& partial_paths) const
{
    auto starting_beat = point->beat_position;
    std::vector<Path> paths;

    std::set<PointPtr> attained_act_ends;

    for (auto p = point; p < m_points.cend(); ++p) {
        auto [min_sp, max_sp] = total_available_sp(starting_beat, p);
        if (max_sp < MINIMUM_SP_AMOUNT) {
            continue;
        }
        std::vector<std::tuple<uint32_t, PointPtr, PointPtr>> acts;
        auto max_q = furthest_reachable_point(p, max_sp);
        for (auto q = p; q <= max_q; ++q) {
            if (attained_act_ends.find(q) != attained_act_ends.end()) {
                continue;
            }
            ActivationCandidate candidate {p, q, starting_beat, min_sp, max_sp};
            if (!is_candidate_valid(candidate)) {
                continue;
            }
            attained_act_ends.insert(q);

            auto act_score = std::accumulate(
                p, std::next(q), 0U,
                [](const auto& sum, const auto& x) { return sum + x.value; });
            auto rest_of_path = get_partial_path(std::next(q), partial_paths);
            auto score = act_score + rest_of_path.score_boost;
            auto act_set = rest_of_path.activations;
            act_set.insert(act_set.begin(), {p, q});
            paths.push_back({act_set, score});
        }
    }

    auto final_act = std::max_element(paths.cbegin(), paths.cend(),
                                      [](const auto& x, const auto& y) {
                                          return x.score_boost < y.score_boost;
                                      });
    if (final_act != paths.cend()) {
        partial_paths[point] = *final_act;
    } else {
        partial_paths[point] = {{}, 0};
    }
}

Path ProcessedTrack::optimal_path() const
{
    std::map<PointPtr, Path> partial_paths;
    partial_paths[m_points.cend()] = {{}, 0};

    return get_partial_path(m_points.cbegin(), partial_paths);
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
