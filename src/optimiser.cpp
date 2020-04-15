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

#include <cassert>
#include <iterator>
#include <limits>
#include <numeric>
#include <set>
#include <sstream>

#include "optimiser.hpp"

ProcessedTrack::ProcessedTrack(const NoteTrack& track, std::int32_t resolution,
                               const SyncTrack& sync_track)
    : m_converter {TimeConverter(sync_track, resolution)}
    , m_points {track, resolution, m_converter}
    , m_sp_data {track, resolution, sync_track}
{
    m_total_solo_boost = std::accumulate(
        track.solos().cbegin(), track.solos().cend(), 0U,
        [](const auto x, const auto& y) { return x + y.value; });
}

PointPtr ProcessedTrack::furthest_reachable_point(PointPtr point,
                                                  double sp) const
{
    SpBar sp_bar {0.0, sp};
    auto current_position = point->position;

    for (auto p = point; p < m_points.cend(); ++p) {
        if (p->is_sp_granting_note) {
            sp_bar = m_sp_data.propagate_sp_over_whammy(current_position,
                                                        p->position, sp_bar);
            if (sp_bar.max() < 0.0) {
                return p - 1;
            }
            sp_bar.add_phrase();
            current_position = p->position;
        }
    }

    return m_points.cend() - 1;
}

std::optional<Position>
ProcessedTrack::is_candidate_valid(const ActivationCandidate& activation) const
{
    constexpr double MEASURES_PER_BAR = 8.0;
    constexpr double MINIMUM_SP_AMOUNT = 0.5;
    constexpr double SP_PHRASE_AMOUNT = 0.25;

    if (!activation.sp_bar.full_enough_to_activate()) {
        return {};
    }

    auto current_beat = hit_window_end(*activation.act_start, m_converter);
    auto current_measure = m_converter.beats_to_measures(current_beat);
    auto current_position = Position {current_beat, current_measure};

    auto sp_bar = activation.sp_bar;
    sp_bar.min() = std::max(sp_bar.min(), MINIMUM_SP_AMOUNT);

    auto starting_meas_diff = current_position.measure
        - activation.earliest_activation_point.measure;
    sp_bar.min() -= starting_meas_diff.value() / MEASURES_PER_BAR;
    sp_bar.min() = std::max(sp_bar.min(), 0.0);

    for (auto p = activation.act_start; p < activation.act_end; ++p) {
        if (p->is_sp_granting_note) {
            auto sp_note_beat = hit_window_start(*p, m_converter);
            auto sp_note_meas = m_converter.beats_to_measures(sp_note_beat);
            auto sp_note_pos = Position {sp_note_beat, sp_note_meas};
            sp_bar = m_sp_data.propagate_sp_over_whammy(current_position,
                                                        sp_note_pos, sp_bar);
            if (sp_bar.max() < 0.0) {
                return {};
            }

            auto sp_note_end_beat = hit_window_end(*p, m_converter);
            auto sp_note_end_meas
                = m_converter.beats_to_measures(sp_note_end_beat);
            auto sp_note_end_pos
                = Position {sp_note_end_beat, sp_note_end_meas};

            auto latest_point_to_hit_sp = m_sp_data.activation_end_point(
                sp_note_pos, sp_note_end_pos, sp_bar.max());
            sp_bar.min() += SP_PHRASE_AMOUNT;
            sp_bar.min() = std::min(sp_bar.min(), 1.0);
            sp_bar = m_sp_data.propagate_sp_over_whammy(
                sp_note_pos, latest_point_to_hit_sp, sp_bar);
            sp_bar.max() += SP_PHRASE_AMOUNT;
            sp_bar.max() = std::min(sp_bar.max(), 1.0);

            current_position = latest_point_to_hit_sp;
        }
    }

    auto ending_beat = hit_window_start(*activation.act_end, m_converter);
    auto ending_meas = m_converter.beats_to_measures(ending_beat);
    sp_bar = m_sp_data.propagate_sp_over_whammy(
        current_position, {ending_beat, ending_meas}, sp_bar);
    if (sp_bar.max() < 0.0) {
        return {};
    }
    if (activation.act_end->is_sp_granting_note) {
        sp_bar.add_phrase();
    }

    const auto next_point = std::next(activation.act_end);
    if (next_point == m_points.cend()) {
        // Return value doesn't matter other than it being non-empty.
        auto pos_inf = std::numeric_limits<double>::infinity();
        return Position {Beat(pos_inf), Measure(pos_inf)};
    }

    auto next_point_beat = hit_window_end(*next_point, m_converter);
    auto next_point_meas = m_converter.beats_to_measures(next_point_beat);
    auto end_meas = ending_meas + Measure(sp_bar.min() * MEASURES_PER_BAR);
    if (end_meas >= next_point_meas) {
        return {};
    }

    auto end_beat = m_converter.measures_to_beats(end_meas);
    return Position {end_beat, end_meas};
}

SpBar ProcessedTrack::total_available_sp(Beat start, PointPtr first_point,
                                         PointPtr act_start) const
{
    SpBar sp_bar {0.0, 0.0};
    for (auto p = first_point; p < act_start; ++p) {
        if (p->is_sp_granting_note) {
            sp_bar.add_phrase();
        }
    }

    sp_bar.max() += m_sp_data.available_whammy(start, act_start->position.beat);
    sp_bar.max() = std::min(sp_bar.max(), 1.0);

    return sp_bar;
}

PointPtr ProcessedTrack::next_candidate_point(PointPtr point) const
{
    while (point != m_points.cend()) {
        if (point->is_sp_granting_note) {
            return point;
        }
        if (point->is_hold_point
            && m_sp_data.is_in_whammy_ranges(point->position.beat)) {
            return point;
        }
        ++point;
    }
    return m_points.cend();
}

Path ProcessedTrack::get_partial_path(CacheKey key, Cache& cache) const
{
    key.point = next_candidate_point(key.point);
    if (key.point == m_points.cend()) {
        return {{}, 0};
    }
    const auto beat = hit_window_start(*key.point, m_converter);
    if (beat >= key.position.beat) {
        key.position = {beat, m_converter.beats_to_measures(beat)};
    }
    if (cache.paths.find(key) == cache.paths.end()) {
        add_point_to_partial_acts(key, cache);
    }
    return cache.paths.at(key);
}

Path ProcessedTrack::get_partial_full_sp_path(PointPtr point,
                                              Cache& cache) const
{
    // We only call this from add_point_to_partial_paths in a situaiton where we
    // know point is not m_points.cend(), so we may assume point points to valid
    // memory.
    if (cache.full_sp_paths.find(point) != cache.full_sp_paths.end()) {
        return cache.full_sp_paths.at(point);
    }

    std::vector<Path> paths;
    std::set<PointPtr> attained_act_ends;
    auto sp_bar = SpBar {1.0, 1.0};

    for (auto p = point; p < m_points.cend(); ++p) {
        if (p != point && std::prev(p)->is_sp_granting_note) {
            paths.push_back(get_partial_full_sp_path(p, cache));
            break;
        }
        auto starting_beat = hit_window_start(*std::prev(p), m_converter);
        auto starting_meas = m_converter.beats_to_measures(starting_beat);
        auto starting_pos = Position {starting_beat, starting_meas};
        auto max_q = furthest_reachable_point(p, 1.0);
        for (auto q = p; q <= max_q; ++q) {
            if (attained_act_ends.find(q) != attained_act_ends.end()) {
                continue;
            }

            ActivationCandidate candidate {p, q, starting_pos, sp_bar};
            auto candidate_result = is_candidate_valid(candidate);
            if (!candidate_result) {
                continue;
            }

            attained_act_ends.insert(q);

            auto act_score = std::accumulate(
                p, std::next(q), 0U,
                [](const auto& sum, const auto& x) { return sum + x.value; });
            auto rest_of_path
                = get_partial_path({std::next(q), *candidate_result}, cache);
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

    // We can never have final_act be paths.cend() because paths must always be
    // nonempty: at the very least, since we have full SP we could activate on
    // point, giving us the start of one path. Therefore unlike
    // add_point_to_partial_paths we do not need to check for this.
    cache.full_sp_paths[point] = *final_act;
    return *final_act;
}

void ProcessedTrack::add_point_to_partial_acts(CacheKey key, Cache& cache) const
{
    std::vector<Path> paths;
    std::set<PointPtr> attained_act_ends;

    for (auto p = key.point; p < m_points.cend(); ++p) {
        auto sp_bar = total_available_sp(key.position.beat, key.point, p);
        if (!sp_bar.full_enough_to_activate()) {
            continue;
        }
        if (sp_bar.max() == 1.0) {
            paths.push_back(get_partial_full_sp_path(p, cache));
            break;
        }
        auto starting_beat = hit_window_start(*std::prev(p), m_converter);
        auto starting_meas = m_converter.beats_to_measures(starting_beat);
        auto starting_pos = Position {starting_beat, starting_meas};
        auto max_q = furthest_reachable_point(p, sp_bar.max());
        for (auto q = p; q <= max_q; ++q) {
            if (attained_act_ends.find(q) != attained_act_ends.end()) {
                continue;
            }

            ActivationCandidate candidate {p, q, starting_pos, sp_bar};
            auto candidate_result = is_candidate_valid(candidate);
            if (!candidate_result) {
                continue;
            }

            attained_act_ends.insert(q);

            auto act_score = std::accumulate(
                p, std::next(q), 0U,
                [](const auto& sum, const auto& x) { return sum + x.value; });
            auto rest_of_path
                = get_partial_path({std::next(q), *candidate_result}, cache);
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
        cache.paths[key] = *final_act;
    } else {
        cache.paths[key] = {{}, 0};
    }
}

Path ProcessedTrack::optimal_path() const
{
    Cache cache;
    auto neg_inf = -std::numeric_limits<double>::infinity();

    return get_partial_path(
        {m_points.cbegin(), {Beat(neg_inf), Measure(neg_inf)}}, cache);
}

std::string ProcessedTrack::path_summary(const Path& path) const
{
    // We use std::stringstream instead of std::string for better formating of
    // floats (measure values).
    std::stringstream stream;
    stream << "Path: ";

    std::vector<std::string> activation_summaries;
    auto start_point = m_points.cbegin();
    for (const auto& act : path.activations) {
        auto sp_before
            = std::count_if(start_point, act.act_start, [](const auto& p) {
                  return p.is_sp_granting_note;
              });
        auto sp_during = std::count_if(
            act.act_start, std::next(act.act_end),
            [](const auto& p) { return p.is_sp_granting_note; });
        auto summary = std::to_string(sp_before);
        if (sp_during != 0) {
            summary += "(+";
            summary += std::to_string(sp_during);
            summary += ")";
        }
        activation_summaries.push_back(summary);
        start_point = std::next(act.act_end);
    }

    auto spare_sp
        = std::count_if(start_point, m_points.cend(),
                        [](const auto& p) { return p.is_sp_granting_note; });
    if (spare_sp != 0) {
        activation_summaries.push_back(std::string("ES")
                                       + std::to_string(spare_sp));
    }

    if (activation_summaries.empty()) {
        stream << "None";
    } else {
        stream << activation_summaries[0];
        for (std::size_t i = 1; i < activation_summaries.size(); ++i) {
            stream << "-" << activation_summaries[i];
        }
    }

    auto no_sp_score = std::accumulate(
        m_points.cbegin(), m_points.cend(), 0U,
        [](const auto x, const auto& y) { return x + y.value; });
    no_sp_score += m_total_solo_boost;
    stream << "\nNo SP score: " << no_sp_score;

    auto total_score = no_sp_score + path.score_boost;
    stream << "\nTotal score: " << total_score;

    for (std::size_t i = 0; i < path.activations.size(); ++i) {
        stream << "\nActivation " << i + 1 << ": Measure "
               << path.activations[i].act_start->position.measure.value() + 1
               << " to Measure "
               << path.activations[i].act_end->position.measure.value() + 1;
    }

    return stream.str();
}
