/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022 Raymond Wright
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

#include <cstdint>
#include <iterator>
#include <stdexcept>

#include "optimiser.hpp"

Optimiser::Optimiser(const ProcessedSong* song,
                     const std::atomic<bool>* terminate, int speed,
                     Second whammy_delay)
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
    const auto index = std::distance(m_song->points().cbegin(), point);
    return m_next_candidate_points[static_cast<std::size_t>(index)];
}

Optimiser::CacheKey Optimiser::advance_cache_key(CacheKey key) const
{
    key.point = next_candidate_point(key.point);
    if (key.point == m_song->points().cend()) {
        return key;
    }
    key = add_whammy_delay(key);
    auto pos = key.point->hit_window_start;
    if (key.point != m_song->points().cbegin()) {
        pos = std::prev(key.point)->hit_window_start;
    }
    if (pos.beat >= key.position.beat) {
        key.position = pos;
    }
    return key;
}

Optimiser::CacheKey Optimiser::add_whammy_delay(CacheKey key) const
{
    auto seconds = m_song->tempo_map().to_seconds(key.position.beat);
    seconds += m_whammy_delay;
    key.position.beat = m_song->tempo_map().to_beats(seconds);
    key.position.measure
        = m_song->converter().beats_to_measures(key.position.beat);
    return key;
}

int Optimiser::get_partial_path(CacheKey key, Cache& cache) const
{
    if (key.point == m_song->points().cend()) {
        return 0;
    }
    if (cache.paths.find(key) == cache.paths.end()) {
        if (m_terminate->load()) {
            throw std::runtime_error("Thread halted");
        }
        auto best_path = find_best_subpaths(key, cache, false);
        cache.paths.emplace(key, best_path);
        return best_path.score_boost;
    }
    return cache.paths.at(key).score_boost;
}

int Optimiser::get_partial_full_sp_path(PointPtr point, Cache& cache) const
{
    if (cache.full_sp_paths.find(point) != cache.full_sp_paths.end()) {
        return cache.full_sp_paths.at(point).score_boost;
    }

    // We only call this from find_best_subpath in a situaiton where we know
    // point is not m_points.cend(), so we may assume point is a real Point.
    CacheKey key {point, std::prev(point)->hit_window_start};
    auto best_path = find_best_subpaths(key, cache, true);
    cache.full_sp_paths.emplace(point, best_path);
    return best_path.score_boost;
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
    if (has_full_sp || !key.point->is_hold_point) {
        return std::nullopt;
    }
    if (key.point != m_song->points().cbegin()
        && !std::prev(key.point)->is_hold_point) {
        return std::nullopt;
    }

    auto prev_key_iter = cache.paths.lower_bound(key);
    if (prev_key_iter == cache.paths.begin()) {
        return std::nullopt;
    }

    prev_key_iter = std::prev(prev_key_iter);
    if (std::distance(prev_key_iter->first.point, key.point) > 1) {
        return std::nullopt;
    }

    const auto& acts = prev_key_iter->second.possible_next_acts;
    std::vector<std::tuple<ProtoActivation, CacheKey>> next_acts;
    for (const auto& act : acts) {
        auto [p, q] = std::get<0>(act);
        const auto& [sp_bar, starting_pos]
            = m_song->total_available_sp_with_earliest_pos(
                key.position.beat, key.point, p,
                std::prev(p)->hit_window_start);
        ActivationCandidate candidate {p, q, starting_pos, sp_bar};
        auto candidate_result = m_song->is_candidate_valid(candidate);
        if (candidate_result.validity == ActValidity::success
            && candidate_result.ending_position.beat
                <= std::get<1>(act).position.beat) {
            next_acts.push_back(act);
        }
    }
    if (next_acts.empty()) {
        return std::nullopt;
    }

    const auto score_boost = prev_key_iter->second.score_boost;
    return {{next_acts, score_boost}};
}

// This function takes some information and completes the optimal subpaths from
// it.
void Optimiser::complete_subpath(
    PointPtr p, Position starting_pos, SpBar sp_bar,
    PointPtrRangeSet& attained_act_ends, Cache& cache, int& best_score_boost,
    std::vector<std::tuple<ProtoActivation, CacheKey>>& acts) const
{
    for (auto q = attained_act_ends.lowest_absent_element();
         q < m_song->points().cend();) {
        if (attained_act_ends.contains(q)) {
            ++q;
            continue;
        }

        ActivationCandidate candidate {p, q, starting_pos, sp_bar};
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
            ++q;
            continue;
        }

        const auto act_score = m_song->points().range_score(p, std::next(q));
        CacheKey next_key {m_song->points().first_after_current_phrase(q),
                           candidate_result.ending_position};
        next_key = advance_cache_key(next_key);
        const auto rest_of_path_score_boost = get_partial_path(next_key, cache);
        const auto score = act_score + rest_of_path_score_boost;
        if (score > best_score_boost) {
            best_score_boost = score;
            acts.clear();
            acts.push_back({{p, q}, next_key});
        } else if (score == best_score_boost) {
            acts.push_back({{p, q}, next_key});
        }
        ++q;
    }
}

Second Optimiser::earliest_fill_appearance(CacheKey key, bool has_full_sp) const
{
    if (!m_song->is_drums() || has_full_sp) {
        return Second(0.0);
    }

    int sp_count = 0;
    for (auto p = key.point; p < m_song->points().cend(); ++p) {
        if (p->is_sp_granting_note) {
            ++sp_count;
            if (sp_count == 2) {
                return m_song->tempo_map().to_seconds(p->hit_window_start.beat)
                    + m_drum_fill_delay;
            }
        }
    }

    return Second(0.0);
}

Optimiser::CacheValue Optimiser::find_best_subpaths(CacheKey key, Cache& cache,
                                                    bool has_full_sp) const
{
    const auto subpath_from_prev
        = try_previous_best_subpaths(key, cache, has_full_sp);
    if (subpath_from_prev) {
        return *subpath_from_prev;
    }

    const auto early_act_bound = earliest_fill_appearance(key, has_full_sp);
    std::vector<std::tuple<ProtoActivation, CacheKey>> acts;
    PointPtrRangeSet attained_act_ends {key.point, m_song->points().cend()};
    auto lower_bound_set = false;
    auto best_score_boost = 0;

    for (auto p = key.point; p < m_song->points().cend(); ++p) {
        if (m_song->is_drums()
            && (!p->fill_start.has_value()
                || p->fill_start < early_act_bound)) {
            continue;
        }
        SpBar sp_bar {1.0, 1.0};
        Position starting_pos {Beat {NEG_INF}, Measure {NEG_INF}};
        if (p != m_song->points().cbegin()) {
            starting_pos = std::prev(p)->hit_window_start;
        }
        if (!has_full_sp) {
            const auto& [new_sp, new_pos]
                = m_song->total_available_sp_with_earliest_pos(
                    key.position.beat, key.point, p, starting_pos);
            sp_bar = new_sp;
            starting_pos = new_pos;
        }
        if (m_song->is_drums()) {
            starting_pos.beat
                = std::max(starting_pos.beat, p->hit_window_start.beat);
            starting_pos.measure
                = std::max(starting_pos.measure, p->hit_window_start.measure);
        }
        if (!sp_bar.full_enough_to_activate()) {
            continue;
        }
        if (p != key.point && sp_bar.min() == 1.0
            && std::prev(p)->is_sp_granting_note) {
            get_partial_full_sp_path(p, cache);
            auto cache_value = cache.full_sp_paths.at(p);
            if (cache_value.score_boost > best_score_boost) {
                return cache_value;
            }
            if (cache_value.score_boost == best_score_boost) {
                const auto& next_acts = cache_value.possible_next_acts;
                acts.insert(acts.end(), next_acts.cbegin(), next_acts.cend());
            }
            break;
        }
        // This skips some points that are too early to be an act end for the
        // earliest possible activation.
        if (!lower_bound_set) {
            const Measure act_length {8.0 * std::max(sp_bar.min(), 0.5)};
            const auto earliest_act_end = starting_pos.measure + act_length;
            auto earliest_pt_end = std::find_if_not(
                std::next(p), m_song->points().cend(), [&](const auto& pt) {
                    return pt.hit_window_end.measure <= earliest_act_end;
                });
            --earliest_pt_end;
            attained_act_ends
                = PointPtrRangeSet {earliest_pt_end, m_song->points().cend()};
            lower_bound_set = true;
        }
        complete_subpath(p, starting_pos, sp_bar, attained_act_ends, cache,
                         best_score_boost, acts);
    }

    return {acts, best_score_boost};
}

Path Optimiser::optimal_path() const
{
    Cache cache;
    CacheKey start_key {m_song->points().cbegin(),
                        {Beat(NEG_INF), Measure(NEG_INF)}};
    start_key = advance_cache_key(start_key);

    const auto best_score_boost = get_partial_path(start_key, cache);
    Path path {{}, best_score_boost};

    while (start_key.point != m_song->points().cend()) {
        const auto& acts = cache.paths.at(start_key).possible_next_acts;
        // We can get here if the song ends in say ES1.
        if (acts.empty()) {
            break;
        }
        auto best_proto_act = std::get<0>(acts[0]);
        auto best_next_key = std::get<1>(acts[0]);
        auto best_sqz_level = act_squeeze_level(best_proto_act, start_key);
        for (auto i = 1U; i < acts.size(); ++i) {
            const auto [proto_act, next_key] = acts[i];
            const auto sqz_level = act_squeeze_level(proto_act, start_key);
            if (sqz_level < best_sqz_level) {
                best_proto_act = std::get<0>(acts[i]);
                best_next_key = std::get<1>(acts[i]);
                best_sqz_level = sqz_level;
            }
        }
        const auto min_whammy_force
            = forced_whammy_end(best_proto_act, start_key, best_sqz_level);
        const auto [start_pos, end_pos] = act_duration(
            best_proto_act, start_key, best_sqz_level, min_whammy_force);
        Activation act {best_proto_act.act_start, best_proto_act.act_end,
                        min_whammy_force.beat, start_pos, end_pos};
        path.activations.push_back(act);
        start_key = best_next_key;
    }

    return path;
}

double Optimiser::act_squeeze_level(ProtoActivation act, CacheKey key) const
{
    constexpr double THRESHOLD = 0.01;

    auto min_sqz = 0.0;
    auto max_sqz = 1.0;
    // Determines what point controls how early we can go: the previous point on
    // guitar and the current point on drums.
    const auto start_bound_point
        = m_song->is_drums() ? act.act_start : std::prev(act.act_start);
    while (max_sqz - min_sqz > THRESHOLD) {
        auto trial_sqz = (min_sqz + max_sqz) / 2;
        auto start_pos
            = m_song->adjusted_hit_window_start(start_bound_point, trial_sqz);
        if (start_pos.beat < key.position.beat) {
            start_pos = key.position;
        }

        const auto& [sp_bar, new_pos]
            = m_song->total_available_sp_with_earliest_pos(
                key.position.beat, key.point, act.act_start, start_pos);
        start_pos = new_pos;

        ActivationCandidate candidate {act.act_start, act.act_end, start_pos,
                                       sp_bar};
        if (m_song->is_candidate_valid(candidate, trial_sqz).validity
            == ActValidity::success) {
            max_sqz = trial_sqz;
        } else {
            min_sqz = trial_sqz;
        }
    }
    return max_sqz;
}

Position Optimiser::forced_whammy_end(ProtoActivation act, CacheKey key,
                                      double sqz_level) const
{
    constexpr double POS_INF = std::numeric_limits<double>::infinity();
    constexpr double THRESHOLD = 0.01;

    auto next_point = std::next(act.act_end);

    if (next_point == m_song->points().cend()) {
        return {Beat {POS_INF}, Measure {POS_INF}};
    }

    auto prev_point = std::prev(act.act_start);
    auto min_whammy_force = key.position;
    auto max_whammy_force = next_point->hit_window_end;
    auto start_pos = m_song->adjusted_hit_window_start(prev_point, sqz_level);
    while ((max_whammy_force.beat - min_whammy_force.beat).value()
           > THRESHOLD) {
        auto mid_beat
            = (min_whammy_force.beat + max_whammy_force.beat) * (1.0 / 2);
        auto mid_meas = m_song->converter().beats_to_measures(mid_beat);
        Position mid_pos {mid_beat, mid_meas};
        auto sp_bar = m_song->total_available_sp(key.position.beat, key.point,
                                                 act.act_start, mid_beat);
        ActivationCandidate candidate {act.act_start, act.act_end, start_pos,
                                       sp_bar};
        auto result = m_song->is_candidate_valid(candidate, sqz_level, mid_pos);
        if (result.validity == ActValidity::success) {
            min_whammy_force = mid_pos;
        } else {
            max_whammy_force = mid_pos;
        }
    }

    return min_whammy_force;
}

std::tuple<Beat, Beat> Optimiser::act_duration(ProtoActivation act,
                                               CacheKey key, double sqz_level,
                                               Position min_whammy_force) const
{
    constexpr double THRESHOLD = 0.01;

    // Determines what point controls how early we can go: the previous point on
    // guitar and the current point on drums.
    const auto start_bound_point
        = m_song->is_drums() ? act.act_start : std::prev(act.act_start);
    auto min_pos
        = m_song->adjusted_hit_window_start(start_bound_point, sqz_level);
    auto max_pos = m_song->adjusted_hit_window_end(act.act_start, sqz_level);
    auto sp_bar = m_song->total_available_sp(
        key.position.beat, key.point, act.act_start, min_whammy_force.beat);
    while ((max_pos.beat - min_pos.beat).value() > THRESHOLD) {
        auto trial_beat = (min_pos.beat + max_pos.beat) * (1.0 / 2);
        auto trial_meas = m_song->converter().beats_to_measures(trial_beat);
        Position trial_pos {trial_beat, trial_meas};
        ActivationCandidate candidate {act.act_start, act.act_end, trial_pos,
                                       sp_bar};
        if (m_song->is_candidate_valid(candidate, sqz_level, min_whammy_force)
                .validity
            == ActValidity::success) {
            min_pos = trial_pos;
        } else {
            max_pos = trial_pos;
        }
    }

    ActivationCandidate candidate {act.act_start, act.act_end, min_pos, sp_bar};
    auto result
        = m_song->is_candidate_valid(candidate, sqz_level, min_whammy_force);
    assert(result.validity == ActValidity::success); // NOLINT
    return {min_pos.beat, result.ending_position.beat};
}
