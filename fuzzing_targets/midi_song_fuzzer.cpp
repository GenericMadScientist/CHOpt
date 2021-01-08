/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2021 Raymond Wright
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

#include <cstddef>

#include "song.hpp"
#include "songparts.hpp"

extern "C" int LLVMFuzzerTestOneInput(const char* data, size_t size)
{
    const std::vector<std::uint8_t> input {data, data + size};
    try {
        Song::from_midi(parse_midi(input), {});
        return 0;
    } catch (const ParseError&) {
        return 0;
    }
}
