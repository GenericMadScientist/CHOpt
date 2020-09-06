/*
 * CHOpt - Star Power optimiser for Clone Hero
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
#include <optional>
#include <stdexcept>

// libc++ is special and doesn't support <cuchar> yet, so we need a different
// method for the UTF-16le -> UTF-8 conversion. The alternative method isn't
// general because it uses a feature deprecated in C++17 and Microsoft's STL
// will complain if we use it.
#ifndef _LIBCPP_VERSION
#include <climits>
#include <cuchar>
#else
#include <codecvt>
#include <locale>
#endif

#include "chart.hpp"

static std::string utf16_to_utf8_string(std::u16string_view input)
{
#ifndef _LIBCPP_VERSION
    std::string u8_string;

    // This conversion method is from the c16rtomb page on cppreference. This
    // does not handle surrogate pairs, but I've only come across two UTF-16
    // charts so a proper solution can wait until the C++ standard library gets
    // a fix or a non-artificial chart comes up that this is a problem for.
    std::mbstate_t state {};
    char out[MB_LEN_MAX];
    for (auto c : input) {
        auto rc = std::c16rtomb(out, c, &state);
        if (rc != static_cast<std::size_t>(-1)) {
            for (auto i = 0u; i < rc; ++i) {
                u8_string.push_back(out[i]);
            }
        }
    }

    return u8_string;
#else
    // std::codecvt_utf8_utf16 is deprecated in C++17 and Microsoft's STL
    // complains about it so we can't have this be a general solution.
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}
        .to_bytes(input.cbegin(), input.cend());
#endif
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
    if (input.empty()) {
        throw std::invalid_argument("No lines left");
    }

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

static std::string_view strip_square_brackets(std::string_view input)
{
    return input.substr(1, input.size() - 2);
}

static bool string_starts_with(std::string_view input, std::string_view pattern)
{
    if (input.size() < pattern.size()) {
        return false;
    }

    return input.substr(0, pattern.size()) == pattern;
}

// Convert a string_view to an int. If there are any problems with the input,
// this function throws.
static std::optional<int> string_view_to_int(std::string_view input)
{
    int result = 0;
    const char* last = input.data() + input.size();
    auto [p, ec] = std::from_chars(input.data(), last, result);
    if ((ec != std::errc()) || (p != last)) {
        return std::nullopt;
    }
    return result;
}

// Split input by space characters, similar to .Split(' ') in C#. Note that
// the lifetime of the string_views in the output is the same as that of the
// input.
static std::vector<std::string_view> split_by_space(std::string_view input)
{
    std::vector<std::string_view> substrings;

    while (true) {
        const auto space_location = input.find(' ');
        if (space_location == std::string_view::npos) {
            break;
        }
        substrings.push_back(input.substr(0, space_location));
        input.remove_prefix(space_location + 1);
    }

    substrings.push_back(input);
    return substrings;
}

static ChartSection read_section(std::string_view& input)
{
    constexpr int FULL_TS_EVENT_SIZE = 5;

    ChartSection section;
    section.name = strip_square_brackets(break_off_newline(input));

    if (break_off_newline(input) != "{") {
        throw std::runtime_error("Section does not open with {");
    }

    while (true) {
        const auto next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        const auto separated_line = split_by_space(next_line);
        const auto key = separated_line[0];
        const auto key_val = string_view_to_int(key);
        if (key_val.has_value()) {
            const auto pos = *key_val;
            if (separated_line[2] == "N") {
                const auto fret = string_view_to_int(separated_line[3]).value();
                const auto length
                    = string_view_to_int(separated_line[4]).value();
                section.note_events.push_back(NoteEvent {pos, fret, length});
            } else if (separated_line[2] == "S") {
                const auto sp_key
                    = string_view_to_int(separated_line[3]).value();
                const auto length
                    = string_view_to_int(separated_line[4]).value();
                section.sp_events.push_back(SPEvent {pos, sp_key, length});
            } else if (separated_line[2] == "B") {
                const auto bpm = string_view_to_int(separated_line[3]).value();
                section.bpm_events.push_back(BpmEvent {pos, bpm});
            } else if (separated_line[2] == "TS") {
                const auto numer
                    = string_view_to_int(separated_line[3]).value();
                auto denom = 2;
                if (separated_line.size() >= FULL_TS_EVENT_SIZE) {
                    denom = string_view_to_int(separated_line[4]).value();
                }
                section.ts_events.push_back(TimeSigEvent {pos, numer, denom});
            } else if (separated_line[2] == "E") {
                section.events.push_back(
                    Event {pos, std::string {separated_line[3]}});
            }
        } else {
            std::string value {separated_line[2]};
            for (auto i = 3U; i < separated_line.size(); ++i) {
                value.append(separated_line[i]);
            }
            section.key_value_pairs[std::string(key)] = value;
        }
    }

    return section;
}

Chart parse_chart(std::string_view data)
{
    Chart chart;

    // We need this for the UTF-16le conversion, because the converted result
    // must live in the larger scope.
    std::string u8_string;

    if (string_starts_with(data, "\xEF\xBB\xBF")) {
        // Trim off UTF-8 BOM if present
        data.remove_prefix(3);
    } else if (string_starts_with(data, "\xFF\xFE")) {
        if (data.size() % 2 != 0) {
            throw std::invalid_argument("UTF-16le file has an odd size");
        }
        // Trim off UTF-16le BOM if present, then convert
        data.remove_prefix(2);
        // I'm pretty sure I really do need the reinterpret_cast here.
        std::u16string utf16_string_view {
            reinterpret_cast<const char16_t*>(data.data()), // NOLINT
            data.size() / 2};
        u8_string = utf16_to_utf8_string(utf16_string_view);
        data = u8_string;
    }

    while (!data.empty()) {
        chart.sections.push_back(read_section(data));
    }

    return chart;
}
