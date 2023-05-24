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

#ifndef CHOPT_SONGFILE_HPP
#define CHOPT_SONGFILE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "ini.hpp"
#include "song.hpp"

class SongFile {
private:
    enum class FileType { Chart, Midi };

    std::vector<std::uint8_t> m_loaded_file;
    IniValues m_ini_values;
    FileType m_file_type;

public:
    explicit SongFile(const std::string& filename);
    Song load_song() const;
};

#endif