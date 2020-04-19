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
#include <iterator>
#include <tuple>

#include "catch.hpp"

#include "optimiser.hpp"
#include "points.hpp"

static bool operator==(const Activation& lhs, const Activation& rhs)
{
    return std::tie(lhs.act_start, lhs.act_end)
        == std::tie(rhs.act_start, rhs.act_end);
}

TEST_CASE("is_candidate_valid works with no whammy", "Valid no whammy acts")
{
    std::vector<Note> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack note_track {notes, {}, {}};
    ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {Beat(0.0), Measure(0.0)},
                                   {1.0, 1.0}};
    ProcessedTrack second_track {note_track, 192, SyncTrack({{0, 3, 4}}, {}),
                                 1.0, 1.0};
    const auto& second_points = second_track.points();
    ActivationCandidate second_candidate {second_points.cbegin(),
                                          second_points.cbegin() + 3,
                                          {Beat(0.0), Measure(0.0)},
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
        NoteTrack overlap_notes {notes, phrases, {}};
        ProcessedTrack overlap_track {overlap_notes, 192, {}, 1.0, 1.0};
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 3,
                                               {Beat(0.0), Measure(0.0)},
                                               {0.8, 0.8}};

        REQUIRE(overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("Check only reached intermediate SP is accounted for")
    {
        notes[2].position = 6000;
        std::vector<StarPower> phrases {{6000, 100}};
        NoteTrack overlap_notes {notes, phrases, {}};
        ProcessedTrack overlap_track {overlap_notes, 192, {}, 1.0, 1.0};
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 3,
                                               {Beat(0.0), Measure(0.0)},
                                               {0.8, 0.8}};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("Last note's SP status is not ignored")
    {
        notes[3].position = 4000;
        std::vector<StarPower> phrases {{3072, 100}};
        NoteTrack overlap_notes {notes, phrases, {}};
        ProcessedTrack overlap_track {overlap_notes, 192, {}, 1.0, 1.0};
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 2,
                                               {Beat(0.0), Measure(0.0)},
                                               {0.5, 0.5}};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("SP bar does not exceed full bar")
    {
        std::vector<Note> overlap_notes {{0}, {2}, {7000}};
        std::vector<StarPower> phrases {{0, 1}, {2, 1}};
        NoteTrack overlap_note_track(overlap_notes, phrases, {});
        ProcessedTrack overlap_track(overlap_note_track, 192, SyncTrack(), 1.0,
                                     1.0);
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {overlap_points.cbegin(),
                                               overlap_points.cbegin() + 2,
                                               {Beat(0.0), Measure(0.0)},
                                               {1.0, 1.0}};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("Earliest activation point is considered")
    {
        candidate.act_end = points.cbegin() + 1;
        candidate.sp_bar = {0.53125, 0.53125};
        candidate.earliest_activation_point = {Beat(-2.0), Measure(-0.5)};

        REQUIRE(track.is_candidate_valid(candidate));
    }
}

TEST_CASE("is_candidate_valid works with whammy", "Valid whammy acts")
{
    std::vector<Note> notes {{0, 960}, {3840}, {6144}};
    std::vector<StarPower> phrases {{0, 7000}};
    NoteTrack note_track {notes, phrases, {}};
    ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

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

TEST_CASE("is_candidate_valid takes into account minimum SP", "Min SP")
{
    std::vector<Note> notes {{0}, {1536}, {2304}, {3072}, {4608}};
    NoteTrack note_track {notes, {}, {}};
    ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 1.0}};

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

TEST_CASE("is_candidate_valid takes into account squeezing", "Valid squeezes")
{
    SECTION("Front end and back end of the activation endpoints are considered")
    {
        std::vector<Note> notes {{0}, {3110}};
        NoteTrack note_track {notes, {}, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
        const auto& points = track.points();
        ActivationCandidate candidate {points.cbegin(),
                                       points.cbegin() + 1,
                                       {Beat(0.0), Measure(0.0)},
                                       {0.5, 0.5}};

        REQUIRE(track.is_candidate_valid(candidate));
    }

    SECTION("Next note can be squeezed late to avoid going too far")
    {
        std::vector<Note> notes {{0}, {3034}, {3053}};
        NoteTrack note_track {notes, {}, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
        const auto& points = track.points();
        ActivationCandidate candidate {points.cbegin(),
                                       points.cbegin() + 1,
                                       {Beat(0.0), Measure(0.0)},
                                       {0.5, 0.5}};

        REQUIRE(track.is_candidate_valid(candidate));
    }

    SECTION("Intermediate SP can be hit early")
    {
        std::vector<Note> notes {{0}, {3102}, {4608}};
        std::vector<StarPower> phrases {{3100, 100}};
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
        const auto& points = track.points();
        ActivationCandidate candidate {points.cbegin(),
                                       points.cbegin() + 2,
                                       {Beat(0.0), Measure(0.0)},
                                       {0.5, 0.5}};

        REQUIRE(track.is_candidate_valid(candidate));
    }

    SECTION("Intermediate SP can be hit late")
    {
        std::vector<Note> notes {{0}, {768}, {6942}};
        std::vector<StarPower> phrases {{768, 100}};
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
        const auto& points = track.points();
        ActivationCandidate candidate {points.cbegin(),
                                       points.cbegin() + 2,
                                       {Beat(0.0), Measure(0.0)},
                                       {1.0, 1.0}};

        REQUIRE(track.is_candidate_valid(candidate));
    }
}

TEST_CASE("total_available_sp counts SP correctly", "Available SP")
{
    std::vector<Note> notes {{0},        {192},  {384},  {576},
                             {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack note_track {notes, phrases, {}};
    ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
    const auto& points = track.points();

    SECTION("Phrases are counted correctly")
    {
        REQUIRE(track.total_available_sp(Beat(0.0), points.cbegin(),
                                         points.cbegin() + 1)
                == SpBar {0.25, 0.25});
        REQUIRE(track.total_available_sp(Beat(0.0), points.cbegin(),
                                         points.cbegin() + 2)
                == SpBar {0.25, 0.25});
        REQUIRE(track.total_available_sp(Beat(0.5), points.cbegin() + 2,
                                         points.cbegin() + 3)
                == SpBar {0.25, 0.25});
    }

    SECTION("Whammy is counted correctly")
    {
        auto result = track.total_available_sp(Beat(4.0), points.cbegin() + 4,
                                               points.cbegin() + 5);
        REQUIRE(result.min() == Approx(0.0));
        REQUIRE(result.max() == Approx(0.00121528));
    }

    SECTION("Whammy is counted correctly even started mid hold")
    {
        auto result = track.total_available_sp(Beat(4.5), points.cend() - 3,
                                               points.cend() - 3);
        REQUIRE(result.min() == Approx(0.0));
        REQUIRE(result.max() == Approx(0.0166667));
    }

    SECTION("SP does not exceed full bar")
    {
        REQUIRE(track.total_available_sp(Beat(0.0), points.cbegin(),
                                         points.cend() - 1)
                == SpBar {1.0, 1.0});
    }

    SECTION("SP notes are counted from first_point even if start is passed its "
            "middle")
    {
        REQUIRE(track.total_available_sp(Beat(0.05), points.cbegin(),
                                         points.cbegin() + 1)
                == SpBar {0.25, 0.25});
    }
}

TEST_CASE("optimal_path produces the correct path")
{
    SECTION("Simplest song with a non-empty path")
    {
        std::vector<Note> notes {{0}, {192}, {384}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
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
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
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
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
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
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
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
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 3, points.cbegin() + 3}};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 100);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song where a phrase must be hit early")
    {
        std::vector<Note> notes {{0},    {192},   {384},  {3224},
                                 {9378}, {15714}, {15715}};
        std::vector<StarPower> phrases {
            {0, 50}, {192, 50}, {3224, 50}, {9378, 50}};
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 2, points.cbegin() + 2},
            {points.cbegin() + 5, points.cbegin() + 6}};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 150);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    // Naively the ideal path would be 2-1, but we have to squeeze the last SP
    // phrase early for the 2 to work and this makes the 1 impossible. So the
    // optimal path is really 3.
    SECTION("Simplest song where activations ending late matter")
    {
        std::vector<Note> notes {
            {0},     {192},   {384},   {3234, 1440}, {10944}, {10945}, {10946},
            {10947}, {10948}, {10949}, {10950},      {10951}, {10952}, {10953}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}, {3234, 50}};
        NoteTrack note_track {notes, phrases, {}};
        ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
        auto opt_path = track.optimal_path();

        REQUIRE(opt_path.score_boost == 750);
        REQUIRE(opt_path.activations.size() == 1);
    }
}

TEST_CASE("path_summary produces the correct output", "Path summary")
{
    std::vector<Note> notes {{0}, {192}, {384}, {576}, {6144}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {384, 50}, {6144, 50}};
    std::vector<Solo> solos {{0, 50, 100}};
    NoteTrack note_track {notes, phrases, solos};
    ProcessedTrack track {note_track, 192, {}, 1.0, 1.0};
    const auto& points = track.points();

    SECTION("Overlap and ES are denoted correctly")
    {
        const Path path {{{points.cbegin() + 2, points.cbegin() + 3}}, 100};

        const char* desired_path_output
            = "Path: 2(+1)-ES1\n"
              "No SP score: 350\n"
              "Total score: 450\n"
              "Activation 1: Measure 1.5 to Measure 1.75";

        REQUIRE(track.path_summary(path) == desired_path_output);
    }

    SECTION("No overlap is denoted correctly")
    {
        const Path path {{{points.cbegin() + 3, points.cbegin() + 3}}, 50};

        const char* desired_path_output
            = "Path: 3-ES1\n"
              "No SP score: 350\n"
              "Total score: 400\n"
              "Activation 1: Measure 1.75 to Measure 1.75";

        REQUIRE(track.path_summary(path) == desired_path_output);
    }

    SECTION("No ES is denoted correctly")
    {
        const Path path {{{points.cbegin() + 4, points.cbegin() + 4}}, 50};

        const char* desired_path_output
            = "Path: 3(+1)\n"
              "No SP score: 350\n"
              "Total score: 400\n"
              "Activation 1: Measure 9 to Measure 9";

        REQUIRE(track.path_summary(path) == desired_path_output);
    }

    SECTION("No SP is denoted correctly")
    {
        const Path path {{}, 0};
        NoteTrack second_note_track {notes, {}, solos};
        ProcessedTrack second_track {second_note_track, 192, {}, 1.0, 1.0};

        const char* desired_path_output = "Path: None\n"
                                          "No SP score: 350\n"
                                          "Total score: 350";

        REQUIRE(second_track.path_summary(path) == desired_path_output);
    }
}
