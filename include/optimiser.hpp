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

#ifndef CHOPT_OPTIMISER_HPP
#define CHOPT_OPTIMISER_HPP

#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include "points.hpp"
#include "processed.hpp"
#include "time.hpp"

// The class that stores extra information needed on top of a ProcessedSong for
// the purposes of optimisation, and finds the optimal path. The song passed to
// Optimiser's constructor must outlive Optimiser; the class is done this way so
// that other code can make use of the PointIters that are returned by Optimiser
// without needing access to Optimiser itself.
class Optimiser {
private:
    // The Cache is used to store paths starting from a certain point onwards,
    // i.e., the solution to our subproblems in our dynamic programming
    // algorithm. Cache.full_sp_paths represents the best path with the first
    // activation at the point key or later, under the condition there is
    // already full SP there.
    struct CacheKey {
        PointPtr point;
        Position position {Beat(0.0), Measure(0.0)};

        friend bool operator<(const CacheKey& lhs, const CacheKey& rhs)
        {
            return std::tie(lhs.point, lhs.position.beat)
                < std::tie(rhs.point, rhs.position.beat);
        }
    };

    struct CacheValue {
        std::vector<std::tuple<ProtoActivation, CacheKey>> possible_next_acts;
        int score_boost;
    };

    struct Cache {
        std::map<CacheKey, CacheValue> paths;
        std::map<PointPtr, CacheValue> full_sp_paths;
    };

    const ProcessedSong* m_song;
    std::vector<PointPtr> m_next_candidate_points;

    [[nodiscard]] PointPtr next_candidate_point(PointPtr point) const;
    [[nodiscard]] CacheKey advance_cache_key(CacheKey key) const;
    [[nodiscard]] PointPtr act_end_lower_bound(PointPtr point, Measure pos,
                                               double sp_bar_amount) const;
    [[nodiscard]] std::optional<CacheValue>
    try_previous_best_subpaths(CacheKey key, const Cache& cache,
                               bool has_full_sp) const;
    CacheValue find_best_subpaths(CacheKey key, Cache& cache,
                                  bool has_full_sp) const;
    int get_partial_path(CacheKey key, Cache& cache) const;
    int get_partial_full_sp_path(PointPtr point, Cache& cache) const;
    [[nodiscard]] double act_squeeze_level(ProtoActivation act,
                                           CacheKey key) const;
    [[nodiscard]] Position forced_whammy_end(ProtoActivation act, CacheKey key,
                                             double sqz_level) const;
    [[nodiscard]] std::tuple<Beat, Beat>
    act_duration(ProtoActivation act, CacheKey key, double sqz_level,
                 Position min_whammy_force) const;

public:
    explicit Optimiser(const ProcessedSong* song);
    // Return the optimal Star Power path.
    [[nodiscard]] Path optimal_path() const;
};

#endif
