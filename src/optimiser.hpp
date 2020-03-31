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
        return std::tie(lhs.beat_position.value, lhs.value, lhs.is_hold_point,
                        lhs.is_sp_granting_note)
            == std::tie(rhs.beat_position.value, rhs.value, rhs.is_hold_point,
                        rhs.is_sp_granting_note);
    }
};

struct ActivationCandidate {
    std::vector<Point>::const_iterator act_start;
    std::vector<Point>::const_iterator act_end;
    Beat earliest_activation_point {0.0};
    double sp_bar_amount {0.0};
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
    };

    std::vector<Point> m_points;
    TimeConverter m_converter;
    std::vector<BeatRate> m_beat_rates;
    std::vector<WhammyRange> m_whammy_ranges;

    [[nodiscard]] double
    propagate_over_whammy_range(Beat start, Beat end,
                                double sp_bar_amount) const;

    static std::vector<BeatRate> form_beat_rates(const SongHeader& header,
                                                 const SyncTrack& sync_track);

public:
    ProcessedTrack(const NoteTrack& track, const SongHeader& header,
                   const SyncTrack& sync_track);
    [[nodiscard]] const std::vector<Point>& points() const { return m_points; }
    [[nodiscard]] bool
    is_candidate_valid(const ActivationCandidate& activation) const;
    // Return how much SP is available at the end after propagating over a
    // range, or -1 if SP runs out at any point. Only includes SP gain from
    // whammy.
    [[nodiscard]] double propagate_sp_over_whammy(Beat start, Beat end,
                                                  double sp_bar_amount) const;
};

// Return the earliest and latest times a point can be hit, in beats.
Beat front_end(const Point& point, const TimeConverter& converter);
Beat back_end(const Point& point, const TimeConverter& converter);

#endif
