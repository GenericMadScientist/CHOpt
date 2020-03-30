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

#include <array>

#include "catch.hpp"

#include "chart.hpp"
#include "optimiser.hpp"

// Last checked: 24.0.1555-master
TEST_CASE("Beats to seconds conversion", "Beats<->S")
{
    const auto track = SyncTrack({{0, 4, 4}}, {{0, 150000}, {800, 200000}});
    const auto header = SongHeader(0.F, 200);
    const auto converter = TimeConverter(track, header);
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.0};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 1.9};

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.beats_to_seconds(beats.at(i))
                == Approx(seconds.at(i)));
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.seconds_to_beats(seconds.at(i))
                == Approx(beats.at(i)));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Beats to measures conversion", "Beats<->Measures")
{
    const auto track = SyncTrack({{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}}, {});
    const auto header = SongHeader(0.F, 200);
    const auto converter = TimeConverter(track, header);
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.5, 6.5};
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.beats_to_measures(beats.at(i))
                == Approx(measures.at(i)));
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.measures_to_beats(measures.at(i))
                == Approx(beats.at(i)));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Measures to seconds conversion", "Measures<->S")
{
    const auto track = SyncTrack({{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}},
                                 {{0, 150000}, {800, 200000}});
    const auto header = SongHeader(0.F, 200);
    const auto converter = TimeConverter(track, header);
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 2.05, 2.35};

    for (auto i = 0U; i < measures.size(); ++i) {
        REQUIRE(converter.measures_to_seconds(measures.at(i))
                == Approx(seconds.at(i)));
    }

    for (auto i = 0U; i < measures.size(); ++i) {
        REQUIRE(converter.seconds_to_measures(seconds.at(i))
                == Approx(measures.at(i)));
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
            {{4.0, 50, false, false}, {5.0, 50, false, false}});

        REQUIRE(points == expected_points);
    }

    SECTION("Chords give multiples of 50 points")
    {
        const auto track = NoteTrack(
            {{768, 0, NoteColour::Green}, {768, 0, NoteColour::Red}}, {}, {});
        const auto points = ProcessedTrack(track, {}, {}).points();
        const auto expected_points
            = std::vector<Point>({{4.0, 100, false, false}});

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
            = std::vector<Point>({{4.0, 50, false, false},
                                  {775.0 / 192.0, 1, true, false},
                                  {782.0 / 192.0, 1, true, false},
                                  {789.0 / 192.0, 1, true, false}});
        const auto header = SongHeader(0.F, 200);
        const auto second_points = ProcessedTrack(track, header, {}).points();
        const auto second_expected_points
            = std::vector<Point>({{768.0 / 200.0, 50, false, false},
                                  {776.0 / 200.0, 1, true, false},
                                  {784.0 / 200.0, 1, true, false}});

        REQUIRE(first_points == first_expected_points);
        REQUIRE(second_points == second_expected_points);
    }

    SECTION("Hold note points and chords")
    {
        const auto track = NoteTrack(
            {{768, 7, NoteColour::Green}, {768, 8, NoteColour::Red}}, {}, {});
        const auto points = ProcessedTrack(track, {}, {}).points();
        const auto expected_points
            = std::vector<Point>({{4.0, 100, false, false},
                                  {775.0 / 192.0, 1, true, false},
                                  {782.0 / 192.0, 1, true, false}});

        REQUIRE(points == expected_points);
    }

    SECTION("Resolutions below 25 do not enter an infinite loop")
    {
        const auto track = NoteTrack({{768, 2}}, {}, {});
        const auto header = SongHeader(0.F, 1);
        const auto points = ProcessedTrack(track, header, {}).points();
        const auto expected_points
            = std::vector<Point>({{768.0, 50, false, false},
                                  {769.0, 1, true, false},
                                  {770.0, 1, true, false}});

        REQUIRE(points == expected_points);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Points are sorted", "Sorted")
{
    const auto track = NoteTrack({{768, 15}, {770, 0}}, {}, {});
    const auto points = ProcessedTrack(track, {}, {}).points();
    const auto expected_points
        = std::vector<Point>({{4.0, 50, false, false},
                              {770.0 / 192.0, 50, false, false},
                              {775.0 / 192.0, 1, true, false},
                              {782.0 / 192.0, 1, true, false},
                              {789.0 / 192.0, 1, true, false}});

    REQUIRE(points == expected_points);
}

// Last checked: 24.0.1555-master
TEST_CASE("End of SP phrase points", "End of SP")
{
    const auto track = NoteTrack({{768}, {960}, {1152}},
                                 {{768, 1}, {900, 50}, {1100, 53}}, {});
    const auto points = ProcessedTrack(track, {}, {}).points();
    const auto expected_points = std::vector<Point>({{4.0, 50, false, true},
                                                     {5.0, 50, false, false},
                                                     {6.0, 50, false, true}});

    REQUIRE(points == expected_points);
}

// Last checked: 24.0.1555-master
TEST_CASE("front_end and back_end work correctly", "Timing window")
{
    const auto converter
        = TimeConverter(SyncTrack({}, {{0, 150000}, {768, 200000}}), {});

    SECTION("Front ends for notes are correct")
    {
        REQUIRE(front_end({1.0, 50, false, false}, converter) == Approx(0.825));
        REQUIRE(front_end({4.1, 50, false, false}, converter) == Approx(3.9));
    }

    SECTION("Back ends for notes are correct")
    {
        REQUIRE(back_end({1.0, 50, false, false}, converter) == Approx(1.175));
        REQUIRE(back_end({3.9, 50, false, false}, converter) == Approx(4.1));
    }

    SECTION("Front and back ends for hold points are correct")
    {
        REQUIRE(front_end({4.1, 50, true, false}, converter) == Approx(4.1));
        REQUIRE(back_end({3.9, 50, true, false}, converter) == Approx(3.9));
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

        REQUIRE(track.propagate_sp_over_whammy(0.0, 4.0, 0.5)
                == Approx(0.508333));
        REQUIRE(track.propagate_sp_over_whammy(1.0, 4.0, 0.5)
                == Approx(0.50625));
    }

    SECTION("Works correctly over 3/4")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}};
        ProcessedTrack track(note_track, {}, {time_sigs, {}});

        REQUIRE(track.propagate_sp_over_whammy(0.0, 4.0, 0.5)
                == Approx(0.466667));
        REQUIRE(track.propagate_sp_over_whammy(-1.0, 4.0, 0.5)
                == Approx(0.435417));
    }

    SECTION("Works correctly over changing time signatures")
    {
        std::vector<TimeSignature> time_sigs {{0, 4, 4}, {384, 3, 4}};
        ProcessedTrack track(note_track, {}, {time_sigs, {}});

        REQUIRE(track.propagate_sp_over_whammy(0.0, 4.0, 0.5)
                == Approx(0.4875));
        REQUIRE(track.propagate_sp_over_whammy(1.0, 4.0, 0.5)
                == Approx(0.485417));
    }

    SECTION("Returns -1 if SP runs out")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}, {384, 4, 4}};
        ProcessedTrack track(note_track, {}, {time_sigs, {}});

        REQUIRE(track.propagate_sp_over_whammy(0.0, 2.0, 0.015)
                == Approx(-1.0));
        REQUIRE(track.propagate_sp_over_whammy(0.0, 10.0, 0.015)
                == Approx(-1.0));
    }

    SECTION("Works even if some of the range isn't whammyable")
    {
        ProcessedTrack track(note_track, {}, {});

        REQUIRE(track.propagate_sp_over_whammy(0.0, 12.0, 0.5)
                == Approx(0.491667));
    }

    SECTION("SP bar does not exceed full bar")
    {
        ProcessedTrack track(note_track, {}, {});

        REQUIRE(track.propagate_sp_over_whammy(0.0, 10.0, 1.0) == Approx(1.0));
        REQUIRE(track.propagate_sp_over_whammy(0.0, 10.5, 1.0)
                == Approx(0.984375));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("is_activation_valid works with no whammy", "Valid no whammy acts")
{
    std::vector<Note> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack note_track(notes, {}, {});
    ProcessedTrack track(note_track, {}, {});
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(), points.cbegin() + 3, 0.0,
                                   1.0};
    ProcessedTrack second_track(note_track, {}, SyncTrack({{0, 3, 4}}, {}));
    const auto& second_points = second_track.points();
    ActivationCandidate second_candidate {second_points.cbegin(),
                                          second_points.cbegin() + 3, 0.0, 1.0};

    SECTION("Full bar works with time signatures")
    {
        REQUIRE(track.is_candidate_valid(candidate));
        REQUIRE(!second_track.is_candidate_valid(second_candidate));
    }

    SECTION("Half bar works with time signatures")
    {
        candidate.act_end = points.cbegin() + 2;
        candidate.sp_bar_amount = 0.5;
        second_candidate.act_end = second_points.cbegin() + 2;
        second_candidate.sp_bar_amount = 0.5;

        REQUIRE(track.is_candidate_valid(candidate));
        REQUIRE(!second_track.is_candidate_valid(second_candidate));
    }

    SECTION("Below half bar never works")
    {
        candidate.act_end = points.cbegin() + 1;
        candidate.sp_bar_amount = 0.25;

        REQUIRE(!track.is_candidate_valid(candidate));
    }

    SECTION("Check next point needs to not lie in activation")
    {
        candidate.act_end = points.cbegin() + 1;
        candidate.sp_bar_amount = 0.6;

        REQUIRE(!track.is_candidate_valid(candidate));
    }

    SECTION("Check intermediate SP is accounted for")
    {
        std::vector<StarPower> phrases {{3000, 100}};
        NoteTrack overlap_notes(notes, phrases, {});
        ProcessedTrack overlap_track(overlap_notes, {}, {});
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {
            overlap_points.cbegin(), overlap_points.cbegin() + 3, 0.0, 0.8};

        REQUIRE(overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("Check only reached intermediate SP is accounted for")
    {
        notes[2].position = 6000;
        std::vector<StarPower> phrases {{6000, 100}};
        NoteTrack overlap_notes(notes, phrases, {});
        ProcessedTrack overlap_track(overlap_notes, {}, {});
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {
            overlap_points.cbegin(), overlap_points.cbegin() + 3, 0.0, 0.8};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
    }

    SECTION("SP bar does not exceed full bar")
    {
        std::vector<Note> overlap_notes {{0}, {2}, {7000}};
        std::vector<StarPower> phrases {{0, 1}, {2, 1}};
        NoteTrack overlap_note_track(overlap_notes, phrases, {});
        ProcessedTrack overlap_track(overlap_note_track, {}, {});
        const auto& overlap_points = overlap_track.points();
        ActivationCandidate overlap_candidate {
            overlap_points.cbegin(), overlap_points.cbegin() + 2, 0.0, 1.0};

        REQUIRE(!overlap_track.is_candidate_valid(overlap_candidate));
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
    ActivationCandidate candidate {points.cbegin(), points.cend() - 2, 0.0,
                                   0.5};

    SECTION("Check whammy is counted")
    {
        REQUIRE(track.is_candidate_valid(candidate));
    }

    SECTION("Check compressed activations are counted")
    {
        candidate.sp_bar_amount = 0.9;
        REQUIRE(track.is_candidate_valid(candidate));
    }
}
