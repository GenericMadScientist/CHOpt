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
#include <numeric>
#include <set>

#include "optimiser.hpp"

static bool phrase_contains_pos(const StarPower& phrase, std::uint32_t position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

template <class InputIt, class OutputIt>
static void append_note_points(InputIt first, InputIt last, OutputIt points,
                               std::int32_t resolution, bool is_note_sp_ender,
                               const TimeConverter& converter)
{
    constexpr auto NOTE_VALUE = 50U;
    const double float_res = resolution;
    const auto tick_gap = std::max(resolution / 25, 1);

    const auto chord_size
        = static_cast<std::uint32_t>(std::distance(first, last));
    auto chord_length = static_cast<std::int32_t>(
        std::max_element(first, last, [](const auto& x, const auto& y) {
            return x.length < y.length;
        })->length);
    auto pos = first->position;
    auto beat = Beat(pos / float_res);
    auto meas = converter.beats_to_measures(beat);
    *points++
        = {{beat, meas}, NOTE_VALUE * chord_size, false, is_note_sp_ender};
    while (chord_length > 0) {
        pos += static_cast<std::uint32_t>(tick_gap);
        chord_length -= tick_gap;
        beat = Beat(pos / float_res);
        meas = converter.beats_to_measures(beat);
        *points++ = {{beat, meas}, 1, true, false};
    }
}

static std::vector<Point> notes_to_points(const NoteTrack& track,
                                          std::int32_t resolution,
                                          const TimeConverter& converter)
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
        append_note_points(p, q, std::back_inserter(points), resolution,
                           is_note_sp_ender, converter);
        p = q;
    }

    std::stable_sort(points.begin(), points.end(),
                     [](const auto& x, const auto& y) {
                         return x.position.beat < y.position.beat;
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

ProcessedTrack::ProcessedTrack(const NoteTrack& track, std::int32_t resolution,
                               const SyncTrack& sync_track)
    : m_converter {TimeConverter(sync_track, resolution)}
    , m_points {notes_to_points(track, resolution, m_converter)}
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

bool ProcessedTrack::is_candidate_valid(
    const ActivationCandidate& activation) const
{
    constexpr double MEASURES_PER_BAR = 8.0;
    constexpr double MINIMUM_SP_AMOUNT = 0.5;

    if (!activation.sp_bar.full_enough_to_activate()) {
        return false;
    }

    auto current_position = activation.act_start->position;

    auto sp_bar = activation.sp_bar;
    sp_bar.min() = std::max(sp_bar.min(), MINIMUM_SP_AMOUNT);

    auto starting_meas_diff = current_position.measure
        - activation.earliest_activation_point.measure;
    sp_bar.min() -= starting_meas_diff.value() / MEASURES_PER_BAR;
    sp_bar.min() = std::max(sp_bar.min(), 0.0);

    for (auto p = activation.act_start; p < activation.act_end; ++p) {
        if (p->is_sp_granting_note) {
            sp_bar = m_sp_data.propagate_sp_over_whammy(current_position,
                                                        p->position, sp_bar);
            if (sp_bar.max() < 0.0) {
                return false;
            }
            sp_bar.add_phrase();
            current_position = p->position;
        }
    }

    sp_bar = m_sp_data.propagate_sp_over_whammy(
        current_position, activation.act_end->position, sp_bar);
    if (sp_bar.max() < 0.0) {
        return false;
    }
    if (activation.act_end->is_sp_granting_note) {
        sp_bar.add_phrase();
    }

    const auto next_point = std::next(activation.act_end);
    if (next_point == m_points.cend()) {
        return true;
    }

    auto end_meas = activation.act_end->position.measure;
    auto meas_diff = next_point->position.measure - end_meas;
    sp_bar.min() -= meas_diff.value() / MEASURES_PER_BAR;

    return sp_bar.min() < 0.0;
}

SpBar ProcessedTrack::total_available_sp(Beat start, PointPtr act_start) const
{
    SpBar sp_bar {0.0, 0.0};
    auto p
        = std::find_if(m_points.cbegin(), m_points.cend(),
                       [=](const auto& x) { return x.position.beat >= start; });
    for (; p < act_start; ++p) {
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
    const auto starting_beat = point->position.beat;
    auto starting_pos = point->position;
    std::vector<Path> paths;

    std::set<PointPtr> attained_act_ends;

    for (auto p = point; p < m_points.cend(); ++p) {
        auto sp_bar = total_available_sp(starting_beat, p);
        if (!sp_bar.full_enough_to_activate()) {
            continue;
        }
        auto max_q = furthest_reachable_point(p, sp_bar.max());
        for (auto q = p; q <= max_q; ++q) {
            if (attained_act_ends.find(q) != attained_act_ends.end()) {
                continue;
            }
            ActivationCandidate candidate {p, q, starting_pos, sp_bar};
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

std::string ProcessedTrack::path_summary(const Path& path) const
{
    std::string output = "Path: ";

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
        output += "None";
    } else {
        output += activation_summaries[0];
        for (std::size_t i = 1; i < activation_summaries.size(); ++i) {
            output += "-";
            output += activation_summaries[i];
        }
    }

    auto no_sp_score = std::accumulate(
        m_points.cbegin(), m_points.cend(), 0U,
        [](const auto x, const auto& y) { return x + y.value; });
    no_sp_score += m_total_solo_boost;
    output += "\nNo SP score: ";
    output += std::to_string(no_sp_score);

    auto total_score = no_sp_score + path.score_boost;
    output += "\nTotal score: ";
    output += std::to_string(total_score);

    return output;
}
