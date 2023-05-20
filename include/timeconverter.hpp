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

#include <vector>

#include "engine.hpp"
#include "tempomap.hpp"
#include "time.hpp"

class TimeConverter {
private:
    struct MeasureTimestamp {
        Measure measure;
        Beat beat;
    };

    static constexpr double DEFAULT_BEAT_RATE = 4.0;
    TempoMap m_tempo_map;
    std::vector<MeasureTimestamp> m_measure_timestamps;
    double m_last_beat_rate;

public:
    TimeConverter(const TempoMap& tempo_map, const Engine& engine,
                  const std::vector<Tick>& od_beats);
    [[nodiscard]] Second measures_to_seconds(Measure measures) const;
    [[nodiscard]] Measure seconds_to_measures(Second seconds) const;
};

#endif
