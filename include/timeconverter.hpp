/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023 Raymond Wright
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

#ifndef CHOPT_TIMECONVERTER_HPP
#define CHOPT_TIMECONVERTER_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "engine.hpp"
#include "time.hpp"

struct TimeSignature {
    int position;
    int numerator;
    int denominator;
};

struct BPM {
    int position;
    // Larger int type is needed to handle speedups.
    std::int64_t bpm;
};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const char* what)
        : std::runtime_error {what}
    {
    }
};

// Invariants:
// bpms() are sorted by position.
// bpms() never has two BPMs with the same position.
// bpms() is never empty.
// time_sigs() are sorted by position.
// time_sigs() never has two TimeSignatures with the same position.
// time_sigs() is never empty.
class SyncTrack {
private:
    std::vector<TimeSignature> m_time_sigs;
    std::vector<BPM> m_bpms;

public:
    SyncTrack()
        : SyncTrack({}, {})
    {
    }
    SyncTrack(std::vector<TimeSignature> time_sigs, std::vector<BPM> bpms);
    [[nodiscard]] const std::vector<TimeSignature>& time_sigs() const
    {
        return m_time_sigs;
    }
    [[nodiscard]] const std::vector<BPM>& bpms() const { return m_bpms; }
    // Return the SyncTrack for a speedup of speed% (normal speed is 100).
    [[nodiscard]] SyncTrack speedup(int speed) const;
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

    static constexpr std::int64_t DEFAULT_BPM = 120000;
    static constexpr double DEFAULT_BEAT_RATE = 4.0;
    std::vector<BeatTimestamp> m_beat_timestamps;
    std::vector<MeasureTimestamp> m_measure_timestamps;
    double m_last_beat_rate;
    std::int64_t m_last_bpm;

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
