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
#include <limits>
#include <numeric>
#include <sstream>

#include "processed.hpp"

ProcessedSong::ProcessedSong(const NoteTrack& track, int resolution,
                             const SyncTrack& sync_track, double early_whammy,
                             double squeeze)
    : m_converter {sync_track, resolution}
    , m_points {track, resolution, m_converter, squeeze}
    , m_sp_data {track, resolution, sync_track, early_whammy}
{
    m_total_solo_boost = std::accumulate(
        track.solos().cbegin(), track.solos().cend(), 0,
        [](const auto x, const auto& y) { return x + y.value; });
}

SpBar ProcessedSong::total_available_sp(Beat start, PointPtr first_point,
                                        PointPtr act_start,
                                        Beat required_whammy_end) const
{
    SpBar sp_bar {0.0, 0.0};
    for (auto p = first_point; p < act_start; ++p) {
        if (p->is_sp_granting_note) {
            sp_bar.add_phrase();
        }
    }

    if (start >= required_whammy_end) {
        sp_bar.max()
            += m_sp_data.available_whammy(start, act_start->position.beat);
        sp_bar.max() = std::min(sp_bar.max(), 1.0);
    } else if (required_whammy_end >= act_start->position.beat) {
        sp_bar.min()
            += m_sp_data.available_whammy(start, act_start->position.beat);
        sp_bar.min() = std::min(sp_bar.min(), 1.0);
        sp_bar.max() = sp_bar.min();
    } else {
        sp_bar.min() += m_sp_data.available_whammy(start, required_whammy_end);
        sp_bar.min() = std::min(sp_bar.min(), 1.0);
        sp_bar.max() = sp_bar.min();
        sp_bar.max() += m_sp_data.available_whammy(required_whammy_end,
                                                   act_start->position.beat);
        sp_bar.max() = std::min(sp_bar.max(), 1.0);
    }

    return sp_bar;
}

ActResult
ProcessedSong::is_candidate_valid(const ActivationCandidate& activation) const
{
    constexpr double MEASURES_PER_BAR = 8.0;
    constexpr double MINIMUM_SP_AMOUNT = 0.5;
    constexpr double SP_PHRASE_AMOUNT = 0.25;
    const Position null_position {Beat(0.0), Measure(0.0)};

    if (!activation.sp_bar.full_enough_to_activate()) {
        return {null_position, ActValidity::insufficient_sp};
    }

    auto current_position = activation.act_start->hit_window_end;

    auto sp_bar = activation.sp_bar;
    sp_bar.min() = std::max(sp_bar.min(), MINIMUM_SP_AMOUNT);

    auto starting_meas_diff = current_position.measure
        - activation.earliest_activation_point.measure;
    sp_bar.min() -= starting_meas_diff.value() / MEASURES_PER_BAR;
    sp_bar.min() = std::max(sp_bar.min(), 0.0);

    for (auto p = activation.act_start; p < activation.act_end; ++p) {
        if (p->is_sp_granting_note) {
            auto sp_note_pos = p->hit_window_start;
            sp_bar = m_sp_data.propagate_sp_over_whammy(current_position,
                                                        sp_note_pos, sp_bar);
            if (sp_bar.max() < 0.0) {
                return {null_position, ActValidity::insufficient_sp};
            }

            auto sp_note_end_pos = p->hit_window_end;

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

    auto ending_pos = activation.act_end->hit_window_start;
    sp_bar = m_sp_data.propagate_sp_over_whammy(current_position, ending_pos,
                                                sp_bar);
    if (sp_bar.max() < 0.0) {
        return {null_position, ActValidity::insufficient_sp};
    }
    if (activation.act_end->is_sp_granting_note) {
        sp_bar.add_phrase();
    }

    const auto next_point = std::next(activation.act_end);
    if (next_point == m_points.cend()) {
        // Return value doesn't matter other than it being non-empty.
        auto pos_inf = std::numeric_limits<double>::infinity();
        return {{Beat(pos_inf), Measure(pos_inf)}, ActValidity::success};
    }

    auto end_meas
        = ending_pos.measure + Measure(sp_bar.min() * MEASURES_PER_BAR);
    if (end_meas >= next_point->hit_window_end.measure) {
        return {null_position, ActValidity::surplus_sp};
    }

    auto end_beat = m_converter.measures_to_beats(end_meas);
    return {{end_beat, end_meas}, ActValidity::success};
}

Position ProcessedSong::adjusted_hit_window_start(PointPtr point,
                                                  double squeeze) const
{
    assert((0.0 <= squeeze) && (squeeze <= 1.0)); // NOLINT

    auto start = m_converter.beats_to_seconds(point->hit_window_start.beat);
    auto mid = m_converter.beats_to_seconds(point->position.beat);
    auto adj_start_s = start + (mid - start) * (1.0 - squeeze);
    auto adj_start_b = m_converter.seconds_to_beats(adj_start_s);
    auto adj_start_m = m_converter.beats_to_measures(adj_start_b);

    return {adj_start_b, adj_start_m};
}

Position ProcessedSong::adjusted_hit_window_end(PointPtr point,
                                                double squeeze) const
{
    assert((0.0 <= squeeze) && (squeeze <= 1.0)); // NOLINT

    auto mid = m_converter.beats_to_seconds(point->position.beat);
    auto end = m_converter.beats_to_seconds(point->hit_window_end.beat);
    auto adj_end_s = mid + (end - mid) * squeeze;
    auto adj_end_b = m_converter.seconds_to_beats(adj_end_s);
    auto adj_end_m = m_converter.beats_to_measures(adj_end_b);

    return {adj_end_b, adj_end_m};
}

ActResult ProcessedSong::is_restricted_candidate_valid(
    const ActivationCandidate& activation, double squeeze,
    Position required_whammy_end) const
{
    constexpr double MEASURES_PER_BAR = 8.0;
    constexpr double MINIMUM_SP_AMOUNT = 0.5;
    constexpr double SP_PHRASE_AMOUNT = 0.25;
    const Position null_position {Beat(0.0), Measure(0.0)};

    if (!activation.sp_bar.full_enough_to_activate()) {
        return {null_position, ActValidity::insufficient_sp};
    }

    auto current_position
        = adjusted_hit_window_end(activation.act_start, squeeze);

    auto sp_bar = activation.sp_bar;
    sp_bar.min() = std::max(sp_bar.min(), MINIMUM_SP_AMOUNT);

    auto starting_meas_diff = current_position.measure
        - activation.earliest_activation_point.measure;
    sp_bar.min() -= starting_meas_diff.value() / MEASURES_PER_BAR;
    sp_bar.min() = std::max(sp_bar.min(), 0.0);

    for (auto p = activation.act_start; p < activation.act_end; ++p) {
        if (p->is_sp_granting_note) {
            auto sp_note_pos = adjusted_hit_window_start(p, squeeze);
            sp_bar = m_sp_data.propagate_sp_over_whammy(
                current_position, sp_note_pos, sp_bar, required_whammy_end);
            if (sp_bar.max() < 0.0) {
                return {null_position, ActValidity::insufficient_sp};
            }

            auto sp_note_end_pos = adjusted_hit_window_end(p, squeeze);

            auto latest_point_to_hit_sp = m_sp_data.activation_end_point(
                sp_note_pos, sp_note_end_pos, sp_bar.max());
            sp_bar.min() += SP_PHRASE_AMOUNT;
            sp_bar.min() = std::min(sp_bar.min(), 1.0);
            sp_bar = m_sp_data.propagate_sp_over_whammy(
                sp_note_pos, latest_point_to_hit_sp, sp_bar,
                required_whammy_end);
            sp_bar.max() += SP_PHRASE_AMOUNT;
            sp_bar.max() = std::min(sp_bar.max(), 1.0);

            current_position = latest_point_to_hit_sp;
        }
    }

    auto ending_pos = adjusted_hit_window_start(activation.act_end, squeeze);
    sp_bar = m_sp_data.propagate_sp_over_whammy(current_position, ending_pos,
                                                sp_bar, required_whammy_end);
    if (sp_bar.max() < 0.0) {
        return {null_position, ActValidity::insufficient_sp};
    }
    if (activation.act_end->is_sp_granting_note) {
        sp_bar.add_phrase();
    }

    const auto next_point = std::next(activation.act_end);
    auto end_meas
        = ending_pos.measure + Measure(sp_bar.min() * MEASURES_PER_BAR);
    if (next_point == m_points.cend()) {
        auto end_beat = m_converter.measures_to_beats(end_meas);
        return {{end_beat, end_meas}, ActValidity::success};
    }

    if (end_meas >= adjusted_hit_window_end(next_point, squeeze).measure) {
        return {null_position, ActValidity::surplus_sp};
    }

    auto end_beat = m_converter.measures_to_beats(end_meas);
    return {{end_beat, end_meas}, ActValidity::success};
}

std::string ProcessedSong::path_summary(const Path& path) const
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
        m_points.cbegin(), m_points.cend(), 0,
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
