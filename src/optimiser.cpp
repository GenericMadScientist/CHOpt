/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2025, 2026 Raymond Wright
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
#include <stdexcept>

#include "optimiser.hpp"

Optimiser::Optimiser(const ProcessedSong* song,
                     const std::atomic<bool>* terminate, int speed,
                     SightRead::Second whammy_delay)
    : m_song {song}
    , m_terminate {terminate}
    , m_drum_fill_delay {BASE_DRUM_FILL_DELAY / speed}
    , m_whammy_delay {whammy_delay}
{
    if (m_song == nullptr || m_terminate == nullptr) {
        throw std::invalid_argument(
            "Optimiser ctor's arguments must be non-null");
    }
    const auto& points = m_song->points();
    const auto& sp_data = m_song->sp_data();

    const auto capacity = std::distance(points.cbegin(), points.cend()) + 1;
    m_next_candidate_points.reserve(static_cast<std::size_t>(capacity));
    int count = 0;
    for (const auto* p = points.cbegin(); p < points.cend(); ++p) { // NOLINT
        ++count;
        if (p->is_sp_granting_note
            || (p->is_hold_point
                && sp_data.is_in_whammy_ranges(p->position.beat))) {
            for (int i = 0; i < count; ++i) {
                m_next_candidate_points.push_back(p);
            }
            count = 0;
        }
    }

    ++count;
    for (int i = 0; i < count; ++i) {
        m_next_candidate_points.push_back(points.cend());
    }
}

Path Optimiser::optimal_path() const
{
    PathGraphVertex vertex {.point = m_song->points().cbegin(),
                            .position = {.beat = SightRead::Beat(NEG_INF),
                                         .sp_measure = SpMeasure(NEG_INF)},
                            .is_max_sp_vertex = false};
    const auto root_vertex = advance_graph_vertex(vertex);

    const auto graph = path_graph(root_vertex);
    return optimal_path_from_graph(graph);
}

OptimiserGraph Optimiser::path_graph(PathGraphVertex root_vertex) const
{
    auto F = [&](auto& graph, auto vertex) { return out_edges(graph, vertex); };
    return generate_optimal_graph<PathGraphVertex, std::vector<ProtoActivation>,
                                  decltype(F)>(root_vertex, F);
}

PointPtr Optimiser::next_candidate_point(PointPtr point) const
{
    const auto index = std::distance(m_song->points().cbegin(), point);
    return m_next_candidate_points.at(static_cast<std::size_t>(index));
}

PathGraphVertex Optimiser::advance_graph_vertex(PathGraphVertex vertex) const
{
    constexpr double POS_INF = std::numeric_limits<double>::infinity();

    vertex.point = next_candidate_point(vertex.point);
    if (vertex.point == m_song->points().cend()) {
        vertex.position = {.beat = SightRead::Beat {POS_INF},
                           .sp_measure = SpMeasure {POS_INF}};
        return vertex;
    }
    vertex = add_whammy_delay(vertex);

    const auto* point = vertex.point;
    if (point != m_song->points().cbegin()) {
        point = std::prev(point);
    }
    const auto pos = point->max_sqz_hit_window_start;

    if (pos.beat >= vertex.position.beat) {
        vertex.position = pos;
    }
    return vertex;
}

PathGraphVertex Optimiser::add_whammy_delay(PathGraphVertex vertex) const
{
    auto seconds = m_song->sp_time_map().to_seconds(vertex.position.beat);
    seconds += m_whammy_delay;
    vertex.position.beat = m_song->sp_time_map().to_beats(seconds);
    vertex.position.sp_measure
        = m_song->sp_time_map().to_sp_measures(vertex.position.beat);
    return vertex;
}

SightRead::Second
Optimiser::earliest_fill_appearance(PathGraphVertex vertex) const
{
    if (!m_song->is_drums() || vertex.is_max_sp_vertex) {
        return SightRead::Second(0.0);
    }

    int sp_count = 0;
    for (const auto* p = vertex.point; p < m_song->points().cend();
         ++p) { // NOLINT
        if (p->is_sp_granting_note) {
            ++sp_count;
            if (sp_count == 2) {
                return m_song->sp_time_map().to_seconds(
                           p->hit_window_start.beat)
                    + m_drum_fill_delay;
            }
        }
    }

    return SightRead::Second(0.0);
}

OutEdgeAggregate<PathGraphVertex, ProtoActivation>
Optimiser::out_edges(OptimiserGraph& graph, std::size_t vertex_id) const
{
    const auto vertex = graph.vertex_property(vertex_id);
    const auto early_act_bound = earliest_fill_appearance(vertex);
    PointPtrRangeSet attained_act_ends {vertex.point, m_song->points().cend()};
    auto lower_bound_set = false;

    OutEdgeAggregate<PathGraphVertex, ProtoActivation> optimal_out_edges;

    for (const auto* p = vertex.point; p < m_song->points().cend();
         ++p) { // NOLINT
        if (m_song->is_drums()
            && (!p->fill_start.has_value()
                || p->fill_start < early_act_bound)) {
            continue;
        }
        SpBar sp_bar {1.0, 1.0, m_song->sp_engine_values()};
        SpPosition starting_pos {.beat = SightRead::Beat {NEG_INF},
                                 .sp_measure = SpMeasure {NEG_INF}};
        if (p != m_song->points().cbegin()) {
            starting_pos = std::prev(p)->hit_window_start;
        }
        if (!vertex.is_max_sp_vertex) {
            const auto& [new_sp, new_pos]
                = m_song->total_available_sp_with_earliest_pos(
                    vertex.position.beat, vertex.point, p, starting_pos);
            sp_bar = new_sp;
            starting_pos = new_pos;
        }
        if (m_song->is_drums()) {
            starting_pos.beat
                = std::max(starting_pos.beat, p->hit_window_start.beat);
            starting_pos.sp_measure = std::max(starting_pos.sp_measure,
                                               p->hit_window_start.sp_measure);
        }
        if (!sp_bar.full_enough_to_activate()) {
            continue;
        }
        if (p != vertex.point && sp_bar.min() == 1.0
            && std::prev(p)->is_sp_granting_note) {
            PathGraphVertex full_sp_vertex {.point = p,
                                            .position
                                            = std::prev(p)->hit_window_start,
                                            .is_max_sp_vertex = true};
            optimal_out_edges.add_activation(full_sp_vertex, {}, 0);
            break;
        }
        // This skips some points that are too early to be an act end for the
        // earliest possible activation.
        if (!lower_bound_set) {
            const SpMeasure act_length {
                8.0
                * std::max(sp_bar.min(),
                           m_song->sp_engine_values().minimum_to_activate)};
            const auto earliest_act_end = starting_pos.sp_measure + act_length;
            const auto* earliest_pt_end = std::find_if_not(
                std::next(p), m_song->points().cend(), [&](const auto& pt) {
                    return pt.hit_window_end.sp_measure <= earliest_act_end;
                });
            --earliest_pt_end; // NOLINT
            attained_act_ends
                = PointPtrRangeSet {earliest_pt_end, m_song->points().cend()};
            lower_bound_set = true;
        }
        add_acts_from_starting_point(p, starting_pos, sp_bar, attained_act_ends,
                                     optimal_out_edges);
    }

    return optimal_out_edges;
}

void Optimiser::add_acts_from_starting_point(
    PointPtr starting_point, SpPosition starting_pos, SpBar sp_bar,
    PointPtrRangeSet& attained_act_ends,
    OutEdgeAggregate<PathGraphVertex, ProtoActivation>& optimal_out_edges) const
{
    for (const auto* q = attained_act_ends.lowest_absent_element();
         q < m_song->points().cend();) {
        if (attained_act_ends.contains(q)) {
            ++q; // NOLINT
            continue;
        }

        ActivationCandidate candidate {.act_start = starting_point,
                                       .act_end = q,
                                       .earliest_activation_point
                                       = starting_pos,
                                       .sp_bar = sp_bar};
        const auto candidate_result = m_song->is_candidate_valid(candidate);
        if (candidate_result.validity != ActValidity::insufficient_sp) {
            attained_act_ends.add(q);
        } else if (!q->is_hold_point) {
            // We cannot hit any later points if q is not a hold point, so
            // we are done.
            q = m_song->points().cend();
            continue;
        } else {
            // We cannot hit any subsequent hold point, so go straight to
            // the next non-hold point.
            q = m_song->points().next_non_hold_point(q);
            continue;
        }

        if (candidate_result.validity != ActValidity::success) {
            ++q; // NOLINT
            continue;
        }

        const auto act_score
            = m_song->points().range_score(starting_point, std::next(q));
        PathGraphVertex destination {
            .point = m_song->points().first_after_current_phrase(q),
            .position = candidate_result.ending_position,
            .is_max_sp_vertex = false};
        destination = advance_graph_vertex(destination);
        optimal_out_edges.add_activation(
            destination, {{.act_start = starting_point, .act_end = q}},
            act_score);

        ++q; // NOLINT
    }
}

Path Optimiser::optimal_path_from_graph(const OptimiserGraph& graph) const
{
    Path path {.activations = {}, .score_boost = 0};

    std::size_t src_vertex_id = graph.root_vertex_id();
    PathGraphVertex act_start_vertex = graph.vertex_property(src_vertex_id);
    while (true) {
        const auto& out_edges = graph.out_edges(src_vertex_id);
        if (out_edges.empty()) {
            break;
        }

        const auto& edge = out_edges.front();
        const auto dest_vertex_id = edge.dest_vertex_id;

        path.score_boost += edge.weight;

        const auto& acts = edge.property;
        if (acts.empty()) {
            src_vertex_id = dest_vertex_id;
            continue;
        }

        auto best_proto_act = acts.at(0);
        auto best_sqz_level
            = act_squeeze_level(best_proto_act, act_start_vertex);
        for (auto i = 1U; i < acts.size(); ++i) {
            const auto proto_act = acts.at(i);
            const auto sqz_level
                = act_squeeze_level(proto_act, act_start_vertex);
            if (sqz_level < best_sqz_level) {
                best_proto_act = proto_act;
                best_sqz_level = sqz_level;
            }
        }
        const auto min_whammy_force = forced_whammy_end(
            best_proto_act, act_start_vertex, best_sqz_level);
        const auto [start_pos, end_pos] = act_duration(
            best_proto_act, act_start_vertex, best_sqz_level, min_whammy_force);
        Activation act {.act_start = best_proto_act.act_start,
                        .act_end = best_proto_act.act_end,
                        .whammy_end = min_whammy_force.beat,
                        .sp_start = start_pos,
                        .sp_end = end_pos};

        const auto act_end_vertex = graph.vertex_property(dest_vertex_id);
        if (act_end_vertex.point != m_song->points().cend()) {
            const auto post_act_first_whammy
                = m_song->sp_data().next_whammy_point(
                    act_end_vertex.position.beat);
            act.sp_end = std::min(act.sp_end, post_act_first_whammy);
        }
        path.activations.push_back(act);

        src_vertex_id = dest_vertex_id;
        act_start_vertex = act_end_vertex;
    }

    return path;
}

double Optimiser::act_squeeze_level(ProtoActivation act,
                                    PathGraphVertex vertex) const
{
    constexpr double THRESHOLD = 0.01;

    auto min_sqz = 0.0;
    auto max_sqz = 1.0;
    // Determines what point controls how early we can go: the previous point on
    // guitar and the current point on drums.
    const auto* start_bound_point
        = m_song->is_drums() ? act.act_start : std::prev(act.act_start);
    while (max_sqz - min_sqz > THRESHOLD) {
        auto trial_sqz = (min_sqz + max_sqz) / 2;
        auto start_pos
            = m_song->adjusted_hit_window_start(start_bound_point, trial_sqz);
        if (start_pos.beat < vertex.position.beat) {
            start_pos = vertex.position;
        }

        const auto& [sp_bar, new_pos]
            = m_song->total_available_sp_with_earliest_pos(
                vertex.position.beat, vertex.point, act.act_start, start_pos);
        start_pos = new_pos;

        ActivationCandidate candidate {.act_start = act.act_start,
                                       .act_end = act.act_end,
                                       .earliest_activation_point = start_pos,
                                       .sp_bar = sp_bar};
        if (m_song->is_candidate_valid(candidate, trial_sqz).validity
            == ActValidity::success) {
            max_sqz = trial_sqz;
        } else {
            min_sqz = trial_sqz;
        }
    }
    return max_sqz;
}

SpPosition Optimiser::forced_whammy_end(ProtoActivation act,
                                        PathGraphVertex vertex,
                                        double sqz_level) const
{
    constexpr double POS_INF = std::numeric_limits<double>::infinity();
    constexpr double THRESHOLD = 0.01;

    const auto* next_point = std::next(act.act_end);

    if (next_point == m_song->points().cend()) {
        return {.beat = SightRead::Beat {POS_INF},
                .sp_measure = SpMeasure {POS_INF}};
    }

    const auto* prev_point = std::prev(act.act_start);
    auto min_whammy_force = vertex.position;
    auto max_whammy_force = next_point->hit_window_end;
    auto start_pos = m_song->adjusted_hit_window_start(prev_point, sqz_level);
    while ((max_whammy_force.beat - min_whammy_force.beat).value()
           > THRESHOLD) {
        auto mid_beat
            = (min_whammy_force.beat + max_whammy_force.beat) * (1.0 / 2);
        auto mid_meas = m_song->sp_time_map().to_sp_measures(mid_beat);
        SpPosition mid_pos {.beat = mid_beat, .sp_measure = mid_meas};
        auto sp_bar = m_song->total_available_sp(
            vertex.position.beat, vertex.point, act.act_start, mid_beat);
        ActivationCandidate candidate {.act_start = act.act_start,
                                       .act_end = act.act_end,
                                       .earliest_activation_point = start_pos,
                                       .sp_bar = sp_bar};
        auto result = m_song->is_candidate_valid(candidate, sqz_level, mid_pos);
        if (result.validity == ActValidity::success) {
            min_whammy_force = mid_pos;
        } else {
            max_whammy_force = mid_pos;
        }
    }

    return min_whammy_force;
}

std::tuple<SightRead::Beat, SightRead::Beat>
Optimiser::act_duration(ProtoActivation act, PathGraphVertex vertex,
                        double sqz_level, SpPosition min_whammy_force) const
{
    constexpr double THRESHOLD = 0.01;

    // Determines what point controls how early we can go: the previous point on
    // guitar and the current point on drums.
    const auto* start_bound_point
        = m_song->is_drums() ? act.act_start : std::prev(act.act_start);
    auto min_pos
        = m_song->adjusted_hit_window_start(start_bound_point, sqz_level);
    auto max_pos = m_song->adjusted_hit_window_end(act.act_start, sqz_level);
    auto sp_bar
        = m_song->total_available_sp(vertex.position.beat, vertex.point,
                                     act.act_start, min_whammy_force.beat);
    while ((max_pos.beat - min_pos.beat).value() > THRESHOLD) {
        auto trial_beat = (min_pos.beat + max_pos.beat) * (1.0 / 2);
        auto trial_meas = m_song->sp_time_map().to_sp_measures(trial_beat);
        SpPosition trial_pos {.beat = trial_beat, .sp_measure = trial_meas};
        ActivationCandidate candidate {.act_start = act.act_start,
                                       .act_end = act.act_end,
                                       .earliest_activation_point = trial_pos,
                                       .sp_bar = sp_bar};
        if (m_song->is_candidate_valid(candidate, sqz_level, min_whammy_force)
                .validity
            == ActValidity::success) {
            min_pos = trial_pos;
        } else {
            max_pos = trial_pos;
        }
    }

    ActivationCandidate candidate {.act_start = act.act_start,
                                   .act_end = act.act_end,
                                   .earliest_activation_point = min_pos,
                                   .sp_bar = sp_bar};
    auto result
        = m_song->is_candidate_valid(candidate, sqz_level, min_whammy_force);
    assert(result.validity == ActValidity::success); // NOLINT
    return {min_pos.beat, result.ending_position.beat};
}
