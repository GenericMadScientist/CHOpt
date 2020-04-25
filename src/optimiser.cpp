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
#include <cstddef>
#include <iterator>
#include <limits>
#include <numeric>
#include <set>

#include "optimiser.hpp"

Optimiser::Optimiser(const ProcessedSong* song)
    : m_song {song}
{
    const auto& points = m_song->points();
    const auto& sp_data = m_song->sp_data();

    auto capacity = std::distance(points.cbegin(), points.cend()) + 1;
    m_next_candidate_points.reserve(static_cast<std::size_t>(capacity));
    int count = 0;
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
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

PointPtr Optimiser::next_candidate_point(PointPtr point) const
{
    auto index = std::distance(m_song->points().cbegin(), point);
    return m_next_candidate_points[static_cast<std::size_t>(index)];
}

Optimiser::CacheKey Optimiser::advance_cache_key(CacheKey key) const
{
    key.point = next_candidate_point(key.point);
    if (key.point == m_song->points().cend()) {
        return key;
    }
    const auto pos = key.point->hit_window_start;
    if (pos.beat >= key.position.beat) {
        key.position = pos;
    }
    return key;
}

// This function merely returns a point such that all activations starting at
// pos, with earliest point point, with the given SP must end at the returned
// point or later. This is merely a bound to help the core algorithm, it need
// not be optimal.
PointPtr Optimiser::act_end_lower_bound(PointPtr point, Measure pos,
                                        double sp_bar_amount) const
{
    constexpr double MEASURES_PER_BAR = 8.0;

    auto end_pos = pos + Measure(MEASURES_PER_BAR * sp_bar_amount);
    auto q = std::find_if(point, m_song->points().cend(), [=](const auto& pt) {
        return pt.hit_window_end.measure > end_pos;
    });
    return std::prev(q);
}

Path Optimiser::get_partial_path(CacheKey key, Cache& cache) const
{
    if (key.point == m_song->points().cend()) {
        return {{}, 0};
    }
    if (cache.paths.find(key) == cache.paths.end()) {
        auto best_path = find_best_subpaths(key, cache, false);
        cache.paths.emplace(key, best_path);
        return best_path.path;
    }
    return cache.paths.at(key).path;
}

Path Optimiser::get_partial_full_sp_path(PointPtr point, Cache& cache) const
{
    if (cache.full_sp_paths.find(point) != cache.full_sp_paths.end()) {
        return cache.full_sp_paths.at(point).path;
    }

    // We only call this from find_best_subpath in a situaiton where we know
    // point is not m_points.cend(), so we may assume point is a real Point.
    CacheKey key {point, std::prev(point)->hit_window_start};
    auto best_path = find_best_subpaths(key, cache, true);
    cache.full_sp_paths.emplace(point, best_path);
    return best_path.path;
}

// This function is an optimisation for the case where key.point is a tick in
// the middle of an SP granting sustain. It is often the case that adjacent
// ticks have the same optimal subpath, and at any rate the optimal subpath
// can't be better than the optimal subpath for the previous point, so we try it
// first. If it works, we return the result, else we return an empty optional.
std::optional<Optimiser::CacheValue>
Optimiser::try_previous_best_subpaths(CacheKey key, const Cache& cache,
                                      bool has_full_sp) const
{
    if (has_full_sp || !key.point->is_hold_point
        || !std::prev(key.point)->is_hold_point) {
        return {};
    }

    auto prev_key_iter = cache.paths.lower_bound(key);
    if (prev_key_iter == cache.paths.begin()) {
        return {};
    }

    prev_key_iter = std::prev(prev_key_iter);
    if (std::distance(prev_key_iter->first.point, key.point) > 1) {
        return {};
    }

    const auto& acts = prev_key_iter->second.possible_next_acts;
    std::vector<std::tuple<Activation, CacheKey>> next_acts;
    for (const auto& act : acts) {
        auto [p, q] = std::get<0>(act);
        auto sp_bar
            = m_song->total_available_sp(key.position.beat, key.point, p);
        auto starting_pos = std::prev(p)->hit_window_start;
        ActivationCandidate candidate {p, q, starting_pos, sp_bar};
        auto candidate_result = m_song->is_candidate_valid(candidate);
        if (candidate_result.validity == ActValidity::success
            && candidate_result.ending_position.beat
                <= std::get<1>(act).position.beat) {
            next_acts.push_back(act);
        }
    }
    if (next_acts.empty()) {
        return {};
    }

    auto score_boost = prev_key_iter->second.path.score_boost;
    auto next_key = std::get<1>(next_acts[0]);
    Path rest_of_path {{}, 0};
    if (next_key.point != m_song->points().cend()) {
        rest_of_path = cache.paths.at(next_key).path;
    }
    auto act_set = rest_of_path.activations;
    act_set.insert(act_set.begin(), std::get<0>(next_acts[0]));
    Path best_path {act_set, score_boost};
    return {{best_path, next_acts}};
}

Optimiser::CacheValue Optimiser::find_best_subpaths(CacheKey key, Cache& cache,
                                                    bool has_full_sp) const
{
    constexpr double MINIMUM_SP_AMOUNT = 0.5;

    auto subpath_from_prev
        = try_previous_best_subpaths(key, cache, has_full_sp);
    if (subpath_from_prev) {
        return *subpath_from_prev;
    }

    std::vector<std::tuple<Activation, CacheKey>> acts;
    std::set<PointPtr> attained_act_ends;
    auto best_score_boost = 0;
    Path best_path {{}, 0};

    for (auto p = key.point; p < m_song->points().cend(); ++p) {
        SpBar sp_bar {1.0, 1.0};
        if (!has_full_sp) {
            sp_bar
                = m_song->total_available_sp(key.position.beat, key.point, p);
        }
        if (!sp_bar.full_enough_to_activate()) {
            continue;
        }
        if (p != key.point && sp_bar.max() == 1.0
            && std::prev(p)->is_sp_granting_note) {
            get_partial_full_sp_path(p, cache);
            auto cache_value = cache.full_sp_paths.at(p);
            if (cache_value.path.score_boost > best_score_boost) {
                return cache_value;
            }
            if (cache_value.path.score_boost == best_score_boost) {
                const auto& next_acts = cache_value.possible_next_acts;
                acts.insert(acts.end(), next_acts.cbegin(), next_acts.cend());
            }
            break;
        }
        auto starting_pos = std::prev(p)->hit_window_start;
        auto q_min = act_end_lower_bound(
            p, starting_pos.measure, std::max(MINIMUM_SP_AMOUNT, sp_bar.min()));
        for (auto q = q_min; q < m_song->points().cend(); ++q) {
            if (attained_act_ends.find(q) != attained_act_ends.end()) {
                continue;
            }

            ActivationCandidate candidate {p, q, starting_pos, sp_bar};
            auto candidate_result = m_song->is_candidate_valid(candidate);
            if (candidate_result.validity != ActValidity::insufficient_sp) {
                attained_act_ends.insert(q);
            } else if (!q->is_hold_point) {
                // We cannot hit any later points if q is not a hold point, so
                // we are done.
                break;
            }

            if (candidate_result.validity != ActValidity::success) {
                continue;
            }

            auto act_score = std::accumulate(
                p, std::next(q), 0,
                [](const auto& sum, const auto& x) { return sum + x.value; });
            CacheKey next_key {std::next(q), candidate_result.ending_position};
            next_key = advance_cache_key(next_key);
            auto rest_of_path = get_partial_path(next_key, cache);
            auto score = act_score + rest_of_path.score_boost;
            if (score > best_score_boost) {
                best_score_boost = score;
                auto act_set = rest_of_path.activations;
                act_set.insert(act_set.begin(), {p, q});
                best_path = {act_set, score};
                acts.clear();
                acts.push_back({{p, q}, next_key});
            } else if (score == best_score_boost) {
                acts.push_back({{p, q}, next_key});
            }
        }
    }

    return {best_path, acts};
}

Path Optimiser::optimal_path() const
{
    Cache cache;
    auto neg_inf = -std::numeric_limits<double>::infinity();
    CacheKey start_key {m_song->points().cbegin(),
                        {Beat(neg_inf), Measure(neg_inf)}};
    start_key = advance_cache_key(start_key);

    return get_partial_path(start_key, cache);
}
