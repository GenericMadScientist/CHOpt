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

#ifndef CHOPT_CHARTPARSER_HPP
#define CHOPT_CHARTPARSER_HPP

#include <set>
#include <string>
#include <string_view>

#include "chart.hpp"
#include "ini.hpp"
#include "song.hpp"
#include "songparts.hpp"

enum class HopoThresholdType { Resolution, HopoFrequency, EighthNote };

struct HopoThreshold {
    HopoThresholdType threshold_type;
    Tick hopo_frequency;

    Tick max_hopo_gap(int resolution) const
    {
        switch (threshold_type) {
        case HopoThresholdType::HopoFrequency:
            return hopo_frequency;
        case HopoThresholdType::EighthNote:
            return Tick {(resolution + 3) / 2};
        default:
            return Tick {(65 * resolution) / 192};
        }
    }
};

class ChartParser {
private:
    std::string m_song_name;
    std::string m_artist;
    std::string m_charter;
    HopoThreshold m_hopo_threshold;
    std::set<Instrument> m_permitted_instruments;
    bool m_permit_solos;

    Song from_chart(const Chart& chart) const;

public:
    explicit ChartParser(const IniValues& ini);
    ChartParser& hopo_threshold(HopoThreshold hopo_threshold);
    ChartParser& permit_instruments(std::set<Instrument> permitted_instruments);
    ChartParser& parse_solos(bool permit_solos);
    Song parse(std::string_view data) const;
};

#endif
