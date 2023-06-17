/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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

#ifndef CHOPT_SPTIMEMAP_HPP
#define CHOPT_SPTIMEMAP_HPP

#include <utility>

#include "tempomap.hpp"

class SpMeasure {
private:
    double m_value;

public:
    explicit SpMeasure(double value)
        : m_value {value}
    {
    }
    [[nodiscard]] double value() const { return m_value; }
    [[nodiscard]] Beat to_beat(double beat_rate) const
    {
        return Beat(m_value * beat_rate);
    }

    std::partial_ordering operator<=>(const SpMeasure& rhs) const
    {
        return m_value <=> rhs.m_value;
    }

    SpMeasure& operator+=(const SpMeasure& rhs)
    {
        m_value += rhs.m_value;
        return *this;
    }

    SpMeasure& operator-=(const SpMeasure& rhs)
    {
        m_value -= rhs.m_value;
        return *this;
    }

    SpMeasure& operator*=(double rhs)
    {
        m_value *= rhs;
        return *this;
    }

    friend SpMeasure operator+(SpMeasure lhs, const SpMeasure& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend SpMeasure operator-(SpMeasure lhs, const SpMeasure& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend SpMeasure operator*(SpMeasure lhs, double rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    friend double operator/(const SpMeasure& lhs, const SpMeasure& rhs)
    {
        return lhs.m_value / rhs.m_value;
    }
};

struct SpPosition {
    Beat beat;
    SpMeasure sp_measure;
};

enum class SpMode { Measure, OdBeat };

class SpTimeMap {
private:
    TempoMap m_tempo_map;
    SpMode m_sp_mode;

public:
    SpTimeMap(TempoMap tempo_map, SpMode sp_mode)
        : m_tempo_map {std::move(tempo_map)}
        , m_sp_mode {sp_mode}
    {
    }

    [[nodiscard]] Beat to_beats(Second seconds) const;
    [[nodiscard]] Beat to_beats(SpMeasure sp_measures) const;
    [[nodiscard]] Beat to_beats(Tick ticks) const;

    [[nodiscard]] Second to_seconds(Beat beats) const;
    [[nodiscard]] Second to_seconds(SpMeasure sp_measures) const;

    [[nodiscard]] SpMeasure to_sp_measures(Beat beats) const;
    [[nodiscard]] SpMeasure to_sp_measures(Second seconds) const;
};

#endif