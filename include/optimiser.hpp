/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023, 2026 Raymond Wright
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

#ifndef CHOPT_OPTIMISER_HPP
#define CHOPT_OPTIMISER_HPP

#include <atomic>
#include <limits>
#include <tuple>
#include <vector>

#include <boost/container_hash/hash.hpp>

#include <sightread/time.hpp>

#include "activationendset.hpp"
#include "pathgraph.hpp"
#include "points.hpp"
#include "processed.hpp"

struct PathGraphVertex {
    PointPtr point = nullptr;
    SpPosition position {.beat = SightRead::Beat {0.0},
                         .sp_measure = SpMeasure {0.0}};
    bool is_max_sp_vertex = false;

    [[nodiscard]] bool operator==(const PathGraphVertex& rhs) const
    {
        return std::tuple {point, position.beat.value(),
                           position.sp_measure.value(), is_max_sp_vertex}
        == std::tuple {rhs.point, rhs.position.beat.value(),
                       rhs.position.sp_measure.value(), rhs.is_max_sp_vertex};
    }

    friend std::size_t hash_value(const PathGraphVertex& vertex)
    {
        std::size_t seed = 0;

        boost::hash_combine(seed, vertex.point);
        boost::hash_combine(seed, vertex.position.beat.value());
        boost::hash_combine(seed, vertex.position.sp_measure.value());
        boost::hash_combine(seed, vertex.is_max_sp_vertex);

        return seed;
    }
};

using OptimiserGraph = PathGraph<PathGraphVertex, std::vector<ProtoActivation>>;

// The class that stores extra information needed on top of a ProcessedSong
// for the purposes of optimisation, and finds the optimal path. The song
// passed to Optimiser's constructor must outlive Optimiser; the class is
// done this way so that other code can make use of the PointIters that are
// returned by Optimiser without needing access to Optimiser itself.
class Optimiser {
private:
    static constexpr double NEG_INF = -std::numeric_limits<double>::infinity();
    static constexpr double BASE_DRUM_FILL_DELAY = 2.0 * 100;
    const ProcessedSong* m_song;
    const std::atomic<bool>* m_terminate;
    SightRead::Second m_drum_fill_delay;
    SightRead::Second m_whammy_delay;
    std::vector<PointPtr> m_next_candidate_points;

    // These methods are involved in constructing the OptimiserGraph.
    [[nodiscard]] OptimiserGraph path_graph(PathGraphVertex root_vertex) const;
    [[nodiscard]] PointPtr next_candidate_point(PointPtr point) const;
    [[nodiscard]] PathGraphVertex
    advance_graph_vertex(PathGraphVertex vertex) const;
    [[nodiscard]] PathGraphVertex
    add_whammy_delay(PathGraphVertex vertex) const;
    [[nodiscard]] SightRead::Second
    earliest_fill_appearance(PathGraphVertex vertex) const;
    OutEdgeAggregate<PathGraphVertex, ProtoActivation>
    out_edges(OptimiserGraph& graph, std::size_t vertex) const;
    void add_acts_from_starting_point(
        PointPtr starting_point, SpPosition starting_pos, SpBar sp_bar,
        ActivationEndSet<PointPtr>& attained_act_ends,
        OutEdgeAggregate<PathGraphVertex, ProtoActivation>& optimal_out_edges)
        const;

    // These methods are involved in extracting an optimal path from the
    // OptimiserGraph.
    [[nodiscard]] Path
    optimal_path_from_graph(const OptimiserGraph& graph) const;
    [[nodiscard]] double act_squeeze_level(ProtoActivation act,
                                           PathGraphVertex vertex) const;
    [[nodiscard]] SpPosition forced_whammy_end(ProtoActivation act,
                                               PathGraphVertex vertex,
                                               double sqz_level) const;
    [[nodiscard]] std::tuple<SightRead::Beat, SightRead::Beat>
    act_duration(ProtoActivation act, PathGraphVertex vertex, double sqz_level,
                 SpPosition min_whammy_force) const;
    [[nodiscard]] bool act_contains_sp_phrase(PointPtr start,
                                              PointPtr end) const;

public:
    Optimiser(const ProcessedSong* song, const std::atomic<bool>* terminate,
              int speed, SightRead::Second whammy_delay);
    // Return the optimal Star Power path.
    [[nodiscard]] Path optimal_path() const;
};

#endif
