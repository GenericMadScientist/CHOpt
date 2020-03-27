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

struct BeatTimestamp {
    double beat;
    double time;
};

struct MeasureTimestamp {
    double measure;
    double beat;
};

class TimeConverter {
private:
    static constexpr double MS_PER_MINUTE = 60000.0;
    static constexpr uint32_t DEFAULT_BPM = 120000;
    static constexpr double DEFAULT_BEAT_RATE = 4.0;
    std::vector<BeatTimestamp> m_beat_timestamps;
    std::vector<MeasureTimestamp> m_measure_timestamps;
    double m_last_beat_rate;
    uint32_t m_last_bpm;

public:
    TimeConverter(const SyncTrack& sync_track, const SongHeader& header);
    [[nodiscard]] double beats_to_seconds(double beats) const;
    [[nodiscard]] double seconds_to_beats(double seconds) const;
    [[nodiscard]] double beats_to_measures(double beats) const;
    [[nodiscard]] double measures_to_beats(double measures) const;
    [[nodiscard]] double measures_to_seconds(double measures) const;
    [[nodiscard]] double seconds_to_measures(double seconds) const;
};

struct Point {
    double beat_position;
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

struct ActivationCandidate {
    std::vector<Point>::const_iterator act_start;
    std::vector<Point>::const_iterator act_end;
    std::vector<Point>::const_iterator points_end;
    double earliest_activation_point = 0.0;
    double sp_bar_amount = 0.0;
};

// Splits a collection of notes into the points a player can get. This function
// should only fail if the program cannot allocate the vector for the return
// value: any invariants on the song are supposed to be upheld by the
// constructors of NoteTrack and SongHeader.
std::vector<Point> notes_to_points(const NoteTrack& track,
                                   const SongHeader& header);

// Return the earliest and latest times a point can be hit, in beats.
double front_end(const Point& point, const TimeConverter& converter);
double back_end(const Point& point, const TimeConverter& converter);

// Return if an ActivationCandidate is valid.
bool is_candidate_valid(const ActivationCandidate& activation,
                        const TimeConverter& converter);

#endif
