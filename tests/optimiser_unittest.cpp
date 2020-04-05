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
#include <array>

#include "catch.hpp"

#include "optimiser.hpp"

TEST_CASE("SpBar methods", "SpBar")
{
    SECTION("add_phrase() works correctly")
    {
        SpBar sp_bar {0.0, 0.25};
        sp_bar.add_phrase();

        REQUIRE(sp_bar == SpBar {0.25, 0.5});

        sp_bar = {0.8, 1.0};
        sp_bar.add_phrase();

        REQUIRE(sp_bar == SpBar {1.0, 1.0});
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Non-hold notes", "Non hold")
{
    SECTION("Single notes give 50 points")
    {
        const auto track = NoteTrack({{768}, {960}}, {}, {});
        const auto points = ProcessedTrack(track, {}, {}).points();
        const auto expected_points = std::vector<Point>(
            {{Beat(4.0), 50, false, false}, {Beat(5.0), 50, false, false}});

        REQUIRE(points == expected_points);
    }

    SECTION("Chords give multiples of 50 points")
    {
        const auto track = NoteTrack(
            {{768, 0, NoteColour::Green}, {768, 0, NoteColour::Red}}, {}, {});
        const auto points = ProcessedTrack(track, {}, {}).points();
        const auto expected_points
            = std::vector<Point>({{Beat(4.0), 100, false, false}});

        REQUIRE(points == expected_points);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Hold notes", "Hold")
{
    SECTION("Hold note points depend on resolution")
    {
        const auto track = NoteTrack({{768, 15}}, {}, {});
        const auto first_points = ProcessedTrack(track, {}, {}).points();
        const auto first_expected_points
            = std::vector<Point>({{Beat(4.0), 50, false, false},
                                  {Beat(775.0 / 192.0), 1, true, false},
                                  {Beat(782.0 / 192.0), 1, true, false},
                                  {Beat(789.0 / 192.0), 1, true, false}});
        const auto header = SongHeader(0.F, 200);
        const auto second_points = ProcessedTrack(track, header, {}).points();
        const auto second_expected_points
            = std::vector<Point>({{Beat(768.0 / 200.0), 50, false, false},
                                  {Beat(776.0 / 200.0), 1, true, false},
                                  {Beat(784.0 / 200.0), 1, true, false}});

        REQUIRE(first_points == first_expected_points);
        REQUIRE(second_points == second_expected_points);
    }

    SECTION("Hold note points and chords")
    {
        const auto track = NoteTrack(
            {{768, 7, NoteColour::Green}, {768, 8, NoteColour::Red}}, {}, {});
        const auto points = ProcessedTrack(track, {}, {}).points();
        const auto expected_points
            = std::vector<Point>({{Beat(4.0), 100, false, false},
                                  {Beat(775.0 / 192.0), 1, true, false},
                                  {Beat(782.0 / 192.0), 1, true, false}});

        REQUIRE(points == expected_points);
    }

    SECTION("Resolutions below 25 do not enter an infinite loop")
    {
        const auto track = NoteTrack({{768, 2}}, {}, {});
        const auto header = SongHeader(0.F, 1);
        const auto points = ProcessedTrack(track, header, {}).points();
        const auto expected_points
            = std::vector<Point>({{Beat(768.0), 50, false, false},
                                  {Beat(769.0), 1, true, false},
                                  {Beat(770.0), 1, true, false}});

        REQUIRE(points == expected_points);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Points are sorted", "Sorted")
{
    const auto track = NoteTrack({{768, 15}, {770, 0}}, {}, {});
    const auto points = ProcessedTrack(track, {}, {}).points();
    const auto expected_points
        = std::vector<Point>({{Beat(4.0), 50, false, false},
                              {Beat(770.0 / 192.0), 50, false, false},
                              {Beat(775.0 / 192.0), 1, true, false},
                              {Beat(782.0 / 192.0), 1, true, false},
                              {Beat(789.0 / 192.0), 1, true, false}});

    REQUIRE(points == expected_points);
}

// Last checked: 24.0.1555-master
TEST_CASE("End of SP phrase points", "End of SP")
{
    const auto track = NoteTrack({{768}, {960}, {1152}},
                                 {{768, 1}, {900, 50}, {1100, 53}}, {});
    const auto points = ProcessedTrack(track, {}, {}).points();
    const auto expected_points
        = std::vector<Point>({{Beat(4.0), 50, false, true},
                              {Beat(5.0), 50, false, false},
                              {Beat(6.0), 50, false, true}});

    REQUIRE(points == expected_points);
}

// Last checked: 24.0.1555-master
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
        std::vector<Point> expected_points;
        expected_points.reserve(50);
        for (auto i = 0U; i < 50U; ++i) {
            const auto mult = 1U + std::min((i + 1) / 10, 3U);
            expected_points.push_back({Beat(i), 50 * mult, false, false});
        }

        REQUIRE(ProcessedTrack(track, {}, {}).points() == expected_points);
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
        const auto points = ProcessedTrack(track, {}, {}).points();

        REQUIRE(points.back().value == 4);
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
        const auto points = ProcessedTrack(track, {}, {}).points();

        REQUIRE(points.back().value == 2);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("front_end and back_end work correctly", "Timing window")
{
    const auto converter
        = TimeConverter(SyncTrack({}, {{0, 150000}, {768, 200000}}), {});

    SECTION("Front ends for notes are correct")
    {
        REQUIRE(front_end({Beat(1.0), 50, false, false}, converter).value()
                == Approx(0.825));
        REQUIRE(front_end({Beat(4.1), 50, false, false}, converter).value()
                == Approx(3.9));
    }

    SECTION("Back ends for notes are correct")
    {
        REQUIRE(back_end({Beat(1.0), 50, false, false}, converter).value()
                == Approx(1.175));
        REQUIRE(back_end({Beat(3.9), 50, false, false}, converter).value()
                == Approx(4.1));
    }

    SECTION("Front and back ends for hold points are correct")
    {
        REQUIRE(front_end({Beat(4.1), 50, true, false}, converter).value()
                == Approx(4.1));
        REQUIRE(back_end({Beat(3.9), 50, true, false}, converter).value()
                == Approx(3.9));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("propagate_sp_over_whammy works correctly", "Whammy SP")
{
    std::vector<Note> notes {{0, 1920}, {2112, 576}, {3000}};
    std::vector<StarPower> phrases {{0, 3000}};
    NoteTrack note_track(notes, phrases, {});

    SECTION("Works correctly over 4/4")
    {
        std::vector<TimeSignature> time_sigs {{0, 4, 4}};
        ProcessedTrack track(note_track, {}, {time_sigs, {}});

        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(1.0),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.508333));
        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(1.0), Beat(4.0),
                                              Measure(0.25), Measure(1.0),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.50625));
    }

    SECTION("Works correctly over 3/4")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}};
        ProcessedTrack track(note_track, {}, {time_sigs, {}});

        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(4.0 / 3),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.466667));
        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(-1.0), Beat(4.0),
                                              Measure(-0.25), Measure(4.0 / 3),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.435417));
    }

    SECTION("Works correctly over changing time signatures")
    {
        std::vector<TimeSignature> time_sigs {{0, 4, 4}, {384, 3, 4}};
        ProcessedTrack track(note_track, {}, {time_sigs, {}});

        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(7.0 / 6),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.4875));
        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(1.0), Beat(4.0),
                                              Measure(0.25), Measure(7.0 / 6),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.485417));
    }

    SECTION("Returns -1 if SP runs out")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}, {384, 4, 4}};
        ProcessedTrack track(note_track, {}, {time_sigs, {}});

        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(2.0),
                                              Measure(0.0), Measure(2.0 / 3),
                                              {0.015, 0.015})
                    .max()
                == Approx(-1.0));
        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(10.0),
                                              Measure(0.0), Measure(8.0 / 3),
                                              {0.015, 0.015})
                    .max()
                == Approx(-1.0));
    }

    SECTION("Works even if some of the range isn't whammyable")
    {
        ProcessedTrack track(note_track, {}, {});

        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(12.0),
                                              Measure(0.0), Measure(3.0),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.491667));
    }

    SECTION("SP bar does not exceed full bar")
    {
        ProcessedTrack track(note_track, {}, {});

        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(10.0),
                                              Measure(0.0), Measure(2.5),
                                              {1.0, 1.0})
                    .max()
                == Approx(1.0));
        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(10.5),
                                              Measure(0.0), Measure(2.625),
                                              {1.0, 1.0})
                    .max()
                == Approx(0.984375));
    }

    SECTION("Hold notes not in a phrase do not contribute SP")
    {
        NoteTrack no_sp_note_track(notes, {}, {});
        ProcessedTrack track(no_sp_note_track, {}, {});

        REQUIRE(track
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(1.0),
                                              {1.0, 1.0})
                    .max()
                == Approx(0.875));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("is_activation_valid works with no whammy", "Valid no whammy acts")
{
    std::vector<Note> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack note_track(notes, {}, {});
    ProcessedTrack track(note_track, {}, {});
    const auto& points = track.points();
    ActivationCandidate candidate {
        points.cbegin(), points.cbegin() + 3, Beat(0.0), {1.0, 1.0}};
    ProcessedTrack second_track(note_track, {}, SyncTrack({{0, 3, 4}}, {}));
    const auto& second_points = second_track.points();
    ActivationCandidate second_candidate {second_points.cbegin(),
                                          second_points.cbegin() + 3,
                                          Beat(0.0),
                                          {1.0, 1.0}};

    SECTION("Full bar works with time signatures")
    {
        REQUIRE(track.is_candidate_valid(candidate));
        REQUIRE(!second_track.is_candidate_valid(second_candidate));
    }

    SECTION("Half bar works with time signatures")
    {
        candidate.act_end = points.cbegin() + 2;
        candidate.sp_bar = {0.5, 0.5};
        second_candidate.act_end = second_points.cbegin() + 2;
        second_candidate.sp_bar = {0.5, 0.5};

        REQUIRE(track.is_candidate_valid(candidate));
        REQUIRE(!second_track.is_candidate_valid(second_candidate));
    }

    SECTION("Below half bar never works")
    {
        candidate.act_end = points.cbegin() + 1;
        candidate.sp_bar.max() = 0.25;

        REQUIRE(!track.is_candidate_valid(candidate));
    }

    SECTION("Check next point needs to not lie in activation")
    {
        candidate.act_end = points.cbegin() + 1;
        candidate.sp_bar.max() = 0.6;

        REQUIRE(!track.is_candidate_valid(candidate));
    }

    SECTION("Check intermediate SP is accounted for")
    {
        std::vector<StarPower> phrases {{3000, 100}};
        NoteTrack overlap_notes(notes, phrases, {});
        ProcessedTrack overlap_track(overlap_notes, {}, {});
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 3,
                                               Beat(0.0),
                                               {0.8, 0.8}};

        REQUIRE(overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("Check only reached intermediate SP is accounted for")
    {
        notes[2].position = 6000;
        std::vector<StarPower> phrases {{6000, 100}};
        NoteTrack overlap_notes(notes, phrases, {});
        ProcessedTrack overlap_track(overlap_notes, {}, {});
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 3,
                                               Beat(0.0),
                                               {0.8, 0.8}};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("Last note's SP status is not ignored")
    {
        notes[3].position = 4000;
        std::vector<StarPower> phrases {{3072, 100}};
        NoteTrack overlap_notes(notes, phrases, {});
        ProcessedTrack overlap_track(overlap_notes, {}, {});
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 2,
                                               Beat(0.0),
                                               {0.5, 0.5}};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("SP bar does not exceed full bar")
    {
        std::vector<Note> overlap_notes {{0}, {2}, {7000}};
        std::vector<StarPower> phrases {{0, 1}, {2, 1}};
        NoteTrack overlap_note_track(overlap_notes, phrases, {});
        ProcessedTrack overlap_track(overlap_note_track, {}, {});
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 2,
                                               Beat(0.0),
                                               {1.0, 1.0}};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("Earliest activation point is considered")
    {
        candidate.act_end = points.cbegin() + 1;
        candidate.sp_bar = {0.53125, 0.53125};
        candidate.earliest_activation_point = Beat(-2.0);

        REQUIRE(track.is_candidate_valid(candidate));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("is_activation_valid works with whammy", "Valid whammy acts")
{
    std::vector<Note> notes {{0, 960}, {3840}, {6144}};
    std::vector<StarPower> phrases {{0, 7000}};
    NoteTrack note_track(notes, phrases, {});
    ProcessedTrack track(note_track, {}, {});
    const auto& points = track.points();
    ActivationCandidate candidate {
        points.cbegin(), points.cend() - 2, Beat(0.0), {0.5, 0.5}};

    SECTION("Check whammy is counted")
    {
        REQUIRE(track.is_candidate_valid(candidate));
    }

    SECTION("Check compressed activations are counted")
    {
        candidate.sp_bar.max() = 0.9;
        REQUIRE(track.is_candidate_valid(candidate));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("is_activation_valid takes into account minimum SP", "Min SP")
{
    std::vector<Note> notes {{0}, {1536}, {2304}, {3072}, {4608}};
    NoteTrack note_track(notes, {}, {});
    ProcessedTrack track(note_track, {}, {});
    const auto& points = track.points();
    ActivationCandidate candidate {
        points.cbegin(), points.cbegin() + 3, Beat(0.0), {0.5, 1.0}};

    SECTION("Lower SP is considered")
    {
        REQUIRE(track.is_candidate_valid(candidate));
    }

    SECTION("Lower SP is only considered down to a half-bar")
    {
        candidate.act_end = points.cbegin() + 1;
        candidate.sp_bar.min() = 0.25;

        REQUIRE(!track.is_candidate_valid(candidate));
    }
}

TEST_CASE("total_available_sp counts SP correctly", "Available SP")
{
    std::vector<Note> notes {{0},        {192},  {384},  {576},
                             {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack note_track(notes, phrases, {});
    ProcessedTrack track(note_track, {}, {});
    const auto& points = track.points();

    SECTION("Phrases are counted correctly")
    {
        REQUIRE(track.total_available_sp(Beat(0.0), points.cbegin() + 1)
                == SpBar {0.25, 0.25});
        REQUIRE(track.total_available_sp(Beat(0.0), points.cbegin() + 2)
                == SpBar {0.25, 0.25});
        REQUIRE(track.total_available_sp(Beat(0.5), points.cbegin() + 3)
                == SpBar {0.25, 0.25});
    }

    SECTION("Whammy is counted correctly")
    {
        auto result = track.total_available_sp(Beat(4.0), points.cbegin() + 5);
        REQUIRE(result.min() == Approx(0.0));
        REQUIRE(result.max() == Approx(0.00121528));
    }

    SECTION("Whammy is counted correctly even started mid hold")
    {
        auto result = track.total_available_sp(Beat(4.5), points.cend() - 3);
        REQUIRE(result.min() == Approx(0.0));
        REQUIRE(result.max() == Approx(0.0166667));
    }

    SECTION("SP does not exceed full bar")
    {
        REQUIRE(track.total_available_sp(Beat(0.0), points.cend() - 1)
                == SpBar {1.0, 1.0});
    }
}

TEST_CASE("optimal_path produces the correct path")
{
    SECTION("Simplest song with a non-empty path")
    {
        std::vector<Note> notes {{0}, {192}, {384}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack note_track(notes, phrases, {});
        ProcessedTrack track(note_track, {}, {});
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 2, points.cbegin() + 2}};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 50);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song with multiple acts")
    {
        std::vector<Note> notes {{0},
                                 {192},
                                 {384},
                                 {384, 0, NoteColour::Red},
                                 {384, 0, NoteColour::Yellow},
                                 {3840},
                                 {4032},
                                 {10368},
                                 {10368, 0, NoteColour::Red},
                                 {10368, 0, NoteColour::Yellow}};
        std::vector<StarPower> phrases {
            {0, 50}, {192, 50}, {3840, 50}, {4032, 50}};
        NoteTrack note_track(notes, phrases, {});
        ProcessedTrack track(note_track, {}, {});
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 2, points.cbegin() + 2},
            {points.cbegin() + 5, points.cbegin() + 5}};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 300);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song with an act containing more than one note")
    {
        std::vector<Note> notes {{0}, {192}, {384}, {576}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack note_track(notes, phrases, {});
        ProcessedTrack track(note_track, {}, {});
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 2, points.cbegin() + 3}};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 100);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song with an act that must go as long as possible")
    {
        std::vector<Note> notes {{0}, {192}, {384}, {3360}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack note_track(notes, phrases, {});
        ProcessedTrack track(note_track, {}, {});
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 2, points.cbegin() + 3}};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 100);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song where greedy algorithm fails")
    {
        std::vector<Note> notes {
            {0}, {192}, {384}, {3840}, {3840, 0, NoteColour::Red}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack note_track(notes, phrases, {});
        ProcessedTrack track(note_track, {}, {});
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 3, points.cbegin() + 3}};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 100);
        REQUIRE(opt_path.activations == optimal_acts);
    }
}
