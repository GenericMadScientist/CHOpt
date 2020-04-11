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

#include <iostream>

#include "catch.hpp"

#include "image.hpp"

static bool operator==(const DrawnNote& lhs, const DrawnNote& rhs)
{
    return lhs.beat == Approx(rhs.beat) && lhs.colour == rhs.colour;
}

static bool operator==(const DrawnRow& lhs, const DrawnRow& rhs)
{
    return lhs.start == Approx(rhs.start) && lhs.end == Approx(rhs.end);
}

TEST_CASE("Non-SP notes are handled correctly")
{
    const auto track = NoteTrack({{0}, {768, 0, NoteColour::Red}}, {}, {});
    const auto insts = create_instructions(track, 192, SyncTrack());
    const auto expected_notes = std::vector<DrawnNote>(
        {{0.0, NoteColour::Green}, {4.0, NoteColour::Red}});

    REQUIRE(insts.notes == expected_notes);
}

TEST_CASE("Drawn rows are handled correctly")
{
    SECTION("Simple 4/4 is handled correctly")
    {
        const auto track = NoteTrack({{2880}}, {}, {});
        const auto insts = create_instructions(track, 192, SyncTrack());
        const auto expected_rows = std::vector<DrawnRow>({{0.0, 16.0}});

        REQUIRE(insts.rows == expected_rows);
    }

    SECTION("3/4 and 3/8 are coped with")
    {
        const auto track = NoteTrack({{2450}}, {}, {});
        const auto sync_track
            = SyncTrack({{0, 4, 4}, {768, 3, 4}, {1344, 3, 8}, {1632, 4, 4}});
        const auto insts = create_instructions(track, 192, sync_track);
        const auto expected_rows
            = std::vector<DrawnRow>({{0.0, 12.5}, {12.5, 16.5}});

        REQUIRE(insts.rows == expected_rows);
    }

    SECTION("Time signature changes off measure are coped with")
    {
        const auto track = NoteTrack({{768}}, {}, {});
        const auto sync_track
            = SyncTrack({{0, 4, 4}, {767, 3, 4}, {1344, 3, 8}});
        const auto insts = create_instructions(track, 192, sync_track);
        const auto expected_rows = std::vector<DrawnRow>({{0.0, 7.0}});

        REQUIRE(insts.rows == expected_rows);
    }

    SECTION("x/4 for x > 16 is coped with")
    {
        const auto track = NoteTrack({{0}}, {}, {});
        const auto sync_track = SyncTrack({{0, 17, 4}});
        const auto insts = create_instructions(track, 192, sync_track);
        const auto expected_rows
            = std::vector<DrawnRow>({{0.0, 16.0}, {16.0, 17.0}});

        REQUIRE(insts.rows == expected_rows);
    }
}
