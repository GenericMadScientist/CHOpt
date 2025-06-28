/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023, 2025 Raymond Wright
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
#include <optional>
#include <string>
#include <vector>

#include <sightread/metadata.hpp>
#include <sightread/qbmidiparser.hpp>
#include <sightread/song.hpp>

#include "settings.hpp"

class SongFile {
private:
    enum class FileType { Chart, Midi, QbMidi };

    std::vector<std::uint8_t> m_loaded_file;
    SightRead::Metadata m_metadata;
    FileType m_file_type;
    std::optional<SightRead::Console> m_console;
    std::string m_file_song_name;

public:
    explicit SongFile(const std::string& filename);
    SightRead::Song load_song(Game game) const;
};

#endif
