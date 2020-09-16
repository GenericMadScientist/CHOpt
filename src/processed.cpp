/*
 * CHOpt - Star Power optimiser for Clone Hero
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
#include <sstream>

#include "processed.hpp"

SpBar ProcessedSong::total_available_sp(Beat start, PointPtr first_point,
                                        PointPtr act_start,
                                        Beat required_whammy_end) const
{
    SpBar sp_bar {0.0, 0.0};
    for (auto p = m_points.next_sp_granting_note(first_point); p < act_start;
         p = m_points.next_sp_granting_note(std::next(p))) {
        sp_bar.add_phrase();
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

std::tuple<SpBar, Position> ProcessedSong::total_available_sp_with_earliest_pos(
    Beat start, PointPtr first_point, PointPtr act_start,
    Position earliest_potential_pos) const
{
    const Beat BEAT_EPSILON {0.0001};

    SpBar sp_bar {0.0, 0.0};
    for (auto p = m_points.next_sp_granting_note(first_point); p < act_start;
         p = m_points.next_sp_granting_note(std::next(p))) {
        sp_bar.add_phrase();
    }

    sp_bar.max()
        += m_sp_data.available_whammy(start, earliest_potential_pos.beat);
    sp_bar.max() = std::min(sp_bar.max(), 1.0);

    if (sp_bar.full_enough_to_activate()) {
        return {sp_bar, earliest_potential_pos};
    }

    const auto extra_sp_required = 0.5 - sp_bar.max();
    auto first_beat = earliest_potential_pos.beat;
    auto last_beat = act_start->position.beat;
    if (m_sp_data.available_whammy(first_beat, last_beat) < extra_sp_required) {
        return {sp_bar, earliest_potential_pos};
    }

    while (last_beat - first_beat > BEAT_EPSILON) {
        const auto mid_beat = (first_beat + last_beat) * 0.5;
        if (m_sp_data.available_whammy(earliest_potential_pos.beat, mid_beat)
            < extra_sp_required) {
            first_beat = mid_beat;
        } else {
            last_beat = mid_beat;
        }
    }

    sp_bar.max()
        += m_sp_data.available_whammy(earliest_potential_pos.beat, last_beat);
    sp_bar.max() = std::min(sp_bar.max(), 1.0);

    return {sp_bar,
            Position {last_beat, m_converter.beats_to_measures(last_beat)}};
}

ActResult
ProcessedSong::is_candidate_valid(const ActivationCandidate& activation) const
{
    return is_restricted_candidate_valid(activation, 1.0,
                                         {Beat(NEG_INF), Measure(NEG_INF)});
}

Position ProcessedSong::adjusted_hit_window_start(PointPtr point,
                                                  double squeeze) const
{
    assert((0.0 <= squeeze) && (squeeze <= 1.0)); // NOLINT

    if (squeeze == 1.0) {
        return point->hit_window_start;
    }

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

    if (squeeze == 1.0) {
        return point->hit_window_end;
    }

    auto mid = m_converter.beats_to_seconds(point->position.beat);
    auto end = m_converter.beats_to_seconds(point->hit_window_end.beat);
    auto adj_end_s = mid + (end - mid) * squeeze;
    auto adj_end_b = m_converter.seconds_to_beats(adj_end_s);
    auto adj_end_m = m_converter.beats_to_measures(adj_end_b);

    return {adj_end_b, adj_end_m};
}

class SpStatus {
private:
    Position m_position;
    double m_sp;

public:
    SpStatus(Position position, double sp)
        : m_position {position}
        , m_sp {sp}
    {
    }

    Position& position() { return m_position; }
    double& sp() { return m_sp; }

    void update_early_end(Position sp_note_start, const SpData& sp_data,
                          Position required_whammy_end)
    {
        constexpr double SP_PHRASE_AMOUNT = 0.25;

        m_sp = sp_data.propagate_sp_over_whammy_min(m_position, sp_note_start,
                                                    m_sp, required_whammy_end);
        m_sp += SP_PHRASE_AMOUNT;
        m_sp = std::min(m_sp, 1.0);
        if (sp_note_start.beat > m_position.beat) {
            m_position = sp_note_start;
        }
    }

    void update_late_end(Position sp_note_start, Position sp_note_end,
                         const SpData& sp_data)
    {
        constexpr double SP_PHRASE_AMOUNT = 0.25;

        if (sp_note_start.beat < m_position.beat) {
            sp_note_start = m_position;
        }

        m_sp = sp_data.propagate_sp_over_whammy_max(m_position, sp_note_start,
                                                    m_sp);
        if (m_sp < 0.0) {
            return;
        }
        // We might run out of SP between sp_note_start and sp_note_end. In this
        // case we just hit the note as early as possible.
        const auto new_sp = sp_data.propagate_sp_over_whammy_max(
            sp_note_start, sp_note_end, m_sp);
        if (new_sp >= 0.0) {
            m_sp = new_sp;
            m_position = sp_note_end;
        } else {
            m_position = sp_note_start;
        }
        m_sp += SP_PHRASE_AMOUNT;
        m_sp = std::min(m_sp, 1.0);
    }
};

ActResult ProcessedSong::is_restricted_candidate_valid(
    const ActivationCandidate& activation, double squeeze,
    Position required_whammy_end) const
{
    const Position null_position {Beat(0.0), Measure(0.0)};

    if (!activation.sp_bar.full_enough_to_activate()) {
        return {null_position, ActValidity::insufficient_sp};
    }

    SpStatus status_for_early_end {
        activation.earliest_activation_point,
        std::max(activation.sp_bar.min(), MINIMUM_SP_AMOUNT)};
    SpStatus status_for_late_end {
        adjusted_hit_window_end(activation.act_start, squeeze),
        activation.sp_bar.max()};

    status_for_late_end.sp()
        += m_sp_data.available_whammy(activation.earliest_activation_point.beat,
                                      status_for_late_end.position().beat);
    status_for_late_end.sp() = std::min(status_for_late_end.sp(), 1.0);

    for (auto p = m_points.next_sp_granting_note(activation.act_start);
         p < activation.act_end;
         p = m_points.next_sp_granting_note(std::next(p))) {
        const auto p_start = adjusted_hit_window_start(p, squeeze);
        const auto p_end = adjusted_hit_window_end(p, squeeze);
        status_for_late_end.update_late_end(p_start, p_end, m_sp_data);
        if (status_for_late_end.sp() < 0.0) {
            return {null_position, ActValidity::insufficient_sp};
        }
        status_for_early_end.update_early_end(p_start, m_sp_data,
                                              required_whammy_end);
    }

    const auto ending_pos
        = adjusted_hit_window_start(activation.act_end, squeeze);
    status_for_late_end.sp() = m_sp_data.propagate_sp_over_whammy_max(
        status_for_late_end.position(), ending_pos, status_for_late_end.sp());
    if (status_for_late_end.sp() < 0.0) {
        return {null_position, ActValidity::insufficient_sp};
    }
    status_for_early_end.sp() = m_sp_data.propagate_sp_over_whammy_min(
        status_for_early_end.position(), ending_pos, status_for_early_end.sp(),
        required_whammy_end);
    if (activation.act_end->is_sp_granting_note) {
        status_for_early_end.sp() += SP_PHRASE_AMOUNT;
        status_for_early_end.sp() = std::min(status_for_early_end.sp(), 1.0);
    }
    status_for_early_end.position() = ending_pos;

    const auto end_meas = status_for_early_end.position().measure
        + Measure(status_for_early_end.sp() * MEASURES_PER_BAR);

    const auto next_point = std::next(activation.act_end);
    if (next_point == m_points.cend()) {
        const auto end_beat = m_converter.measures_to_beats(end_meas);
        return {{end_beat, end_meas}, ActValidity::success};
    }

    if (end_meas >= adjusted_hit_window_end(next_point, squeeze).measure) {
        return {null_position, ActValidity::surplus_sp};
    }

    const auto end_beat = m_converter.measures_to_beats(end_meas);
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
