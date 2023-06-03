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
#include "settings.hpp"
#include "song.hpp"

class ChartParser {
private:
    std::string m_song_name;
    std::string m_artist;
    std::string m_charter;
    std::set<Instrument> m_permitted_instruments;

    Song from_chart(const Chart& chart) const;

public:
    explicit ChartParser(const IniValues& ini);
    ChartParser& permit_instruments(std::set<Instrument> permitted_instruments);
    Song parse(std::string_view data) const;
};

#endif
