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

#include "image.hpp"

static bool operator==(const DrawnNote& lhs, const DrawnNote& rhs)
{
    return lhs.beat == Approx(rhs.beat) && lhs.colour == rhs.colour;
}

TEST_CASE("Non-SP notes are handled correctly")
{
    const auto track = NoteTrack({{0}, {768, 0, NoteColour::Red}}, {}, {});
    const auto insts = create_instructions(track, 192);
    const auto expected_notes = std::vector<DrawnNote>(
        {{0.0, NoteColour::Green}, {4.0, NoteColour::Red}});

    REQUIRE(insts.notes == expected_notes);
    REQUIRE(insts.number_of_rows == 1);
}

TEST_CASE("Number of rows is handled correctly")
{
    const auto track = NoteTrack({{36864}}, {}, {});
    const auto insts = create_instructions(track, 192);

    REQUIRE(insts.number_of_rows == 13);
}
