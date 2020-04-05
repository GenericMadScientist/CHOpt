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

#include <algorithm>
#include <cstdint>
#include <map>
#include <tuple>
#include <vector>

#include "chart.hpp"
#include "time.hpp"

struct Point {
    Beat beat_position;
    uint32_t value;
    bool is_hold_point;
    bool is_sp_granting_note;

    friend bool operator==(const Point& lhs, const Point& rhs)
    {
        return std::tie(lhs.beat_position, lhs.value, lhs.is_hold_point,
                        lhs.is_sp_granting_note)
            == std::tie(rhs.beat_position, rhs.value, rhs.is_hold_point,
                        rhs.is_sp_granting_note);
    }
};

using PointPtr = std::vector<Point>::const_iterator;

// Represents the minimum and maximum SP possible at a given time.
class SpBar {
private:
    double m_min;
    double m_max;

public:
    SpBar(double min, double max)
        : m_min {min}
        , m_max {max}
    {
    }

    double& min() { return m_min; }
    double& max() { return m_max; }
    [[nodiscard]] double min() const { return m_min; }
    [[nodiscard]] double max() const { return m_max; }

    friend bool operator==(const SpBar& lhs, const SpBar& rhs)
    {
        return std::tie(lhs.m_min, lhs.m_max) == std::tie(rhs.m_min, rhs.m_max);
    }

    void add_phrase()
    {
        constexpr double SP_PHRASE_AMOUNT = 0.25;

        m_min += SP_PHRASE_AMOUNT;
        m_max += SP_PHRASE_AMOUNT;
        m_min = std::min(m_min, 1.0);
        m_max = std::min(m_max, 1.0);
    }

    [[nodiscard]] bool full_enough_to_activate() const
    {
        constexpr double MINIMUM_SP_AMOUNT = 0.5;
        return m_max >= MINIMUM_SP_AMOUNT;
    }
};

struct ActivationCandidate {
    PointPtr act_start;
    PointPtr act_end;
    Beat earliest_activation_point {0.0};
    SpBar sp_bar {0.0, 0.0};
};

struct Activation {
    PointPtr act_start;
    PointPtr act_end;

    friend bool operator==(const Activation& lhs, const Activation& rhs)
    {
        return std::tie(lhs.act_start, lhs.act_end)
            == std::tie(rhs.act_start, rhs.act_end);
    }
};

struct Path {
    std::vector<Activation> activations;
    uint32_t score_boost;
};

// Represents a song processed for Star Power optimisation. The constructor
// should only fail due to OOM; invariants on the song are supposed to be
// upheld by the constructors of the arguments.
class ProcessedTrack {
private:
    struct BeatRate {
        Beat position;
        double net_sp_gain_rate;
    };

    struct WhammyRange {
        Beat start_beat;
        Beat end_beat;
        Measure start_meas;
        Measure end_meas;
    };

    static constexpr double SP_GAIN_RATE = 1 / 30.0;

    std::vector<Point> m_points;
    TimeConverter m_converter;
    std::vector<BeatRate> m_beat_rates;
    std::vector<WhammyRange> m_whammy_ranges;
    std::vector<Measure> m_point_measures;

    [[nodiscard]] double
    propagate_over_whammy_range(Beat start, Beat end,
                                double sp_bar_amount) const;
    [[nodiscard]] PointPtr furthest_reachable_point(PointPtr point,
                                                    double sp) const;
    [[nodiscard]] bool is_in_whammy_ranges(Beat beat) const;
    [[nodiscard]] PointPtr next_candidate_point(PointPtr point) const;
    Path get_partial_path(PointPtr point,
                          std::map<PointPtr, Path>& partial_paths) const;
    void
    add_point_to_partial_acts(PointPtr point,
                              std::map<PointPtr, Path>& partial_paths) const;

    static std::vector<BeatRate> form_beat_rates(int32_t resolution,
                                                 const SyncTrack& sync_track);

public:
    ProcessedTrack(const NoteTrack& track, int32_t resolution,
                   const SyncTrack& sync_track);
    [[nodiscard]] const std::vector<Point>& points() const { return m_points; }
    [[nodiscard]] bool
    is_candidate_valid(const ActivationCandidate& activation) const;
    // Return how much SP is available at the end after propagating over a
    // range, or -1 if SP runs out at any point. Only includes SP gain from
    // whammy.
    [[nodiscard]] SpBar propagate_sp_over_whammy(Beat start, Beat end,
                                                 Measure start_meas,
                                                 Measure end_meas,
                                                 SpBar sp_bar) const;
    // Return the minimum and maximum amount of SP can be acquired between two
    // points. Does not include SP from the point act_start.
    [[nodiscard]] SpBar total_available_sp(Beat start,
                                           PointPtr act_start) const;
    // Return the optimal Star Power path.
    [[nodiscard]] Path optimal_path() const;
};

#endif
