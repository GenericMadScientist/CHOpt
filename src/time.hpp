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

#ifndef CHOPT_TIME_HPP
#define CHOPT_TIME_HPP

#include <vector>

#include "chart.hpp"

// We really do want to be able to compare Beats for equality in tests.

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"

struct Beat {
private:
    double m_value;

public:
    explicit Beat(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }

    friend bool operator<(const Beat& lhs, const Beat& rhs)
    {
        return lhs.m_value < rhs.m_value;
    }
    friend bool operator>(const Beat& lhs, const Beat& rhs)
    {
        return rhs < lhs;
    }
    friend bool operator<=(const Beat& lhs, const Beat& rhs)
    {
        return !(lhs > rhs);
    }
    friend bool operator>=(const Beat& lhs, const Beat& rhs)
    {
        return !(lhs < rhs);
    }
    friend bool operator==(const Beat& lhs, const Beat& rhs)
    {
        return lhs.m_value == rhs.m_value;
    }
    friend bool operator!=(const Beat& lhs, const Beat& rhs)
    {
        return !(lhs == rhs);
    }

    Beat& operator+=(const Beat& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    Beat& operator-=(const Beat& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    friend Beat operator+(Beat lhs, const Beat& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Beat operator-(Beat lhs, const Beat& rhs)
    {
        lhs -= rhs;
        return lhs;
    }
};

#pragma clang diagnostic pop

struct Measure {
    double value;
};

struct Second {
    double value;
};

class TimeConverter {
private:
    struct BeatTimestamp {
        Beat beat;
        Second time;
    };

    struct MeasureTimestamp {
        Measure measure;
        Beat beat;
    };

    static constexpr double MS_PER_MINUTE = 60000.0;
    static constexpr uint32_t DEFAULT_BPM = 120000;
    static constexpr double DEFAULT_BEAT_RATE = 4.0;
    std::vector<BeatTimestamp> m_beat_timestamps;
    std::vector<MeasureTimestamp> m_measure_timestamps;
    double m_last_beat_rate;
    uint32_t m_last_bpm;

public:
    TimeConverter(const SyncTrack& sync_track, const SongHeader& header);
    [[nodiscard]] Second beats_to_seconds(Beat beats) const;
    [[nodiscard]] Beat seconds_to_beats(Second seconds) const;
    [[nodiscard]] Measure beats_to_measures(Beat beats) const;
    [[nodiscard]] Beat measures_to_beats(Measure measures) const;
    [[nodiscard]] Second measures_to_seconds(Measure measures) const;
    [[nodiscard]] Measure seconds_to_measures(Second seconds) const;
};

#endif
