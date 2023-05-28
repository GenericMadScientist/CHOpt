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

#ifndef CHOPT_MIDIPARSER_HPP
#define CHOPT_MIDIPARSER_HPP

#include <string>

#include "ini.hpp"
#include "midi.hpp"
#include "song.hpp"

class MidiParser {
private:
    std::string m_song_name;
    std::string m_artist;
    std::string m_charter;

public:
    explicit MidiParser(const IniValues& ini);
    Song parse(const Midi& midi) const;
};

#endif
