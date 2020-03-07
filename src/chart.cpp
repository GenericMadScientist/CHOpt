/* 
 *  chopt - Star Power optimiser for Clone Hero
 *  Copyright (C) 2020  Raymond Wright
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <stdexcept>

#include "chart.h"

static std::string_view skip_whitespace(std::string_view input)
{
    const auto first_non_ws_location = input.find_first_not_of(" \f\n\r\t\v");
    input.remove_prefix(std::min(first_non_ws_location, input.size()));
    return input;
}

// This returns a string_view from the start of input until a carriage return
// or newline. input is changed to point to the first character past the
// detected newline character that is not a whitespace character.
static std::string_view break_off_newline(std::string_view& input)
{
    const auto newline_location = input.find_first_of("\r\n");
    if (newline_location == std::string_view::npos) {
        throw std::runtime_error("Missing newline");
    }

    const auto line = input.substr(0, newline_location);
    input.remove_prefix(newline_location);
    input = skip_whitespace(input);
    return line;
}

std::string_view Chart::read_song_header(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("[Song] does not open with {");
    }

    while (1) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        song_header_lines.push_back(std::string(next_line));
    }

    return input;
}

std::string_view Chart::read_sync_track(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("[SyncTrack] does not open with {");
    }

    while (1) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        sync_track_lines.push_back(std::string(next_line));
    }

    return input;
}

std::string_view Chart::read_events(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("[Events] does not open with {");
    }

    while (1) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        event_lines.push_back(std::string(next_line));
    }

    return input;
}

std::string_view Chart::read_single_track(std::string_view input, Difficulty diff)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("A [*Single] track does not open with {");
    }

    while (1) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        // We are exploiting the std::map::operator[] behaviour of default
        // initialising map[key] if it does not exist.
        single_track_lines[diff].push_back(std::string(next_line));
    }

    return input;
}

std::string_view Chart::skip_unrecognised_section(std::string_view input) const
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("Unrecognised track does not open with {");
    }

    do {
        next_line = break_off_newline(input);
    } while (next_line != "}");

    return input;
}

Chart::Chart(std::string_view input)
{
    if (break_off_newline(input) != "[Song]") {
        throw std::runtime_error("Chart does not start with [Song] section");
    }
    input = read_song_header(input);

    if (break_off_newline(input) != "[SyncTrack]") {
        throw std::runtime_error("Chart does not start with [SyncTrack] section");
    }
    input = read_sync_track(input);

    if (break_off_newline(input) != "[Events]") {
        throw std::runtime_error("Chart does not start with [Events] section");
    }
    input = read_events(input);

    while (!input.empty()) {
        const auto header = break_off_newline(input);
        if (header == "[EasySingle]") {
            input = read_single_track(input, Difficulty::Easy);
        } else if (header == "[MediumSingle]") {
            input = read_single_track(input, Difficulty::Medium);
        } else if (header == "[HardSingle]") {
            input = read_single_track(input, Difficulty::Hard);
        } else if (header == "[ExpertSingle]") {
            input = read_single_track(input, Difficulty::Expert);
        } else {
            input = skip_unrecognised_section(input);
        }
    }
}
