/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2025 Raymond Wright
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
#include <cstdlib>
#include <iterator>

#include <boost/test/unit_test.hpp>

#include "optimiser.hpp"
#include "test_helpers.hpp"

namespace {
const std::atomic<bool> term_bool {false};

PathingSettings whammy_delay_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            1.0,
            1.0,
            SightRead::DrumSettings::default_settings(),
            {SightRead::Second {0.0}, SightRead::Second {0.0},
             SightRead::Second {0.1}}};
}
}

BOOST_AUTO_TEST_SUITE(overlap_guitar_paths)

BOOST_AUTO_TEST_CASE(simplest_song_with_a_non_empty_path)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 2, SightRead::Beat {0.0},
         SightRead::Beat {2.0}, SightRead::Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_with_multiple_acts)
{
    std::vector<SightRead::Note> notes {
        make_note(0),
        make_note(192),
        make_chord(384,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}}),
        make_note(3840),
        make_note(4032),
        make_chord(10368,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}})};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {3840}, SightRead::Tick {50}},
        {SightRead::Tick {4032}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 2, SightRead::Beat {0.0},
         SightRead::Beat {2.0}, SightRead::Beat {18.0}},
        {points.cbegin() + 5, points.cbegin() + 5, SightRead::Beat {0.0},
         SightRead::Beat {54.0}, SightRead::Beat {70.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 300);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_with_an_act_containing_more_than_one_note)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(576)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 3, SightRead::Beat {0.0},
         SightRead::Beat {2.0}, SightRead::Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 100);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_with_an_act_that_must_go_as_long_as_possible)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(3360)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 3, SightRead::Beat {0.0},
         SightRead::Beat {2.0}, SightRead::Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 100);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_where_greedy_algorithm_fails)
{
    std::vector<SightRead::Note> notes {
        make_note(0), make_note(192), make_note(384),
        make_chord(
            3840,
            {{SightRead::FIVE_FRET_GREEN, 0}, {SightRead::FIVE_FRET_RED, 0}})};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 3, points.cbegin() + 3, SightRead::Beat {0.0},
         SightRead::Beat {20.0}, SightRead::Beat {36.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 100);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_where_a_phrase_must_be_hit_early)
{
    std::vector<SightRead::Note> notes {
        make_note(0),    make_note(192),   make_note(384),  make_note(3224),
        make_note(9378), make_note(15714), make_note(15715)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {3224}, SightRead::Tick {50}},
        {SightRead::Tick {9378}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 2, SightRead::Beat {0.0},
         SightRead::Beat {0.8958}, SightRead::Beat {16.8958}},
        {points.cbegin() + 5, points.cbegin() + 6, SightRead::Beat {0.0},
         SightRead::Beat {81.84375}, SightRead::Beat {97.84375}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 150);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

// Naively the ideal path would be 2-1, but we have to squeeze the last SP
// phrase early for the 2 to work and this makes the 1 impossible. So the
// optimal path is really 3.
BOOST_AUTO_TEST_CASE(simplest_song_where_activations_ending_late_matter)
{
    std::vector<SightRead::Note> notes {
        make_note(0),          make_note(192),   make_note(384),
        make_note(3234, 1440), make_note(10944), make_note(10945),
        make_note(10946),      make_note(10947), make_note(10948),
        make_note(10949),      make_note(10950), make_note(10951),
        make_note(10952),      make_note(10953)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {3234}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 750);
    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
}

// There was a bug where sustains at the start of an SP phrase right after an
// activation/start of song had their early whammy discounted, if that note
// didn't also grant SP. This affected a squeeze in GH3 Cult of Personality.
// This test is to catch that.
BOOST_AUTO_TEST_CASE(early_whammy_at_start_of_an_sp_phrase_is_always_counted)
{
    std::vector<SightRead::Note> notes {make_note(0, 1420), make_note(1500),
                                        make_note(1600)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1550}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
}

// There was a bug where an activation on a note right after an SP sustain could
// double count the whammy available between the burst at the end of the sustain
// and the note. This affected a squeeze in Epidox, making chopt think you could
// squeeze from the O right before Robotic Buildup to a B in the next section.
BOOST_AUTO_TEST_CASE(
    whammy_just_before_the_activation_point_is_not_double_counted)
{
    std::vector<SightRead::Note> notes {make_note(192, 1440), make_note(1632),
                                        make_note(6336)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {192}, SightRead::Tick {1}},
        {SightRead::Tick {1632}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_LT(opt_path.score_boost, 100);
}

// There was a bug where an activation on a note right after an SP sustain could
// be drawn starting right after the tick burst, rather than the proper place.
// An example is the last activation of Gamer National Anthem in CSC August
// 2020.
BOOST_AUTO_TEST_CASE(activation_right_after_a_sp_sustain_is_drawn_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0, 1488),
                                        make_note(2880, 3264)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_GE(opt_path.activations[0].sp_start.value(), 15.0);
}

BOOST_AUTO_TEST_CASE(songs_ending_in_es1_are_pathed_correctly)
{
    std::vector<SightRead::Note> notes {
        make_note(0),   make_note(192),  make_note(384), make_note(576),
        make_note(768), make_note(4032), make_note(4224)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {4032}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 150);
    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
}

BOOST_AUTO_TEST_CASE(compressed_whammy_is_specified_correctly)
{
    std::vector<SightRead::Note> notes {
        make_note(192, 192),
        make_note(672),
        make_chord(1000,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}}),
        make_note(3840),
        make_note(9984),
        make_chord(10176,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}})};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {672}, SightRead::Tick {50}},
        {SightRead::Tick {3840}, SightRead::Tick {50}},
        {SightRead::Tick {9984}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    const auto& act = opt_path.activations[0];

    BOOST_CHECK_EQUAL(opt_path.score_boost, 300);
    BOOST_CHECK_EQUAL(opt_path.activations.size(), 2);
    BOOST_CHECK_GT(act.whammy_end.value(), 1.06);
    BOOST_CHECK_LT(act.whammy_end.value(), 1.74);
    BOOST_CHECK_LT(act.sp_start.value(), 3.6);
}

BOOST_AUTO_TEST_CASE(acts_covering_the_last_note_do_not_compress_whammy)
{
    std::vector<SightRead::Note> notes {make_note(0, 1536), make_note(1728)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    const auto& act = opt_path.activations[0];

    BOOST_CHECK_GT(act.whammy_end.value(), 16.0);
}

BOOST_AUTO_TEST_CASE(use_next_point_to_work_out_compressed_whammy)
{
    std::vector<SightRead::Note> notes {
        make_note(0),
        make_note(192),
        make_chord(384,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}}),
        make_note(3350),
        make_note(3360),
        make_note(9504),
        make_chord(9696,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}})};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {3350}, SightRead::Tick {50}},
        {SightRead::Tick {9504}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    const auto& act = opt_path.activations[0];

    BOOST_CHECK_GT(act.whammy_end.value(), 17.45);
}

// There was a bug where an activation after an SP sustain that comes after an
// act with a forbidden squeeze would be shown to have ticks possible on the
// forbidden squeeze even if ticks were not possible. An example is given by the
// path for EON BREAK in CSC December 2019.
BOOST_AUTO_TEST_CASE(forbidden_squeeze_does_not_grant_extra_whammy_next_act)
{
    std::vector<SightRead::Note> notes {make_note(0),     make_note(192),
                                        make_note(768),   make_note(3840, 192),
                                        make_note(4224),  make_note(19200, 192),
                                        make_note(38400), make_note(41990)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}},
        {SightRead::Tick {3840}, SightRead::Tick {576}},
        {SightRead::Tick {19200}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 200);
}

// This isn't terribly well-defined. The heuristic is to still do a greedy
// approach but to pick the easiest activation at any point given a tie. The
// test is just enough to spot a difference between that and simple greedy.
BOOST_AUTO_TEST_CASE(easier_activations_are_chosen_where_possible)
{
    std::vector<SightRead::Note> notes {make_note(0),    make_note(192),
                                        make_note(384),  make_note(3504),
                                        make_note(9600), make_note(12672)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_GT(opt_path.activations[0].sp_start.value(), 20.0);
}

// There was a bug where EW could be obtained from a note before the note was
// hit. This came up in xOn Our Kneesx from CSC November 2020, where this makes
// CHOpt believe you can activate before the GY note and get an extra 300
// points.
BOOST_AUTO_TEST_CASE(
    early_whammy_from_a_note_cannot_be_obtained_until_the_note_is_hit)
{
    std::vector<SightRead::Note> notes {make_note(0, 1392),
                                        make_note(1536, 192)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {1536}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 28);
}

// Video lag can cause a hold point to be the first point in a song. If this
// happens then we cannot use std::prev on the iterator for the first point, so
// we must check this before calling std::prev.
BOOST_AUTO_TEST_CASE(does_not_crash_with_positive_video_lag)
{
    std::vector<SightRead::Note> notes {make_note(192, 192)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {192}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         positive_video_lag_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 0);
}

BOOST_AUTO_TEST_CASE(whammy_delay_is_handled_correctly)
{
    std::vector<SightRead::Note> notes {
        make_note(0),          make_note(192),   make_note(768),
        make_note(3840, 1420), make_note(5376),  make_note(13056),
        make_note(13248),      make_note(13440), make_note(13632),
        make_note(13824),      make_note(14016), make_note(14208)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}},
        {SightRead::Tick {3840}, SightRead::Tick {1728}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {
        note_track, {{}, SpMode::Measure}, whammy_delay_settings(), {}, {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.1)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 2);
    BOOST_CHECK_EQUAL(opt_path.score_boost, 550);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(drum_paths)

BOOST_AUTO_TEST_CASE(drum_paths_can_only_activate_on_activation_notes)
{
    std::vector<SightRead::Note> notes {make_drum_note(0), make_drum_note(192),
                                        make_drum_note(3000),
                                        make_drum_note(4000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}}};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {3900}, SightRead::Tick {101}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.drum_fills(fills);
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_drums_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
}

BOOST_AUTO_TEST_CASE(
    drum_paths_cant_activate_way_earlier_than_an_activation_note)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0),     make_drum_note(192),  make_drum_note(3840),
        make_drum_note(3940),  make_drum_note(4040), make_drum_note(17000),
        make_drum_note(20000), make_drum_note(20100)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}},
        {SightRead::Tick {4040}, SightRead::Tick {1}},
        {SightRead::Tick {17000}, SightRead::Tick {1}}};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {3830}, SightRead::Tick {20}},
        {SightRead::Tick {19990}, SightRead::Tick {20}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.drum_fills(fills);

    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_drums_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
    BOOST_CHECK_EQUAL(opt_path.score_boost, 150);
}

BOOST_AUTO_TEST_CASE(drum_reverse_squeezes_are_drawn_properly)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0),     make_drum_note(192),   make_drum_note(19200),
        make_drum_note(22232), make_drum_note(22260), make_drum_note(90000),
        make_drum_note(90100), make_drum_note(90200)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}},
        {SightRead::Tick {22232}, SightRead::Tick {1}},
        {SightRead::Tick {22260}, SightRead::Tick {1}}};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {19190}, SightRead::Tick {20}},
        {SightRead::Tick {89990}, SightRead::Tick {20}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.drum_fills(fills);
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_drums_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 2);
    BOOST_CHECK_GT(opt_path.activations[0].sp_start.value(), 99.8);
}

BOOST_AUTO_TEST_CASE(
    drum_activations_can_only_happen_two_seconds_after_getting_sp)
{
    std::vector<SightRead::Note> notes {make_drum_note(0), make_drum_note(192),
                                        make_drum_note(800),
                                        make_drum_note(1000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}}};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {800}, SightRead::Tick {1}},
        {SightRead::Tick {1000}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.drum_fills(fills);

    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_drums_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
}

BOOST_AUTO_TEST_CASE(drum_activation_delay_is_affected_by_speed)
{
    std::vector<SightRead::Note> notes {make_drum_note(0), make_drum_note(192),
                                        make_drum_note(800),
                                        make_drum_note(1000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}}};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {800}, SightRead::Tick {1}},
        {SightRead::Tick {1000}, SightRead::Tick {1}}};

    auto tempo_map = SightRead::TempoMap().speedup(200);
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    SightRead::NoteTrack note_track {notes, phrases,
                                     SightRead::TrackType::Drums, global_data};
    note_track.drum_fills(fills);

    ProcessedSong track {note_track,
                         {tempo_map, SpMode::Measure},
                         default_drums_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 200, SightRead::Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(no_overlap_guitar_paths)

BOOST_AUTO_TEST_CASE(simplest_song_where_overlap_matters)
{
    std::vector<SightRead::Note> notes {
        make_note(0), make_note(192),
        make_chord(
            384,
            {{SightRead::FIVE_FRET_GREEN, 0}, {SightRead::FIVE_FRET_RED, 0}}),
        make_note(3456), make_note(4224)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {3456}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 3, SightRead::Beat {0.0},
         SightRead::Beat {2.0}, SightRead::Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 150);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(partial_overlap_doesnt_work)
{
    std::vector<SightRead::Note> notes {
        make_note(0),
        make_note(192),
        make_chord(384,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}}),
        make_note(3456),
        make_note(3648),
        make_note(4224),
        make_chord(4416,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}})};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {3456}, SightRead::Tick {200}},
        {SightRead::Tick {4224}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 2, SightRead::Beat {0.0},
         SightRead::Beat {2.0}, SightRead::Beat {18.0}},
        {points.cbegin() + 6, points.cbegin() + 6, SightRead::Beat {0.0},
         SightRead::Beat {23.0}, SightRead::Beat {39.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 300);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(compressed_whammy_considered_even_with_maxable_sp)
{
    std::vector<SightRead::Note> notes {
        make_note(0, 3072),
        make_note(9600),
        make_note(10368),
        make_chord(11136,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}}),
        make_chord(15744,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}}),
        make_note(15936),
        make_note(23616),
        make_chord(24384,
                   {{SightRead::FIVE_FRET_GREEN, 0},
                    {SightRead::FIVE_FRET_RED, 0},
                    {SightRead::FIVE_FRET_YELLOW, 0}})};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {9600}, SightRead::Tick {800}},
        {SightRead::Tick {15936}, SightRead::Tick {50}},
        {SightRead::Tick {23616}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cend() - 5, points.cend() - 4, SightRead::Beat {0.0},
         SightRead::Beat {54.0}, SightRead::Beat {83.0}},
        {points.cend() - 1, points.cend() - 1, SightRead::Beat {0.0},
         SightRead::Beat {127.0}, SightRead::Beat {143.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 450);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(quarter_bar_activations_are_possible_on_fortnite_engine)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FortniteFestival,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::OdBeat},
                         default_fortnite_guitar_pathing_settings(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, SightRead::Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 1, points.cbegin() + 1, SightRead::Beat {0.0},
         SightRead::Beat {1.0}, SightRead::Beat {9.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 36);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}
