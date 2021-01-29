/*
 * CHOpt - Star Power optimiser for Clone Hero
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

#include "engine.hpp"
#include "songparts.hpp"

class Measure;
class Second;

class Beat {
private:
    double m_value;

public:
    explicit Beat(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Second to_second(int bpm) const;
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

class Second {
private:
    double m_value;

public:
    explicit Second(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Beat to_beat(int bpm) const
    {
        constexpr double MS_PER_MINUTE = 60000.0;
        return Beat(m_value * bpm / MS_PER_MINUTE);
    }

    friend bool operator<(const Second& lhs, const Second& rhs)
    {
        return lhs.m_value < rhs.m_value;
    }
    friend bool operator>(const Second& lhs, const Second& rhs)
    {
        return rhs < lhs;
    }
    friend bool operator<=(const Second& lhs, const Second& rhs)
    {
        return !(lhs > rhs);
    }
    friend bool operator>=(const Second& lhs, const Second& rhs)
    {
        return !(lhs < rhs);
    }

    Second& operator+=(const Second& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    Second& operator-=(const Second& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    Second& operator*=(double rhs)
    {
        m_value *= rhs;
        return *this;
    }

    friend Second operator+(Second lhs, const Second& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Second operator-(Second lhs, const Second& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend Second operator*(Second lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const Second& lhs, const Second& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }
};

inline Measure Beat::to_measure(double beat_rate) const
{
    return Measure(m_value / beat_rate);
}

inline Second Beat::to_second(int bpm) const
{
    constexpr double MS_PER_MINUTE = 60000.0;
    return Second(m_value * MS_PER_MINUTE / bpm);
}

struct Position {
    Beat beat;
    Measure measure;
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

    static constexpr int DEFAULT_BPM = 120000;
    static constexpr double DEFAULT_BEAT_RATE = 4.0;
    std::vector<BeatTimestamp> m_beat_timestamps;
    std::vector<MeasureTimestamp> m_measure_timestamps;
    double m_last_beat_rate;
    int m_last_bpm;

public:
    TimeConverter(const SyncTrack& sync_track, int resolution,
                  const Engine& engine, const std::vector<int>& od_beats);
    [[nodiscard]] Second beats_to_seconds(Beat beats) const;
    [[nodiscard]] Beat seconds_to_beats(Second seconds) const;
    [[nodiscard]] Measure beats_to_measures(Beat beats) const;
    [[nodiscard]] Beat measures_to_beats(Measure measures) const;
    [[nodiscard]] Second measures_to_seconds(Measure measures) const;
    [[nodiscard]] Measure seconds_to_measures(Second seconds) const;
};

#endif
