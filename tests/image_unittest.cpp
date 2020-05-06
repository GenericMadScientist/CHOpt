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
        DrawingInstructions insts {track, 192, {}};
        std::vector<DrawnNote> expected_notes {
            {0.0, 0.0, NoteColour::Green, false},
            {4.0, 0.0, NoteColour::Red, false}};

        REQUIRE(insts.notes() == expected_notes);
    }

    SECTION("Sustains are handled correctly")
    {
        NoteTrack track {{{0, 96}}, {}, {}};
        DrawingInstructions insts {track, 192, {}};
        std::vector<DrawnNote> expected_notes {
            {0.0, 0.5, NoteColour::Green, false}};

        REQUIRE(insts.notes() == expected_notes);
    }

    SECTION("SP notes are recorded")
    {
        NoteTrack track {{{0}, {768}}, {{768, 100}}, {}};
        DrawingInstructions insts {track, 192, {}};
        std::vector<DrawnNote> expected_notes {
            {0.0, 0.0, NoteColour::Green, false},
            {4.0, 0.0, NoteColour::Green, true}};

        REQUIRE(insts.notes() == expected_notes);
    }
}

TEST_CASE("Drawn rows are handled correctly")
{
    SECTION("Simple 4/4 is handled correctly")
    {
        NoteTrack track {{{2880}}, {}, {}};
        DrawingInstructions insts {track, 192, {}};
        std::vector<DrawnRow> expected_rows {{0.0, 16.0}};

        REQUIRE(insts.rows() == expected_rows);
    }

    SECTION("3/4 and 3/8 are coped with")
    {
        NoteTrack track {{{2450}}, {}, {}};
        SyncTrack sync_track {
            {{0, 4, 4}, {768, 3, 4}, {1344, 3, 8}, {1632, 4, 4}}, {}};
        DrawingInstructions insts {track, 192, sync_track};
        std::vector<DrawnRow> expected_rows {{0.0, 12.5}, {12.5, 16.5}};

        REQUIRE(insts.rows() == expected_rows);
    }

    SECTION("Time signature changes off measure are coped with")
    {
        NoteTrack track {{{768}}, {}, {}};
        SyncTrack sync_track {{{0, 4, 4}, {767, 3, 4}, {1344, 3, 8}}, {}};
        DrawingInstructions insts {track, 192, sync_track};
        std::vector<DrawnRow> expected_rows {{0.0, 7.0}};

        REQUIRE(insts.rows() == expected_rows);
    }

    SECTION("x/4 for x > 16 is coped with")
    {
        NoteTrack track {{{0}}, {}, {}};
        SyncTrack sync_track {{{0, 17, 4}}, {}};
        DrawingInstructions insts {track, 192, sync_track};
        std::vector<DrawnRow> expected_rows {{0.0, 16.0}, {16.0, 17.0}};

        REQUIRE(insts.rows() == expected_rows);
    }

    SECTION("Enough rows are drawn for end of song sustains")
    {
        NoteTrack track {{{0, 3840}}, {}, {}};
        DrawingInstructions insts {track, 192, {}};

        REQUIRE(insts.rows().size() == 2);
    }
}

TEST_CASE("Beat lines are correct")
{
    SECTION("4/4 works fine")
    {
        NoteTrack track {{{767}}, {}, {}};
        DrawingInstructions insts {track, 192, {}};
        std::vector<double> expected_half_beat_lines {0.5, 1.5, 2.5, 3.5};
        std::vector<double> expected_beat_lines {1.0, 2.0, 3.0};
        std::vector<double> expected_measure_lines {0.0, 4.0};

        REQUIRE(insts.half_beat_lines() == expected_half_beat_lines);
        REQUIRE(insts.beat_lines() == expected_beat_lines);
        REQUIRE(insts.measure_lines() == expected_measure_lines);
    }

    SECTION("4/8 works fine")
    {
        NoteTrack track {{{767}}, {}, {}};
        SyncTrack sync_track {{{0, 4, 8}}, {}};
        DrawingInstructions insts {track, 192, sync_track};
        std::vector<double> expected_half_beat_lines {0.25, 0.75, 1.25, 1.75,
                                                      2.25, 2.75, 3.25, 3.75};
        std::vector<double> expected_beat_lines {0.5, 1.0, 1.5, 2.5, 3.0, 3.5};
        std::vector<double> expected_measure_lines {0.0, 2.0, 4.0};

        REQUIRE(insts.half_beat_lines() == expected_half_beat_lines);
        REQUIRE(insts.beat_lines() == expected_beat_lines);
        REQUIRE(insts.measure_lines() == expected_measure_lines);
    }

    SECTION("Combination of 4/4 and 4/8 works fine")
    {
        NoteTrack track {{{1151}}, {}, {}};
        SyncTrack sync_track {{{0, 4, 4}, {768, 4, 8}}, {}};
        DrawingInstructions insts {track, 192, sync_track};
        std::vector<double> expected_half_beat_lines {0.5,  1.5,  2.5,  3.5,
                                                      4.25, 4.75, 5.25, 5.75};
        std::vector<double> expected_beat_lines {1.0, 2.0, 3.0, 4.5, 5.0, 5.5};
        std::vector<double> expected_measure_lines {0.0, 4.0, 6.0};

        REQUIRE(insts.half_beat_lines() == expected_half_beat_lines);
        REQUIRE(insts.beat_lines() == expected_beat_lines);
        REQUIRE(insts.measure_lines() == expected_measure_lines);
    }
}

TEST_CASE("Time signatures are handled correctly")
{
    NoteTrack track {{{1920}}, {}, {}};
    SyncTrack sync_track {{{0, 4, 4}, {768, 4, 8}}, {}};
    DrawingInstructions insts {track, 192, sync_track};
    std::vector<std::tuple<double, int, int>> expected_time_sigs {{0.0, 4, 4},
                                                                  {4.0, 4, 8}};

    REQUIRE(insts.time_sigs() == expected_time_sigs);
}

TEST_CASE("Tempos are handled correctly")
{
    SECTION("Normal tempos are handled correctly")
    {
        NoteTrack track {{{1920}}, {}, {}};
        SyncTrack sync_track {{}, {{0, 150000}, {384, 120000}, {768, 200000}}};
        DrawingInstructions insts {track, 192, sync_track};
        std::vector<std::tuple<double, double>> expected_bpms {
            {0.0, 150.0}, {2.0, 120.0}, {4.0, 200.0}};

        REQUIRE(insts.bpms() == expected_bpms);
    }

    SECTION("Tempo changes past the end of the song are removed")
    {
        NoteTrack track {{{768}}, {}, {}};
        SyncTrack sync_track {{}, {{0, 120000}, {1920, 200000}}};
        DrawingInstructions insts {track, 192, sync_track};

        REQUIRE(insts.bpms().size() == 1);
    }
}

TEST_CASE("Green ranges for SP phrases are added correctly")
{
    NoteTrack track {{{1000}}, {{768, 384}}, {}};
    DrawingInstructions insts {track, 192, {}};
    insts.add_sp_phrases(track, 192);
    std::vector<std::tuple<double, double>> expected_green_ranges {{4.0, 6.0}};

    REQUIRE(insts.green_ranges() == expected_green_ranges);
}

TEST_CASE("add_sp_acts adds correct ranges")
{
    NoteTrack track {{{0}, {192}}, {}, {}};
    TimeConverter converter {{}, 192};
    PointSet points {track, 192, converter, 1.0};
    DrawingInstructions insts {track, 192, {}};
    Path path {{{points.cbegin(), points.cbegin() + 1}}, 0};
    insts.add_sp_acts(path);
    std::vector<std::tuple<double, double>> expected_blue_ranges {{0.0, 1.0}};

    REQUIRE(insts.blue_ranges() == expected_blue_ranges);
}

TEST_CASE("add_solo_sections add correct ranges")
{
    NoteTrack track {{{0}}, {}, {{192, 384, 0}}};
    DrawingInstructions insts {track, 192, {}};
    insts.add_solo_sections(track, 192);
    std::vector<std::tuple<double, double>> expected_solo_ranges {{1.0, 2.0}};

    REQUIRE(insts.solo_ranges() == expected_solo_ranges);
}

TEST_CASE("add_measure_values gives correct values")
{
    SECTION("Notes with no activations or solos")
    {
        NoteTrack track {{{0}, {768}}, {}, {}};
        PointSet points {track, 192, {{}, 192}, 1.0};
        Path path;
        DrawingInstructions insts {track, 192, {}};
        insts.add_measure_values(points, path);
        std::vector<int> expected_base_values {50, 50};
        std::vector<int> expected_score_values {50, 100};

        REQUIRE(insts.base_values() == expected_base_values);
        REQUIRE(insts.score_values() == expected_score_values);
    }

    SECTION("Solos are added")
    {
        NoteTrack track {{{768}}, {}, {{0, 100, 100}, {200, 800, 100}}};
        PointSet points {track, 192, {{}, 192}, 1.0};
        Path path;
        DrawingInstructions insts {track, 192, {}};
        insts.add_measure_values(points, path);
        std::vector<int> expected_score_values {100, 250};

        REQUIRE(insts.score_values() == expected_score_values);
    }

    SECTION("Activations are added")
    {
        NoteTrack track {{{0}, {192}, {384}, {768}}, {}, {}};
        PointSet points {track, 192, {{}, 192}, 1.0};
        Path path {{{points.cbegin() + 2, points.cbegin() + 3}}, 100};
        DrawingInstructions insts {track, 192, {}};
        insts.add_measure_values(points, path);
        std::vector<int> expected_score_values {200, 300};

        REQUIRE(insts.score_values() == expected_score_values);
    }
}

TEST_CASE("add_sp_values gives correct values")
{
    NoteTrack track {{{0}, {192, 768}}, {{192, 50}}, {}};
    SpData sp_data {track, 192, {}, 1.0};
    DrawingInstructions insts {track, 192, {}};
    insts.add_sp_values(sp_data);
    std::vector<double> expected_sp_values {3.14, 1.0};

    REQUIRE(insts.sp_values() == expected_sp_values);
}
