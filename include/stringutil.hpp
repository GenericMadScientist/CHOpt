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

#ifndef CHOPT_STRINGUTIL_HPP
#define CHOPT_STRINGUTIL_HPP

#include <string>
#include <string_view>

// This returns a string_view from the start of input until a carriage return
// or newline. input is changed to point to the first character past the
// detected newline character that is not a whitespace character.
std::string_view break_off_newline(std::string_view& input);

std::string_view skip_whitespace(std::string_view input);

std::string to_ordinal(int ordinal);

// Convert a UTF-8 or UTF-16le string to a UTF-8 string.
std::string to_utf8_string(std::string_view input);

bool ends_with_suffix(const std::string& string, std::string_view suffix);

#endif
