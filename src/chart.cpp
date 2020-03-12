/*
 * chopt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020 Raymond Wright
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
#include <charconv>
#include <cstdlib>
#include <set>
#include <stdexcept>

#include "chart.hpp"

static bool string_starts_with(std::string_view input, std::string_view pattern)
{
    if (input.size() < pattern.size()) {
        return false;
    }

    return input.substr(0, pattern.size()) == pattern;
}

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
        const auto line = input;
        input.remove_prefix(input.size());
        return line;
    }

    const auto line = input.substr(0, newline_location);
    input.remove_prefix(newline_location);
    input = skip_whitespace(input);
    return line;
}

// Split input by space characters, similar to .Split(' ') in C#. Note that
// the lifetime of the string_views in the output is the same as that of the
// input.
static std::vector<std::string_view> split_by_space(std::string_view input)
{
    std::vector<std::string_view> substrings;

    while (true) {
        auto space_location = input.find(' ');
        if (space_location == std::string_view::npos) {
            break;
        }
        substrings.push_back(input.substr(0, space_location));
        input.remove_prefix(space_location + 1);
    }

    substrings.push_back(input);
    return substrings;
}

// Return the substring with no leading or trailing quotation marks.
static std::string_view trim_quotes(std::string_view input)
{
    const auto first_non_quote = input.find_first_not_of('"');
    if (first_non_quote == std::string_view::npos) {
        return input.substr(0, 0);
    }
    const auto last_non_quote = input.find_last_not_of('"');
    return input.substr(first_non_quote, last_non_quote + 1);
}

// Convert a string_view to a uint32_t. If there are any problems with the
// input, this function throws.
static uint32_t string_view_to_uint(std::string_view input)
{
    uint32_t result;
    const char* last = input.data() + input.size();
    auto [p, ec] = std::from_chars(input.data(), last, result);
    if ((ec != std::errc()) || (p != last)) {
        throw std::invalid_argument("string_view does not convert to uint");
    }
    return result;
}

// Convert a string_view to an int32_t. If there are any problems with the
// input, this function throws.
static int32_t string_view_to_int(std::string_view input)
{
    int32_t result;
    const char* last = input.data() + input.size();
    auto [p, ec] = std::from_chars(input.data(), last, result);
    if ((ec != std::errc()) || (p != last)) {
        throw std::invalid_argument("string_view does not convert to int");
    }
    return result;
}

// Convert a string_view to a float. If there are any problems with the
// input, this function throws.
static float string_view_to_float(std::string_view input)
{
// We need to do this conditional because for now only MSVC's STL
// implements std::from_chars for floats.
#if defined(_MSC_VER) && _MSC_VER >= 1924
    float result;
    const char* last = input.data() + input.size();
    auto [p, ec] = std::from_chars(input.data(), last, result);
    if ((ec != std::errc()) || (p != last)) {
        throw std::invalid_argument("string_view does not convert to float");
    }
    return result;
#else
    std::string null_terminated_input(input);
    size_t chars_processed = 0;
    float result = std::stof(null_terminated_input, &chars_processed);
    if (chars_processed != null_terminated_input.size()) {
        throw std::invalid_argument("string_view does not convert to float");
    }
    return result;
#endif
}

std::string_view Chart::read_song_header(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("[Song] does not open with {");
    }

    std::vector<std::string_view> lines;

    while (true) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        lines.push_back(next_line);
    }

    for (auto line : lines) {
        if (string_starts_with(line, "Offset = ")) {
            constexpr auto OFFSET_LEN = 9;
            line.remove_prefix(OFFSET_LEN);
            offset = string_view_to_float(line);
        } else if (string_starts_with(line, "Resolution = ")) {
            constexpr auto RESOLUTION_LEN = 13;
            line.remove_prefix(RESOLUTION_LEN);
            resolution = string_view_to_float(line);
        }
    }

    return input;
}

std::string_view Chart::read_sync_track(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("[SyncTrack] does not open with {");
    }

    std::vector<std::string> lines;

    while (true) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        lines.emplace_back(next_line);
    }

    for (const auto& line : lines) {
        const auto split_string = split_by_space(line);
        if (split_string.size() < 4) {
            throw std::invalid_argument("Event missing data");
        }
        const auto position = string_view_to_uint(split_string[0]);

        auto type = split_string[2];

        if (type == "TS") {
            const auto numerator = string_view_to_uint(split_string[3]);
            uint32_t denominator = 2;
            if (split_string.size() > 4) {
                denominator = string_view_to_uint(split_string[4]);
            }
            time_sigs.push_back({position, numerator, 1U << denominator});
        } else if (type == "B") {
            const auto bpm = string_view_to_uint(split_string[3]);
            bpms.push_back({position, bpm});
        }
    }

    return input;
}

std::string_view Chart::read_events(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("[Events] does not open with {");
    }

    std::vector<std::string> lines;

    while (true) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        lines.emplace_back(next_line);
    }

    for (const auto& line : lines) {
        const auto split_string = split_by_space(line);
        if (split_string.size() < 4) {
            throw std::invalid_argument("Event missing data");
        }
        const auto position = string_view_to_uint(split_string[0]);

        auto type = split_string[2];

        if (type == "E") {
            if ((split_string[3] == "\"section") || (split_string.size() > 4)) {
                std::string section_name(trim_quotes(split_string[4]));
                constexpr auto NAME_START = 5U;
                for (auto i = NAME_START; i < split_string.size(); ++i) {
                    section_name += " ";
                    section_name += trim_quotes(split_string[i]);
                }
                sections.push_back({position, section_name});
            }
        }
    }

    return input;
}

std::string_view Chart::read_single_track(std::string_view input,
                                          Difficulty diff)
{
    constexpr auto GREEN_CODE = 0;
    constexpr auto RED_CODE = 1;
    constexpr auto YELLOW_CODE = 2;
    constexpr auto BLUE_CODE = 3;
    constexpr auto ORANGE_CODE = 4;
    constexpr auto FORCED_CODE = 5;
    constexpr auto TAP_CODE = 6;
    constexpr auto OPEN_CODE = 7;

    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("A [*Single] track does not open with {");
    }

    std::vector<std::string> lines;

    while (true) {
        next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        lines.emplace_back(next_line);
    }

    std::set<uint32_t> forced_flags;
    std::set<uint32_t> tap_flags;

    for (const auto& line : lines) {
        const auto split_string = split_by_space(line);
        if (split_string.size() < 4) {
            throw std::invalid_argument("Event missing data");
        }
        const auto position = string_view_to_uint(split_string[0]);
        auto type = split_string[2];

        if (type == "N") {
            constexpr auto NOTE_EVENT_LENGTH = 5;
            if (split_string.size() < NOTE_EVENT_LENGTH) {
                throw std::invalid_argument("Note event missing data");
            }
            const auto fret_type = string_view_to_int(split_string[3]);
            const auto length = string_view_to_uint(split_string[4]);
            switch (fret_type) {
            case GREEN_CODE:
                note_tracks[diff].notes.push_back(
                    {position, length, NoteColour::Green});
                break;
            case RED_CODE:
                note_tracks[diff].notes.push_back(
                    {position, length, NoteColour::Red});
                break;
            case YELLOW_CODE:
                note_tracks[diff].notes.push_back(
                    {position, length, NoteColour::Yellow});
                break;
            case BLUE_CODE:
                note_tracks[diff].notes.push_back(
                    {position, length, NoteColour::Blue});
                break;
            case ORANGE_CODE:
                note_tracks[diff].notes.push_back(
                    {position, length, NoteColour::Orange});
                break;
            case FORCED_CODE:
                forced_flags.insert(position);
                break;
            case TAP_CODE:
                tap_flags.insert(position);
                break;
            case OPEN_CODE:
                note_tracks[diff].notes.push_back(
                    {position, length, NoteColour::Open});
                break;
            default:
                throw std::invalid_argument("Invalid note type");
            }
        } else if (type == "S") {
            constexpr auto SP_EVENT_LENGTH = 5;
            if (split_string.size() < SP_EVENT_LENGTH) {
                throw std::invalid_argument("SP event missing data");
            }
            if (string_view_to_int(split_string[3]) != 2) {
                continue;
            }
            const auto length = string_view_to_uint(split_string[4]);
            note_tracks[diff].sp_phrases.push_back({position, length});
        } else if (type == "E") {
            const auto event_name = split_string[3];
            note_tracks[diff].events.push_back(
                {position, std::string(event_name)});
        }
    }

    for (auto& note : note_tracks[diff].notes) {
        if (forced_flags.count(note.position) != 0) {
            note.is_forced = true;
        }
        if (tap_flags.count(note.position) != 0) {
            note.is_tap = true;
        }
    }

    return input;
}

static std::string_view skip_unrecognised_section(std::string_view input)
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
    // Trim off UTF-8 BOM if present
    if (input.size() >= 3 && input.substr(0, 3) == "\xEF\xBB\xBF") {
        input.remove_prefix(3);
    }

    if (break_off_newline(input) != "[Song]") {
        throw std::runtime_error("Chart missing [Song] section");
    }
    input = read_song_header(input);

    if (break_off_newline(input) != "[SyncTrack]") {
        throw std::runtime_error("Chart missing [SyncTrack] section");
    }
    input = read_sync_track(input);

    if (break_off_newline(input) != "[Events]") {
        throw std::runtime_error("Chart missing [Events] section");
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
