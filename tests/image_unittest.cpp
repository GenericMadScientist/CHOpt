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
    return lhs.beat == Approx(rhs.beat) && lhs.length == Approx(rhs.length)
        && lhs.colour == rhs.colour && lhs.is_sp_note == rhs.is_sp_note;
}

static bool operator==(const DrawnRow& lhs, const DrawnRow& rhs)
{
    return lhs.start == Approx(rhs.start) && lhs.end == Approx(rhs.end);
}

TEST_CASE("Notes are handled correclty")
{
    SECTION("Non-SP non-sustains are handled correctly")
    {
        NoteTrack track {{{0}, {768, 0, NoteColour::Red}}, {}, {}};
        const auto insts = create_instructions(track, 192, {});
        std::vector<DrawnNote> expected_notes {
            {0.0, 0.0, NoteColour::Green, false},
            {4.0, 0.0, NoteColour::Red, false}};

        REQUIRE(insts.notes == expected_notes);
    }

    SECTION("Sustains are handled correctly")
    {
        NoteTrack track {{{0, 96}}, {}, {}};
        const auto insts = create_instructions(track, 192, {});
        std::vector<DrawnNote> expected_notes {
            {0.0, 0.5, NoteColour::Green, false}};

        REQUIRE(insts.notes == expected_notes);
    }

    SECTION("SP notes are recorded")
    {
        NoteTrack track {{{0}, {768}}, {{768, 100}}, {}};
        const auto insts = create_instructions(track, 192, {});
        std::vector<DrawnNote> expected_notes {
            {0.0, 0.0, NoteColour::Green, false},
            {4.0, 0.0, NoteColour::Green, true}};

        REQUIRE(insts.notes == expected_notes);
    }
}

TEST_CASE("Drawn rows are handled correctly")
{
    SECTION("Simple 4/4 is handled correctly")
    {
        NoteTrack track {{{2880}}, {}, {}};
        const auto insts = create_instructions(track, 192, {});
        std::vector<DrawnRow> expected_rows {{0.0, 16.0}};

        REQUIRE(insts.rows == expected_rows);
    }

    SECTION("3/4 and 3/8 are coped with")
    {
        NoteTrack track {{{2450}}, {}, {}};
        SyncTrack sync_track {
            {{0, 4, 4}, {768, 3, 4}, {1344, 3, 8}, {1632, 4, 4}}, {}};
        const auto insts = create_instructions(track, 192, sync_track);
        std::vector<DrawnRow> expected_rows {{0.0, 12.5}, {12.5, 16.5}};

        REQUIRE(insts.rows == expected_rows);
    }

    SECTION("Time signature changes off measure are coped with")
    {
        NoteTrack track {{{768}}, {}, {}};
        SyncTrack sync_track {{{0, 4, 4}, {767, 3, 4}, {1344, 3, 8}}, {}};
        const auto insts = create_instructions(track, 192, sync_track);
        std::vector<DrawnRow> expected_rows {{0.0, 7.0}};

        REQUIRE(insts.rows == expected_rows);
    }

    SECTION("x/4 for x > 16 is coped with")
    {
        NoteTrack track {{{0}}, {}, {}};
        SyncTrack sync_track {{{0, 17, 4}}, {}};
        const auto insts = create_instructions(track, 192, sync_track);
        std::vector<DrawnRow> expected_rows {{0.0, 16.0}, {16.0, 17.0}};

        REQUIRE(insts.rows == expected_rows);
    }

    SECTION("Enough rows are drawn for end of song sustains")
    {
        NoteTrack track {{{0, 3840}}, {}, {}};
        const auto insts = create_instructions(track, 192, {});

        REQUIRE(insts.rows.size() == 2);
    }
}

TEST_CASE("Beat lines are correct")
{
    SECTION("4/4 works fine")
    {
        NoteTrack track {{{767}}, {}, {}};
        const auto insts = create_instructions(track, 192, {});
        std::vector<double> expected_half_beat_lines {0.5, 1.5, 2.5, 3.5};
        std::vector<double> expected_beat_lines {1.0, 2.0, 3.0};
        std::vector<double> expected_measure_lines {0.0};

        REQUIRE(insts.half_beat_lines == expected_half_beat_lines);
        REQUIRE(insts.beat_lines == expected_beat_lines);
        REQUIRE(insts.measure_lines == expected_measure_lines);
    }

    SECTION("4/8 works fine")
    {
        NoteTrack track {{{767}}, {}, {}};
        SyncTrack sync_track {{{0, 4, 8}}, {}};
        const auto insts = create_instructions(track, 192, sync_track);
        std::vector<double> expected_half_beat_lines {0.25, 0.75, 1.25, 1.75,
                                                      2.25, 2.75, 3.25, 3.75};
        std::vector<double> expected_beat_lines {0.5, 1.0, 1.5, 2.5, 3.0, 3.5};
        std::vector<double> expected_measure_lines {0.0, 2.0};

        REQUIRE(insts.half_beat_lines == expected_half_beat_lines);
        REQUIRE(insts.beat_lines == expected_beat_lines);
        REQUIRE(insts.measure_lines == expected_measure_lines);
    }

    SECTION("Combination of 4/4 and 4/8 works fine")
    {
        NoteTrack track {{{1151}}, {}, {}};
        SyncTrack sync_track {{{0, 4, 4}, {768, 4, 8}}, {}};
        const auto insts = create_instructions(track, 192, sync_track);
        std::vector<double> expected_half_beat_lines {0.5,  1.5,  2.5,  3.5,
                                                      4.25, 4.75, 5.25, 5.75};
        std::vector<double> expected_beat_lines {1.0, 2.0, 3.0, 4.5, 5.0, 5.5};
        std::vector<double> expected_measure_lines {0.0, 4.0};

        REQUIRE(insts.half_beat_lines == expected_half_beat_lines);
        REQUIRE(insts.beat_lines == expected_beat_lines);
        REQUIRE(insts.measure_lines == expected_measure_lines);
    }
}

TEST_CASE("Green ranges for SP phrases are added correctly")
{
    NoteTrack track {{{1000}}, {{768, 384}}, {}};
    const auto insts = create_instructions(track, 192, {});
    std::vector<std::tuple<double, double>> expected_green_ranges {{4.0, 6.0}};

    REQUIRE(insts.green_ranges == expected_green_ranges);
}
