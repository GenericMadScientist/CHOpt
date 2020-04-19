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

#include "catch.hpp"

#include "points.hpp"

static bool operator==(const Beat& lhs, const Beat& rhs)
{
    return lhs.value() == Approx(rhs.value());
}

static std::vector<int> set_values(const PointSet& points)
{
    std::vector<int> values;
    values.reserve(static_cast<std::size_t>(
        std::distance(points.cbegin(), points.cend())));
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        values.push_back(p->value);
    }
    return values;
}

static std::vector<Beat> set_position_beats(const PointSet& points)
{
    std::vector<Beat> values;
    values.reserve(static_cast<std::size_t>(
        std::distance(points.cbegin(), points.cend())));
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        values.push_back(p->position.beat);
    }
    return values;
}

TEST_CASE("Non-hold notes")
{
    SECTION("Single notes give 50 points")
    {
        NoteTrack track {{{768}, {960}}, {}, {}};
        TimeConverter converter {{}, 192};
        PointSet points {track, 192, converter, 1.0};
        std::vector<int> expected_values {50, 50};

        REQUIRE(set_values(points) == expected_values);
    }

    SECTION("Chords give multiples of 50 points")
    {
        NoteTrack track {
            {{768, 0, NoteColour::Green}, {768, 0, NoteColour::Red}}, {}, {}};
        TimeConverter converter {{}, 192};
        PointSet points {track, 192, converter, 1.0};
        std::vector<int> expected_values {100};

        REQUIRE(set_values(points) == expected_values);
    }
}

TEST_CASE("Hold notes")
{
    SECTION("Hold note points depend on resolution")
    {
        NoteTrack track {{{768, 15}}, {}, {}};
        TimeConverter converter {{}, 192};
        PointSet first_points {track, 192, converter, 1.0};
        std::vector<int> first_expected_values {50, 1, 1, 1};
        std::vector<Beat> first_expected_beats {Beat(4.0), Beat(4.03646),
                                                Beat(4.07292), Beat(4.10938)};
        TimeConverter second_converter {{}, 200};
        PointSet second_points {track, 200, second_converter, 1.0};
        std::vector<int> second_expected_values {50, 1, 1};
        std::vector<Beat> second_expected_beats {Beat(3.84), Beat(3.88),
                                                 Beat(3.92)};

        REQUIRE(set_values(first_points) == first_expected_values);
        REQUIRE(set_position_beats(first_points) == first_expected_beats);
        REQUIRE(set_values(second_points) == second_expected_values);
        REQUIRE(set_position_beats(second_points) == second_expected_beats);
    }

    SECTION("Hold note points and chords")
    {
        NoteTrack track {
            {{768, 7, NoteColour::Green}, {768, 8, NoteColour::Red}}, {}, {}};
        TimeConverter converter {{}, 192};
        PointSet points {track, 192, converter, 1.0};
        std::vector<int> expected_values {100, 1, 1};
        std::vector<Beat> expected_beats {Beat(4.0), Beat(4.03646),
                                          Beat(4.07292)};

        REQUIRE(set_values(points) == expected_values);
        REQUIRE(set_position_beats(points) == expected_beats);
    }

    SECTION("Resolutions below 25 do not enter an infinite loop")
    {
        NoteTrack track {{{768, 2}}, {}, {}};
        TimeConverter converter {{}, 1};
        PointSet points {track, 1, converter, 1.0};

        REQUIRE(std::distance(points.cbegin(), points.cend()) == 3);
    }
}

TEST_CASE("Points are sorted")
{
    NoteTrack track {{{768, 15}, {770, 0}}, {}, {}};
    TimeConverter converter {{}, 192};
    PointSet points {track, 192, converter, 1.0};
    const auto beats = set_position_beats(points);

    REQUIRE(std::is_sorted(beats.cbegin(), beats.cend()));
}

TEST_CASE("End of SP phrase points")
{
    NoteTrack track {
        {{768}, {960}, {1152}}, {{768, 1}, {900, 50}, {1100, 53}}, {}};
    TimeConverter converter {{}, 192};
    PointSet points {track, 192, converter, 1.0};

    REQUIRE(points.cbegin()->is_sp_granting_note);
    REQUIRE(!std::next(points.cbegin())->is_sp_granting_note);
    REQUIRE(std::next(points.cbegin(), 2)->is_sp_granting_note);
}

TEST_CASE("Combo multiplier is taken into account")
{
    SECTION("Multiplier applies to non-holds")
    {
        std::vector<Note> notes;
        notes.reserve(50);
        for (int i = 0; i < 50; ++i) {
            notes.push_back({192 * i});
        }
        NoteTrack track {notes, {}, {}};
        TimeConverter converter {{}, 192};
        PointSet points {track, 192, converter, 1.0};
        std::vector<int> expected_values;
        expected_values.reserve(50);
        for (int i = 0; i < 50; ++i) {
            const auto mult = 1 + std::min((i + 1) / 10, 3);
            expected_values.push_back(50 * mult);
        }

        REQUIRE(set_values(points) == expected_values);
    }

    SECTION("Hold points are multiplied")
    {
        std::vector<Note> notes;
        notes.reserve(50);
        for (int i = 0; i < 50; ++i) {
            notes.push_back({192 * i});
        }
        notes.push_back({9600, 192});

        NoteTrack track {notes, {}, {}};
        TimeConverter converter {{}, 192};
        PointSet points {track, 192, converter, 1.0};

        REQUIRE(std::prev(points.cend())->value == 4);
    }

    SECTION("Later hold points in extended sustains are multiplied")
    {
        std::vector<Note> notes;
        notes.reserve(10);
        for (int i = 0; i < 10; ++i) {
            notes.push_back({192 * i});
        }
        notes[0].length = 2000;

        NoteTrack track {notes, {}, {}};
        TimeConverter converter {{}, 192};
        PointSet points {track, 192, converter, 1.0};

        REQUIRE(std::prev(points.cend())->value == 2);
    }
}

TEST_CASE("hit_window_start and hit_window_end are set correctly")
{
    TimeConverter converter {{{}, {{0, 150000}, {768, 200000}}}, 192};

    SECTION("Hit window starts for notes are correct")
    {
        std::vector<Note> notes {{192}, {787}};
        NoteTrack track {notes, {}, {}};
        PointSet points {track, 192, converter, 1.0};

        REQUIRE(points.cbegin()->hit_window_start.beat == Beat(0.825));
        REQUIRE(std::next(points.cbegin())->hit_window_start.beat
                == Beat(3.89922));
    }

    SECTION("Hit window ends for notes are correct")
    {
        std::vector<Note> notes {{192}, {749}};
        NoteTrack track {notes, {}, {}};
        PointSet points {track, 192, converter, 1.0};

        REQUIRE(points.cbegin()->hit_window_end.beat == Beat(1.175));
        REQUIRE(std::next(points.cbegin())->hit_window_end.beat
                == Beat(4.10139));
    }

    SECTION("Hit window starts and ends for hold points are correct")
    {
        std::vector<Note> notes {{672, 192}};
        NoteTrack track {notes, {}, {}};
        PointSet points {track, 192, converter, 1.0};

        for (auto p = std::next(points.cbegin()); p < points.cend(); ++p) {
            REQUIRE(p->position.beat == p->hit_window_start.beat);
            REQUIRE(p->position.beat == p->hit_window_end.beat);
        }
    }

    SECTION("Squeeze setting is accounted for")
    {
        std::vector<Note> notes {{192}};
        NoteTrack track {notes, {}, {}};
        PointSet points {track, 192, converter, 0.5};

        REQUIRE(points.cbegin()->hit_window_start.beat == Beat(0.9125));
        REQUIRE(points.cbegin()->hit_window_end.beat == Beat(1.0875));
    }
}
