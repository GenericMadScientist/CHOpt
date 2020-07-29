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

#include "catch.hpp"

#include "midi.hpp"

TEST_CASE("parse_midi reads header correctly")
{
    std::vector<std::uint8_t> data {0x4D, 0x54, 0x68, 0x64, 0, 0, 0,
                                    6,    0,    1,    0,    0, 1, 0xE0};
    std::vector<std::uint8_t> bad_data {0x4D, 0x53, 0x68, 0x64, 0, 0, 0,
                                        6,    0,    1,    0,    0, 1, 0xE0};

    const auto midi = parse_midi(data);

    REQUIRE(midi.ticks_per_quarter_note == 0x1E0);
    REQUIRE(midi.num_of_tracks == 0);
    REQUIRE_THROWS([&] { return parse_midi(bad_data); }());
}
