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
#include <stdexcept>

#include "chart.hpp"

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

static std::string_view skip_section(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("Section does not open with {");
    }

    do {
        next_line = break_off_newline(input);
    } while (next_line != "}");

    return input;
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

Chart parse_chart(std::string_view data)
{
    Chart chart;

    // Trim off UTF-8 BOM if present
    if (string_starts_with(data, "\xEF\xBB\xBF")) {
        data.remove_prefix(3);
    }

    while (!data.empty()) {
        const auto header = strip_square_brackets(break_off_newline(data));
        chart.sections.push_back(ChartSection {std::string(header)});
        data = skip_section(data);
    }

    return chart;
}
