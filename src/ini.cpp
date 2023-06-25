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

#include <algorithm>
#include <stdexcept>

#include "ini.hpp"
#include "stringutil.hpp"

SightRead::Metadata parse_ini(std::string_view data)
{
    constexpr auto ARTIST_SIZE = 6;
    constexpr auto CHARTER_SIZE = 7;
    constexpr auto FRETS_SIZE = 5;
    constexpr auto NAME_SIZE = 4;

    std::string u8_string = to_utf8_string(data);
    data = u8_string;

    SightRead::Metadata metadata;
    metadata.name = "Unknown Song";
    metadata.artist = "Unknown Artist";
    metadata.charter = "Unknown Charter";
    while (!data.empty()) {
        const auto line = break_off_newline(data);
        if (line.starts_with("name")) {
            auto value = skip_whitespace(line.substr(NAME_SIZE));
            if (value[0] != '=') {
                continue;
            }
            value = skip_whitespace(value.substr(1));
            metadata.name = value;
        } else if (line.starts_with("artist")) {
            auto value = skip_whitespace(line.substr(ARTIST_SIZE));
            if (value[0] != '=') {
                continue;
            }
            value = skip_whitespace(value.substr(1));
            metadata.artist = value;
        } else if (line.starts_with("charter")) {
            auto value = skip_whitespace(line.substr(CHARTER_SIZE));
            if (value[0] != '=') {
                continue;
            }
            value = skip_whitespace(value.substr(1));
            if (!value.empty()) {
                metadata.charter = value;
            }
        } else if (line.starts_with("frets")) {
            auto value = skip_whitespace(line.substr(FRETS_SIZE));
            if (value[0] != '=') {
                continue;
            }
            value = skip_whitespace(value.substr(1));
            if (!value.empty()) {
                metadata.charter = value;
            }
        }
    }

    return metadata;
}
