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

#include "points.hpp"

static bool operator==(const Position& lhs, const Position& rhs)
{
    if (lhs.beat.value() != Approx(rhs.beat.value())) {
        return false;
    }
    return lhs.measure.value() == Approx(rhs.measure.value());
}

static bool operator==(const Point& lhs, const Point& rhs)
{
    return std::tie(lhs.position, lhs.hit_window_start, lhs.hit_window_end,
                    lhs.value, lhs.is_hold_point, lhs.is_sp_granting_note)
        == std::tie(rhs.position, rhs.hit_window_start, rhs.hit_window_end,
                    rhs.value, rhs.is_hold_point, rhs.is_sp_granting_note);
}

static bool operator==(const PointSet& set, const std::vector<Point>& points)
{
    return std::equal(set.cbegin(), set.cend(), points.cbegin(), points.cend());
}

TEST_CASE("Non-hold notes", "Non hold")
{
    SECTION("Single notes give 50 points")
    {
        const auto track = NoteTrack({{768}, {960}}, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 192);
        const auto points = PointSet(track, 192, converter);
        const auto expected_points
            = std::vector<Point>({{{Beat(4.0), Measure(1.0)},
                                   {Beat(3.86), Measure(0.965)},
                                   {Beat(4.14), Measure(1.035)},
                                   50,
                                   false,
                                   false},
                                  {{Beat(5.0), Measure(1.25)},
                                   {Beat(4.86), Measure(1.215)},
                                   {Beat(5.14), Measure(1.285)},
                                   50,
                                   false,
                                   false}});

        REQUIRE(points == expected_points);
    }

    SECTION("Chords give multiples of 50 points")
    {
        const auto track = NoteTrack(
            {{768, 0, NoteColour::Green}, {768, 0, NoteColour::Red}}, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 192);
        const auto points = PointSet(track, 192, converter);
        const auto expected_points
            = std::vector<Point>({{{Beat(4.0), Measure(1.0)},
                                   {Beat(3.86), Measure(0.965)},
                                   {Beat(4.14), Measure(1.035)},
                                   100,
                                   false,
                                   false}});

        REQUIRE(points == expected_points);
    }
}

TEST_CASE("Hold notes", "Hold")
{
    SECTION("Hold note points depend on resolution")
    {
        const auto track = NoteTrack({{768, 15}}, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 192);
        const auto first_points = PointSet(track, 192, converter);
        const auto first_expected_points = std::vector<Point>(
            {{{Beat(4.0), Measure(1.0)},
              {Beat(3.86), Measure(0.965)},
              {Beat(4.14), Measure(1.035)},
              50,
              false,
              false},
             {{Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
              {Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
              {Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
              1,
              true,
              false},
             {{Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
              {Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
              {Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
              1,
              true,
              false},
             {{Beat(789.0 / 192.0), Measure(789.0 / 768.0)},
              {Beat(789.0 / 192.0), Measure(789.0 / 768.0)},
              {Beat(789.0 / 192.0), Measure(789.0 / 768.0)},
              1,
              true,
              false}});
        const auto second_converter = TimeConverter(SyncTrack(), 200);
        const auto second_points = PointSet(track, 200, second_converter);
        const auto second_expected_points = std::vector<Point>(
            {{{Beat(768.0 / 200.0), Measure(768.0 / 800.0)},
              {Beat(768.0 / 200.0 - 0.14), Measure(768.0 / 800.0 - 0.035)},
              {Beat(768.0 / 200.0 + 0.14), Measure(768.0 / 800.0 + 0.035)},
              50,
              false,
              false},
             {{Beat(776.0 / 200.0), Measure(776.0 / 800.0)},
              {Beat(776.0 / 200.0), Measure(776.0 / 800.0)},
              {Beat(776.0 / 200.0), Measure(776.0 / 800.0)},
              1,
              true,
              false},
             {{Beat(784.0 / 200.0), Measure(784.0 / 800.0)},
              {Beat(784.0 / 200.0), Measure(784.0 / 800.0)},
              {Beat(784.0 / 200.0), Measure(784.0 / 800.0)},
              1,
              true,
              false}});

        REQUIRE(first_points == first_expected_points);
        REQUIRE(second_points == second_expected_points);
    }

    SECTION("Hold note points and chords")
    {
        const auto track = NoteTrack(
            {{768, 7, NoteColour::Green}, {768, 8, NoteColour::Red}}, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 192);
        const auto points = PointSet(track, 192, converter);
        const auto expected_points = std::vector<Point>(
            {{{Beat(4.0), Measure(1.0)},
              {Beat(3.86), Measure(0.965)},
              {Beat(4.14), Measure(1.035)},
              100,
              false,
              false},
             {{Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
              {Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
              {Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
              1,
              true,
              false},
             {{Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
              {Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
              {Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
              1,
              true,
              false}});

        REQUIRE(points == expected_points);
    }

    SECTION("Resolutions below 25 do not enter an infinite loop")
    {
        const auto track = NoteTrack({{768, 2}}, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 1);
        const auto points = PointSet(track, 1, converter);
        const auto expected_points
            = std::vector<Point>({{{Beat(768.0), Measure(192.0)},
                                   {Beat(767.86), Measure(191.965)},
                                   {Beat(768.14), Measure(192.035)},
                                   50,
                                   false,
                                   false},
                                  {{Beat(769.0), Measure(192.25)},
                                   {Beat(769.0), Measure(192.25)},
                                   {Beat(769.0), Measure(192.25)},
                                   1,
                                   true,
                                   false},
                                  {{Beat(770.0), Measure(192.5)},
                                   {Beat(770.0), Measure(192.5)},
                                   {Beat(770.0), Measure(192.5)},
                                   1,
                                   true,
                                   false}});

        REQUIRE(points == expected_points);
    }
}

TEST_CASE("Points are sorted", "Sorted")
{
    const auto track = NoteTrack({{768, 15}, {770, 0}}, {}, {});
    const auto converter = TimeConverter(SyncTrack(), 192);
    const auto points = PointSet(track, 192, converter);
    const auto expected_points = std::vector<Point>(
        {{{Beat(4.0), Measure(1.0)},
          {Beat(3.86), Measure(0.965)},
          {Beat(4.14), Measure(1.035)},
          50,
          false,
          false},
         {{Beat(770.0 / 192.0), Measure(770.0 / 768.0)},
          {Beat(770.0 / 192.0 - 0.14), Measure(770.0 / 768.0 - 0.035)},
          {Beat(770.0 / 192.0 + 0.14), Measure(770.0 / 768.0 + 0.035)},
          50,
          false,
          false},
         {{Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
          {Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
          {Beat(775.0 / 192.0), Measure(775.0 / 768.0)},
          1,
          true,
          false},
         {{Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
          {Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
          {Beat(782.0 / 192.0), Measure(782.0 / 768.0)},
          1,
          true,
          false},
         {{Beat(789.0 / 192.0), Measure(789.0 / 768.0)},
          {Beat(789.0 / 192.0), Measure(789.0 / 768.0)},
          {Beat(789.0 / 192.0), Measure(789.0 / 768.0)},
          1,
          true,
          false}});

    REQUIRE(points == expected_points);
}

TEST_CASE("End of SP phrase points", "End of SP")
{
    const auto track = NoteTrack({{768}, {960}, {1152}},
                                 {{768, 1}, {900, 50}, {1100, 53}}, {});
    const auto converter = TimeConverter(SyncTrack(), 192);
    const auto points = PointSet(track, 192, converter);
    const auto expected_points
        = std::vector<Point>({{{Beat(4.0), Measure(1.0)},
                               {Beat(3.86), Measure(0.965)},
                               {Beat(4.14), Measure(1.035)},
                               50,
                               false,
                               true},
                              {{Beat(5.0), Measure(1.25)},
                               {Beat(4.86), Measure(1.215)},
                               {Beat(5.14), Measure(1.285)},
                               50,
                               false,
                               false},
                              {{Beat(6.0), Measure(1.5)},
                               {Beat(5.86), Measure(1.465)},
                               {Beat(6.14), Measure(1.535)},
                               50,
                               false,
                               true}});

    REQUIRE(points == expected_points);
}

TEST_CASE("Combo multiplier is taken into account", "Multiplier")
{
    SECTION("Multiplier applies to non-holds")
    {
        std::vector<Note> notes;
        notes.reserve(50);
        for (auto i = 0U; i < 50U; ++i) {
            notes.push_back({192 * i});
        }
        const auto track = NoteTrack(notes, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 192);
        std::vector<Point> expected_points;
        expected_points.reserve(50);
        for (auto i = 0U; i < 50U; ++i) {
            const auto mult = 1U + std::min((i + 1) / 10, 3U);
            expected_points.push_back(
                {{Beat(i), Measure(i / 4.0)},
                 {Beat(i - 0.14), Measure(i / 4.0 - 0.035)},
                 {Beat(i + 0.14), Measure(i / 4.0 + 0.035)},
                 50 * mult,
                 false,
                 false});
        }

        REQUIRE(PointSet(track, 192, converter) == expected_points);
    }

    SECTION("Hold points are multiplied")
    {
        std::vector<Note> notes;
        notes.reserve(50);
        for (auto i = 0U; i < 50U; ++i) {
            notes.push_back({192 * i});
        }
        notes.push_back({9600, 192});

        const auto track = NoteTrack(notes, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 192);
        const auto points = PointSet(track, 192, converter);

        REQUIRE(std::prev(points.cend())->value == 4);
    }

    SECTION("Later hold points in extended sustains are multiplied")
    {
        std::vector<Note> notes;
        notes.reserve(10);
        for (auto i = 0U; i < 10U; ++i) {
            notes.push_back({192 * i});
        }
        notes[0].length = 2000;

        const auto track = NoteTrack(notes, {}, {});
        const auto converter = TimeConverter(SyncTrack(), 192);
        const auto points = PointSet(track, 192, converter);

        REQUIRE(std::prev(points.cend())->value == 2);
    }
}
