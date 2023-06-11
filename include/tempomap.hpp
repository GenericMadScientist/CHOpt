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

#ifndef CHOPT_TEMPOMAP_HPP
#define CHOPT_TEMPOMAP_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "time.hpp"

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const char* what)
        : std::runtime_error {what}
    {
    }
};

struct TimeSignature {
    Tick position;
    int numerator;
    int denominator;
};

struct BPM {
    Tick position;
    // Larger int type is needed to handle speedups.
    std::int64_t bpm;
};

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

// Invariants:
// bpms() are sorted by position.
// bpms() never has two BPMs with the same position.
// bpms() is never empty.
// time_sigs() are sorted by position.
// time_sigs() never has two TimeSignatures with the same position.
// time_sigs() is never empty.
class TempoMap {
private:
    struct BeatTimestamp {
        Beat beat;
        Second time;
    };

    struct MeasureTimestamp {
        SpMeasure measure;
        Beat beat;
    };

    static constexpr double DEFAULT_BEAT_RATE = 4.0;
    static constexpr std::int64_t DEFAULT_BPM = 120000;
    static constexpr int DEFAULT_RESOLUTION = 192;

    std::vector<TimeSignature> m_time_sigs;
    std::vector<BPM> m_bpms;
    std::vector<Tick> m_od_beats;
    int m_resolution;

    std::vector<BeatTimestamp> m_beat_timestamps;
    std::int64_t m_last_bpm;

    std::vector<MeasureTimestamp> m_measure_timestamps;
    double m_last_beat_rate;

    std::vector<MeasureTimestamp> m_od_beat_timestamps;
    double m_last_od_beat_rate;

    bool m_use_od_beats = false;

    const std::vector<TempoMap::MeasureTimestamp>& measure_timestamps() const;
    double last_beat_rate() const;

public:
    TempoMap()
        : TempoMap({}, {}, {}, DEFAULT_RESOLUTION)
    {
    }
    TempoMap(std::vector<TimeSignature> time_sigs, std::vector<BPM> bpms,
             std::vector<Tick> od_beats, int resolution);
    [[nodiscard]] const std::vector<TimeSignature>& time_sigs() const
    {
        return m_time_sigs;
    }
    [[nodiscard]] const std::vector<BPM>& bpms() const { return m_bpms; }

    // Return the TempoMap for a speedup of speed% (normal speed is 100).
    [[nodiscard]] TempoMap speedup(int speed) const;

    [[nodiscard]] Beat to_beats(SpMeasure measures) const;
    [[nodiscard]] Beat to_beats(Second seconds) const;
    [[nodiscard]] Beat to_beats(Tick ticks) const
    {
        return Beat {ticks.value() / static_cast<double>(m_resolution)};
    }

    [[nodiscard]] SpMeasure to_sp_measures(Beat beats) const;
    [[nodiscard]] SpMeasure to_sp_measures(Second seconds) const;

    [[nodiscard]] Second to_seconds(Beat beats) const;
    [[nodiscard]] Second to_seconds(SpMeasure measures) const;
    [[nodiscard]] Second to_seconds(Tick ticks) const;

    [[nodiscard]] Tick to_ticks(Beat beats) const
    {
        return Tick {static_cast<int>(beats.value() * m_resolution)};
    }
    [[nodiscard]] Tick to_ticks(Second seconds) const;

    void use_od_beats(bool value) { m_use_od_beats = value; }
};

#endif
