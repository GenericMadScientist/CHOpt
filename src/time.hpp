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

#include <cstdint>
#include <vector>

#include "chart.hpp"

// We really do want to be able to compare Beats and Measures for equality in
// tests.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif

class Measure;

class Beat {
private:
    double m_value;

public:
    explicit Beat(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Measure to_measure(double beat_rate) const;

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

    Beat& operator*=(double rhs)
    {
        m_value *= rhs;
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

    friend Beat operator*(Beat lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const Beat& lhs, const Beat& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }
};

class Measure {
private:
    double m_value;

public:
    explicit Measure(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Beat to_beat(double beat_rate) const
    {
        return Beat(m_value * beat_rate);
    }

    friend bool operator<(const Measure& lhs, const Measure& rhs)
    {
        return lhs.m_value < rhs.m_value;
    }
    friend bool operator>(const Measure& lhs, const Measure& rhs)
    {
        return rhs < lhs;
    }
    friend bool operator<=(const Measure& lhs, const Measure& rhs)
    {
        return !(lhs > rhs);
    }
    friend bool operator>=(const Measure& lhs, const Measure& rhs)
    {
        return !(lhs < rhs);
    }
    friend bool operator==(const Measure& lhs, const Measure& rhs)
    {
        return lhs.m_value == rhs.m_value;
    }
    friend bool operator!=(const Measure& lhs, const Measure& rhs)
    {
        return !(lhs == rhs);
    }

    Measure& operator+=(const Measure& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    Measure& operator-=(const Measure& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    Measure& operator*=(double rhs)
    {
        m_value *= rhs;
        return *this;
    }

    friend Measure operator+(Measure lhs, const Measure& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Measure operator-(Measure lhs, const Measure& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend Measure operator*(Measure lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const Measure& lhs, const Measure& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }
};

inline Measure Beat::to_measure(double beat_rate) const
{
    return Measure(m_value / beat_rate);
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

struct Position {
    Beat beat;
    Measure measure;
};

class TimeConverter {
private:
    struct MeasureTimestamp {
        Measure measure;
        Beat beat;
    };

    static constexpr double DEFAULT_BEAT_RATE = 4.0;
    std::vector<MeasureTimestamp> m_measure_timestamps;
    double m_last_beat_rate;

public:
    TimeConverter(const SyncTrack& sync_track, std::int32_t resolution);
    [[nodiscard]] Measure beats_to_measures(Beat beats) const;
    [[nodiscard]] Beat measures_to_beats(Measure measures) const;
};

#endif
