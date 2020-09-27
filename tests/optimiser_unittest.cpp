/*
 * CHOpt - Star Power optimiser for Clone Hero
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

#include "catch.hpp"

#include "optimiser.hpp"

static bool operator==(const Beat& lhs, const Beat& rhs)
{
    return lhs.value() == Approx(rhs.value()).margin(0.01);
}

static bool operator==(const Activation& lhs, const Activation& rhs)
{
    return std::tie(lhs.act_start, lhs.act_end, lhs.sp_start, lhs.sp_end)
        == std::tie(rhs.act_start, rhs.act_end, rhs.sp_start, rhs.sp_end);
}

TEST_CASE("path_summary produces the correct output", "Path summary")
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {576}, {6144}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {384, 50}, {6144, 50}};
    std::vector<Solo> solos {{0, 50, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, solos};
    ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
    const auto& points = track.points();

    SECTION("Overlap and ES are denoted correctly")
    {
        Path path {{{points.cbegin() + 2, points.cbegin() + 3, Beat {0.0},
                     Beat {0.0}}},
                   100};

        const char* desired_path_output
            = "Path: 2(+1)-ES1\n"
              "No SP score: 350\n"
              "Total score: 450\n"
              "Activation 1: Measure 1.5 to Measure 1.75";

        REQUIRE(track.path_summary(path) == desired_path_output);
    }

    SECTION("No overlap is denoted correctly")
    {
        Path path {{{points.cbegin() + 3, points.cbegin() + 3, Beat {0.0},
                     Beat {0.0}}},
                   50};

        const char* desired_path_output
            = "Path: 3-ES1\n"
              "No SP score: 350\n"
              "Total score: 400\n"
              "Activation 1: Measure 1.75 to Measure 1.75";

        REQUIRE(track.path_summary(path) == desired_path_output);
    }

    SECTION("No ES is denoted correctly")
    {
        Path path {{{points.cbegin() + 4, points.cbegin() + 4, Beat {0.0},
                     Beat {0.0}}},
                   50};

        const char* desired_path_output
            = "Path: 3(+1)\n"
              "No SP score: 350\n"
              "Total score: 400\n"
              "Activation 1: Measure 9 to Measure 9";

        REQUIRE(track.path_summary(path) == desired_path_output);
    }

    SECTION("No SP is denoted correctly")
    {
        Path path {{}, 0};
        NoteTrack<NoteColour> second_note_track {notes, {}, solos};
        ProcessedSong second_track {second_note_track, 192, {}, 1.0, 1.0,
                                    Second(0.0)};

        const char* desired_path_output = "Path: None\n"
                                          "No SP score: 350\n"
                                          "Total score: 350";

        REQUIRE(second_track.path_summary(path) == desired_path_output);
    }
}

TEST_CASE("optimal_path produces the correct path")
{
    std::atomic<bool> terminate {false};

    SECTION("Simplest song with a non-empty path")
    {
        std::vector<Note<NoteColour>> notes {{0}, {192}, {384}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {{points.cbegin() + 2,
                                               points.cbegin() + 2, Beat {0.0},
                                               Beat {2.0}, Beat {18.0}}};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 50);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song with multiple acts")
    {
        std::vector<Note<NoteColour>> notes {{0},
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
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 2, points.cbegin() + 2, Beat {0.0}, Beat {2.0},
             Beat {18.0}},
            {points.cbegin() + 5, points.cbegin() + 5, Beat {0.0}, Beat {54.0},
             Beat {70.0}}};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 300);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song with an act containing more than one note")
    {
        std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {576}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {{points.cbegin() + 2,
                                               points.cbegin() + 3, Beat {0.0},
                                               Beat {2.0}, Beat {18.0}}};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 100);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song with an act that must go as long as possible")
    {
        std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {3360}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {{points.cbegin() + 2,
                                               points.cbegin() + 3, Beat {0.0},
                                               Beat {2.0}, Beat {18.0}}};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 100);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song where greedy algorithm fails")
    {
        std::vector<Note<NoteColour>> notes {
            {0}, {192}, {384}, {3840}, {3840, 0, NoteColour::Red}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {{points.cbegin() + 3,
                                               points.cbegin() + 3, Beat {0.0},
                                               Beat {20.0}, Beat {36.0}}};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 100);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    SECTION("Simplest song where a phrase must be hit early")
    {
        std::vector<Note<NoteColour>> notes {{0},    {192},   {384},  {3224},
                                             {9378}, {15714}, {15715}};
        std::vector<StarPower> phrases {
            {0, 50}, {192, 50}, {3224, 50}, {9378, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        const auto& points = track.points();
        std::vector<Activation> optimal_acts {
            {points.cbegin() + 2, points.cbegin() + 2, Beat {0.0},
             Beat {0.8958}, Beat {16.8958}},
            {points.cbegin() + 5, points.cbegin() + 6, Beat {0.0},
             Beat {81.84375}, Beat {97.84375}}};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 150);
        REQUIRE(opt_path.activations == optimal_acts);
    }

    // Naively the ideal path would be 2-1, but we have to squeeze the last SP
    // phrase early for the 2 to work and this makes the 1 impossible. So the
    // optimal path is really 3.
    SECTION("Simplest song where activations ending late matter")
    {
        std::vector<Note<NoteColour>> notes {
            {0},     {192},   {384},   {3234, 1440}, {10944}, {10945}, {10946},
            {10947}, {10948}, {10949}, {10950},      {10951}, {10952}, {10953}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}, {3234, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 750);
        REQUIRE(opt_path.activations.size() == 1);
    }

    // There was a bug where sustains at the start of an SP phrase right after
    // an activation/start of song had their early whammy discounted, if that
    // note didn't also grant SP. This affected a squeeze in GH3 Cult of
    // Personality. This test is to catch that.
    SECTION("Early whammy at start of an SP phrase is always counted")
    {
        std::vector<Note<NoteColour>> notes {{0, 1420}, {1500}, {1600}};
        std::vector<StarPower> phrases {{0, 1550}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 50);
        REQUIRE(opt_path.activations.size() == 1);
    }

    // There was a bug where an activation on a note right after an SP sustain
    // could double count the whammy available between the burst at the end of
    // the sustain and the note. This affected a squeeze in Epidox, making chopt
    // think you could squeeze from the O right before Robotic Buildup to a B in
    // the next section.
    SECTION("Whammy just before the activation point is not double counted")
    {
        std::vector<Note<NoteColour>> notes {{192, 1440}, {1632}, {6336}};
        std::vector<StarPower> phrases {{192, 1}, {1632, 1}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost < 100);
    }

    // There was a bug where an activation on a note right after an SP sustain
    // could be drawn starting right after the tick burst, rather than the
    // proper place. An example is the last activation of Gamer National Anthem
    // in CSC August 2020.
    SECTION("Activation right after a SP sustain is drawn correctly")
    {
        std::vector<Note<NoteColour>> notes {{0, 1488}, {2880, 3264}};
        std::vector<StarPower> phrases {{0, 1}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.activations[0].sp_start >= Beat(15.0));
    }

    SECTION("Songs ending in ES1 are pathed correctly")
    {
        std::vector<Note<NoteColour>> notes {{0},   {192},  {384}, {576},
                                             {768}, {4032}, {4224}};
        std::vector<StarPower> phrases {{0, 50}, {192, 50}, {4032, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};
        auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 150);
        REQUIRE(opt_path.activations.size() == 1);
    }

    SECTION("Compressed whammy is specified correctly")
    {
        std::vector<Note<NoteColour>> notes {{192, 192},
                                             {672},
                                             {1000},
                                             {1000, 0, NoteColour::Red},
                                             {1000, 0, NoteColour::Yellow},
                                             {3840},
                                             {9984},
                                             {10176},
                                             {10176, 0, NoteColour::Red},
                                             {10176, 0, NoteColour::Yellow}};
        std::vector<StarPower> phrases {
            {192, 50}, {672, 50}, {3840, 50}, {9984, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};

        auto opt_path = optimiser.optimal_path();
        auto act = opt_path.activations[0];

        REQUIRE(opt_path.score_boost == 300);
        REQUIRE(opt_path.activations.size() == 2);
        REQUIRE(act.whammy_end > Beat {1.06});
        REQUIRE(act.whammy_end < Beat {1.74});
        REQUIRE(act.sp_start < Beat {3.6});
    }

    SECTION("Acts covering the last note do not compress whammy")
    {
        std::vector<Note<NoteColour>> notes {{0, 1536}, {1728}};
        std::vector<StarPower> phrases {{0, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};

        auto opt_path = optimiser.optimal_path();
        auto act = opt_path.activations[0];

        REQUIRE(act.whammy_end > Beat {16.0});
    }

    SECTION("Use next point to work out compressed whammy")
    {
        std::vector<Note<NoteColour>> notes {{0},
                                             {192},
                                             {384},
                                             {384, 0, NoteColour::Red},
                                             {384, 0, NoteColour::Yellow},
                                             {3350},
                                             {3360},
                                             {9504},
                                             {9696},
                                             {9696, 0, NoteColour::Red},
                                             {9696, 0, NoteColour::Yellow}};
        std::vector<StarPower> phrases {
            {0, 50}, {192, 50}, {3350, 50}, {9504, 50}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};

        const auto opt_path = optimiser.optimal_path();
        const auto act = opt_path.activations[0];

        REQUIRE(act.whammy_end > Beat {17.45});
    }

    // There was a bug where an activation after an SP sustain that comes after
    // an act with a forbidden squeeze would be shown to have ticks possible on
    // the forbidden squeeze even if ticks were not possible. An example is
    // given by the path for EON BREAK in CSC December 2019.
    SECTION("Forbidden squeeze does not grant extra whammy next act")
    {
        std::vector<Note<NoteColour>> notes {{0},         {192},  {768},
                                             {3840, 192}, {4224}, {19200, 192},
                                             {38400},     {41990}};
        std::vector<StarPower> phrases {
            {0, 1}, {192, 1}, {3840, 576}, {19200, 1}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};

        const auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.score_boost == 200);
    }

    // This isn't terribly well-defined. The heuristic is to still do a greedy
    // approach but to pick the easiest activation at any point given a tie. The
    // test is just enough to spot a difference between that and simple greedy.
    SECTION("Easier activations are chosen where possible")
    {
        std::vector<Note<NoteColour>> notes {{0},    {192},  {384},
                                             {3504}, {9600}, {12672}};
        std::vector<StarPower> phrases {{0, 1}, {192, 1}};
        NoteTrack<NoteColour> note_track {notes, phrases, {}};
        ProcessedSong track {note_track, 192, {}, 1.0, 1.0, Second(0.0)};
        Optimiser optimiser {&track, &terminate};

        const auto opt_path = optimiser.optimal_path();

        REQUIRE(opt_path.activations[0].sp_start > Beat {20.0});
    }
}
