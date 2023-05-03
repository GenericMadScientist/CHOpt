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

// Invariants:
// bpms() are sorted by position.
// bpms() never has two BPMs with the same position.
// bpms() is never empty.
// time_sigs() are sorted by position.
// time_sigs() never has two TimeSignatures with the same position.
// time_sigs() is never empty.
class TempoMap {
private:
    static constexpr int DEFAULT_RESOLUTION = 192;

    std::vector<TimeSignature> m_time_sigs;
    std::vector<BPM> m_bpms;
    int m_resolution;

public:
    TempoMap()
        : TempoMap({}, {}, DEFAULT_RESOLUTION)
    {
    }
    TempoMap(std::vector<TimeSignature> time_sigs, std::vector<BPM> bpms,
             int resolution);
    [[nodiscard]] const std::vector<TimeSignature>& time_sigs() const
    {
        return m_time_sigs;
    }
    [[nodiscard]] const std::vector<BPM>& bpms() const { return m_bpms; }

    // Return the TempoMap for a speedup of speed% (normal speed is 100).
    [[nodiscard]] TempoMap speedup(int speed) const;

    [[nodiscard]] Beat to_beat(Tick tick) const
    {
        return Beat {tick.value() / static_cast<double>(m_resolution)};
    }
    [[nodiscard]] Tick to_tick(Beat beat) const
    {
        return Tick {static_cast<int>(beat.value() * m_resolution)};
    }
};

#endif
