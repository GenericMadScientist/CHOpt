/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024, 2025 Raymond Wright
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

#ifndef CHOPT_PROCESSED_HPP
#define CHOPT_PROCESSED_HPP

#include <limits>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include <sightread/drumsettings.hpp>
#include <sightread/songparts.hpp>
#include <sightread/tempomap.hpp>
#include <sightread/time.hpp>

#include "engine.hpp"
#include "points.hpp"
#include "settings.hpp"
#include "sp.hpp"
#include "sptimemap.hpp"

struct ActivationCandidate {
    PointPtr act_start;
    PointPtr act_end;
    SpPosition earliest_activation_point {SightRead::Beat(0.0), SpMeasure(0.0)};
    SpBar sp_bar;

    ActivationCandidate(const SpEngineValues& sp_engine_values)
        : sp_bar {0.0, 0.0, sp_engine_values}
    {
    }

    ActivationCandidate(PointPtr act_start, PointPtr act_end,
                        SpPosition earliest_activation_point, SpBar sp_bar)
        : act_start {act_start}
        , act_end {act_end}
        , earliest_activation_point {earliest_activation_point}
        , sp_bar {sp_bar}
    {
    }
};

struct ProtoActivation {
    PointPtr act_start;
    PointPtr act_end;
};

struct Activation {
    PointPtr act_start;
    PointPtr act_end;
    SightRead::Beat whammy_end {0.0};
    SightRead::Beat sp_start {0.0};
    SightRead::Beat sp_end {0.0};
};

// Part of the return value of ProcessedSong::is_candidate_valid. Says if an
// activation is valid, and if not whether the problem is too little or too much
// Star Power.
enum class ActValidity { success, insufficient_sp, surplus_sp };

// Return value of ProcessedSong::is_candidate_valid, providing information on
// whether an activation is valid, and if so the earliest position it can end.
struct ActResult {
    SpPosition ending_position;
    ActValidity validity;
};

struct Path {
    std::vector<Activation> activations;
    int score_boost {0};
};

// Represents a song processed for Star Power optimisation. The constructor
// should only fail due to OOM; invariants on the song are supposed to be
// upheld by the constructors of the arguments.
class ProcessedSong {
private:
    static constexpr double NEG_INF = -std::numeric_limits<double>::infinity();

    SpTimeMap m_time_map;
    PointSet m_points;
    SpData m_sp_data;
    SpEngineValues m_sp_engine_values;
    int m_total_bre_boost;
    int m_total_solo_boost;
    int m_base_score;
    bool m_ignore_average_multiplier;
    bool m_is_drums;
    bool m_overlaps;

    SpBar sp_from_phrases(PointPtr begin, PointPtr end) const;
    std::vector<std::string> act_summaries(const Path& path) const;
    std::vector<std::string> drum_act_summaries(const Path& path) const;
    void append_activation(std::stringstream& stream,
                           const Activation& activation,
                           const std::string& act_summary) const;

    // This static function is necessary to deal with a bug in MSVC. See
    // https://developercommunity.visualstudio.com/t/ICE-with-MSVC-1940-with-default-functio/10750601
    // for details.
    static SpPosition default_position()
    {
        return {SightRead::Beat {NEG_INF}, SpMeasure {NEG_INF}};
    }

public:
    ProcessedSong(const SightRead::NoteTrack& track,
                  const SpDurationData& duration_data,
                  const PathingSettings& pathing_settings);

    // Return the minimum and maximum amount of SP can be acquired between two
    // points. Does not include SP from the point act_start. first_point is
    // given for the purposes of counting SP grantings notes, e.g. if start is
    // after the middle of first_point's timing window. All whammy up to
    // required_whammy_end is mandatory.
    [[nodiscard]] SpBar total_available_sp(SightRead::Beat start,
                                           PointPtr first_point,
                                           PointPtr act_start,
                                           SightRead::Beat required_whammy_end
                                           = SightRead::Beat {NEG_INF}) const;
    // Similar to total_available_sp, but no whammy is required and if it is
    // possible to get a half bar then the earliest position >=
    // earliest_potential_pos that grants a half bar is returned along with the
    // SP only up to that position.
    [[nodiscard]] std::tuple<SpBar, SpPosition>
    total_available_sp_with_earliest_pos(
        SightRead::Beat start, PointPtr first_point, PointPtr act_start,
        SpPosition earliest_potential_pos) const;
    // Returns an ActResult which says if an activation is valid, and if so the
    // earliest position it can end. Checks squeezes against the given amount
    // only.
    [[nodiscard]] ActResult is_candidate_valid(
        const ActivationCandidate& activation, double squeeze = 1.0,
        SpPosition required_whammy_end = default_position()) const;
    // Return the summary of a path.
    [[nodiscard]] std::string path_summary(const Path& path) const;

    // Return the position that is (100 - squeeze)% along the start of point's
    // timing window.
    [[nodiscard]] SpPosition adjusted_hit_window_start(PointPtr point,
                                                       double squeeze) const;
    // Return the position that is squeeze% along the end of point's timing
    // window.
    [[nodiscard]] SpPosition adjusted_hit_window_end(PointPtr point,
                                                     double squeeze) const;

    [[nodiscard]] const PointSet& points() const { return m_points; }
    [[nodiscard]] const SpData& sp_data() const { return m_sp_data; }
    [[nodiscard]] const SpTimeMap& sp_time_map() const { return m_time_map; }
    [[nodiscard]] bool is_drums() const { return m_is_drums; }
    [[nodiscard]] const SpEngineValues& sp_engine_values() const
    {
        return m_sp_engine_values;
    }
};

#endif
