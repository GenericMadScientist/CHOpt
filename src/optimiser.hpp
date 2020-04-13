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

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "chart.hpp"
#include "points.hpp"
#include "sp.hpp"
#include "time.hpp"

struct ActivationCandidate {
    PointPtr act_start;
    PointPtr act_end;
    Position earliest_activation_point {Beat(0.0), Measure(0.0)};
    SpBar sp_bar {0.0, 0.0};
};

struct Activation {
    PointPtr act_start;
    PointPtr act_end;
};

struct Path {
    std::vector<Activation> activations;
    std::uint32_t score_boost;
};

// Represents a song processed for Star Power optimisation. The constructor
// should only fail due to OOM; invariants on the song are supposed to be
// upheld by the constructors of the arguments.
class ProcessedTrack {
private:
    // The order of these members is important. We must have m_converter before
    // m_points.
    TimeConverter m_converter;
    PointSet m_points;
    SpData m_sp_data;
    std::uint32_t m_total_solo_boost;

    [[nodiscard]] PointPtr furthest_reachable_point(PointPtr point,
                                                    double sp) const;
    [[nodiscard]] PointPtr next_candidate_point(PointPtr point) const;
    Path get_partial_path(PointPtr point,
                          std::map<PointPtr, Path>& partial_paths) const;
    void
    add_point_to_partial_acts(PointPtr point,
                              std::map<PointPtr, Path>& partial_paths) const;

public:
    ProcessedTrack(const NoteTrack& track, std::int32_t resolution,
                   const SyncTrack& sync_track);
    [[nodiscard]] const PointSet& points() const { return m_points; }
    [[nodiscard]] bool
    is_candidate_valid(const ActivationCandidate& activation) const;
    // Return the minimum and maximum amount of SP can be acquired between two
    // points. Does not include SP from the point act_start.
    [[nodiscard]] SpBar total_available_sp(Beat start,
                                           PointPtr act_start) const;
    // Return the optimal Star Power path.
    [[nodiscard]] Path optimal_path() const;
    // Return the summary of a path.
    [[nodiscard]] std::string path_summary(const Path& path) const;
};

#endif
