/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023 Raymond Wright
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
#include <cassert>
#include <limits>
#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include <sightread/time.hpp>

#include "points.hpp"
#include "processed.hpp"

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
        SpPosition position {SightRead::Beat(0.0), SpMeasure(0.0)};

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

    // The idea is this is like a std::set<PointPtr>, but is add-only and takes
    // advantage of the fact that we often tend to add all elements before a
    // certain point.
    class PointPtrRangeSet {
    private:
        PointPtr m_start;
        PointPtr m_end;
        PointPtr m_min_absent_ptr;
        std::vector<PointPtr> m_abnormal_elements;

    public:
        PointPtrRangeSet(PointPtr start, PointPtr end)
            : m_start {start}
            , m_end {end}
            , m_min_absent_ptr {start}
        {
            assert(start < end); // NOLINT
        }

        [[nodiscard]] bool contains(PointPtr element) const
        {
            if (m_start > element || m_end <= element) {
                return false;
            }
            if (element < m_min_absent_ptr) {
                return true;
            }
            return std::find(m_abnormal_elements.cbegin(),
                             m_abnormal_elements.cend(), element)
                != m_abnormal_elements.cend();
        }

        [[nodiscard]] PointPtr lowest_absent_element() const
        {
            return m_min_absent_ptr;
        }

        void add(PointPtr element)
        {
            assert(m_start <= element); // NOLINT
            assert(element < m_end); // NOLINT
            if (m_min_absent_ptr == element) {
                ++m_min_absent_ptr;
                while (true) {
                    auto next_elem_iter = std::find(m_abnormal_elements.begin(),
                                                    m_abnormal_elements.end(),
                                                    m_min_absent_ptr);
                    if (next_elem_iter == m_abnormal_elements.end()) {
                        return;
                    }
                    std::swap(*next_elem_iter, m_abnormal_elements.back());
                    m_abnormal_elements.pop_back();
                    ++m_min_absent_ptr;
                }
            } else {
                m_abnormal_elements.push_back(element);
            }
        }
    };

    static constexpr double NEG_INF = -std::numeric_limits<double>::infinity();
    static constexpr double BASE_DRUM_FILL_DELAY = 2.0 * 100;
    const ProcessedSong* m_song;
    const std::atomic<bool>* m_terminate;
    const SightRead::Second m_drum_fill_delay;
    SightRead::Second m_whammy_delay;
    std::vector<PointPtr> m_next_candidate_points;

    [[nodiscard]] PointPtr next_candidate_point(PointPtr point) const;
    [[nodiscard]] CacheKey advance_cache_key(CacheKey key) const;
    [[nodiscard]] CacheKey add_whammy_delay(CacheKey key) const;
    [[nodiscard]] std::optional<CacheValue>
    try_previous_best_subpaths(CacheKey key, const Cache& cache,
                               bool has_full_sp) const;
    CacheValue find_best_subpaths(CacheKey key, Cache& cache,
                                  bool has_full_sp) const;
    int get_partial_path(CacheKey key, Cache& cache) const;
    int get_partial_full_sp_path(PointPtr point, Cache& cache) const;
    [[nodiscard]] double act_squeeze_level(ProtoActivation act,
                                           CacheKey key) const;
    [[nodiscard]] SpPosition forced_whammy_end(ProtoActivation act,
                                               CacheKey key,
                                               double sqz_level) const;
    [[nodiscard]] std::tuple<SightRead::Beat, SightRead::Beat>
    act_duration(ProtoActivation act, CacheKey key, double sqz_level,
                 SpPosition min_whammy_force) const;
    [[nodiscard]] SightRead::Second
    earliest_fill_appearance(CacheKey key, bool has_full_sp) const;
    void complete_subpath(
        PointPtr p, SpPosition starting_pos, SpBar sp_bar,
        PointPtrRangeSet& attained_act_ends, Cache& cache,
        int& best_score_boost,
        std::vector<std::tuple<ProtoActivation, CacheKey>>& acts) const;

public:
    Optimiser(const ProcessedSong* song, const std::atomic<bool>* terminate,
              int speed, SightRead::Second whammy_delay);
    // Return the optimal Star Power path.
    [[nodiscard]] Path optimal_path() const;
};

#endif
