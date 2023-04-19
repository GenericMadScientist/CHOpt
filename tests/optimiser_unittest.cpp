/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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
}

BOOST_AUTO_TEST_SUITE(overlap_guitar_paths)

BOOST_AUTO_TEST_CASE(simplest_song_with_a_non_empty_path)
{
    std::vector<Note> notes {make_note(0), make_note(192), make_note(384)};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {{points.cbegin() + 2,
                                           points.cbegin() + 2, Beat {0.0},
                                           Beat {2.0}, Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_with_multiple_acts)
{
    std::vector<Note> notes {
        make_note(0),
        make_note(192),
        make_chord(
            384,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}}),
        make_note(3840),
        make_note(4032),
        make_chord(
            10368,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}})};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {3840, 50}, {4032, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 2, Beat {0.0}, Beat {2.0},
         Beat {18.0}},
        {points.cbegin() + 5, points.cbegin() + 5, Beat {0.0}, Beat {54.0},
         Beat {70.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 300);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_with_an_act_containing_more_than_one_note)
{
    std::vector<Note> notes {make_note(0), make_note(192), make_note(384),
                             make_note(576)};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {{points.cbegin() + 2,
                                           points.cbegin() + 3, Beat {0.0},
                                           Beat {2.0}, Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 100);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_with_an_act_that_must_go_as_long_as_possible)
{
    std::vector<Note> notes {make_note(0), make_note(192), make_note(384),
                             make_note(3360)};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {{points.cbegin() + 2,
                                           points.cbegin() + 3, Beat {0.0},
                                           Beat {2.0}, Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 100);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_where_greedy_algorithm_fails)
{
    std::vector<Note> notes {
        make_note(0), make_note(192), make_note(384),
        make_chord(3840, {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}})};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {{points.cbegin() + 3,
                                           points.cbegin() + 3, Beat {0.0},
                                           Beat {20.0}, Beat {36.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 100);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(simplest_song_where_a_phrase_must_be_hit_early)
{
    std::vector<Note> notes {make_note(0),    make_note(192),  make_note(384),
                             make_note(3224), make_note(9378), make_note(15714),
                             make_note(15715)};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {3224, 50}, {9378, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 2, Beat {0.0}, Beat {0.8958},
         Beat {16.8958}},
        {points.cbegin() + 5, points.cbegin() + 6, Beat {0.0}, Beat {81.84375},
         Beat {97.84375}}};
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
    std::vector<Note> notes {
        make_note(0),          make_note(192),   make_note(384),
        make_note(3234, 1440), make_note(10944), make_note(10945),
        make_note(10946),      make_note(10947), make_note(10948),
        make_note(10949),      make_note(10950), make_note(10951),
        make_note(10952),      make_note(10953)};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {3234, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
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
    std::vector<Note> notes {make_note(0, 1420), make_note(1500),
                             make_note(1600)};
    std::vector<StarPower> phrases {{0, 1550}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
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
    std::vector<Note> notes {make_note(192, 1440), make_note(1632),
                             make_note(6336)};
    std::vector<StarPower> phrases {{192, 1}, {1632, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_LT(opt_path.score_boost, 100);
}

// There was a bug where an activation on a note right after an SP sustain could
// be drawn starting right after the tick burst, rather than the proper place.
// An example is the last activation of Gamer National Anthem in CSC August
// 2020.
BOOST_AUTO_TEST_CASE(activation_right_after_a_sp_sustain_is_drawn_correctly)
{
    std::vector<Note> notes {make_note(0, 1488), make_note(2880, 3264)};
    std::vector<StarPower> phrases {{0, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_GE(opt_path.activations[0].sp_start.value(), 15.0);
}

BOOST_AUTO_TEST_CASE(songs_ending_in_es1_are_pathed_correctly)
{
    std::vector<Note> notes {make_note(0),   make_note(192), make_note(384),
                             make_note(576), make_note(768), make_note(4032),
                             make_note(4224)};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {4032, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 150);
    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
}

BOOST_AUTO_TEST_CASE(compressed_whammy_is_specified_correctly)
{
    std::vector<Note> notes {
        make_note(192, 192),
        make_note(672),
        make_chord(
            1000,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}}),
        make_note(3840),
        make_note(9984),
        make_chord(
            10176,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}})};
    std::vector<StarPower> phrases {
        {192, 50}, {672, 50}, {3840, 50}, {9984, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

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
    std::vector<Note> notes {make_note(0, 1536), make_note(1728)};
    std::vector<StarPower> phrases {{0, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    const auto& act = opt_path.activations[0];

    BOOST_CHECK_GT(act.whammy_end.value(), 16.0);
}

BOOST_AUTO_TEST_CASE(use_next_point_to_work_out_compressed_whammy)
{
    std::vector<Note> notes {
        make_note(0),
        make_note(192),
        make_chord(
            384,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}}),
        make_note(3350),
        make_note(3360),
        make_note(9504),
        make_chord(
            9696,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}})};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {3350, 50}, {9504, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

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
    std::vector<Note> notes {make_note(0),     make_note(192),
                             make_note(768),   make_note(3840, 192),
                             make_note(4224),  make_note(19200, 192),
                             make_note(38400), make_note(41990)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}, {3840, 576}, {19200, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 200);
}

// This isn't terribly well-defined. The heuristic is to still do a greedy
// approach but to pick the easiest activation at any point given a tie. The
// test is just enough to spot a difference between that and simple greedy.
BOOST_AUTO_TEST_CASE(easier_activations_are_chosen_where_possible)
{
    std::vector<Note> notes {make_note(0),    make_note(192),
                             make_note(384),  make_note(3504),
                             make_note(9600), make_note(12672)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

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
    std::vector<Note> notes {make_note(0, 1392), make_note(1536, 192)};
    std::vector<StarPower> phrases {{0, 1}, {1536, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 28);
}

// Video lag can cause a hold point to be the first point in a song. If this
// happens then we cannot use std::prev on the iterator for the first point, so
// we must check this before calling std::prev.
BOOST_AUTO_TEST_CASE(does_not_crash_with_positive_video_lag)
{
    std::vector<Note> notes {make_note(192, 192)};
    std::vector<StarPower> phrases {{192, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         {1.0, 1.0, Second {0.0}, Second {0.1}, Second {0.0}},
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 0);
}

BOOST_AUTO_TEST_CASE(whammy_delay_is_handled_correctly)
{
    std::vector<Note> notes {
        make_note(0),          make_note(192),   make_note(768),
        make_note(3840, 1420), make_note(5376),  make_note(13056),
        make_note(13248),      make_note(13440), make_note(13632),
        make_note(13824),      make_note(14016), make_note(14208)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}, {3840, 1728}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         {1.0, 1.0, Second {0.0}, Second {0.0}, Second {0.1}},
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.1)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 2);
    BOOST_CHECK_EQUAL(opt_path.score_boost, 550);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(drum_paths)

BOOST_AUTO_TEST_CASE(drum_paths_can_only_activate_on_activation_notes)
{
    std::vector<Note> notes {make_drum_note(0), make_drum_note(192),
                             make_drum_note(3000), make_drum_note(4000)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}};
    std::vector<DrumFill> fills {{3900, 101}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          fills,
                          {},
                          {},
                          TrackType::Drums,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChDrumEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
}

BOOST_AUTO_TEST_CASE(
    drum_paths_cant_activate_way_earlier_than_an_activation_note)
{
    std::vector<Note> notes {make_drum_note(0),     make_drum_note(192),
                             make_drum_note(3840),  make_drum_note(3940),
                             make_drum_note(4040),  make_drum_note(17000),
                             make_drum_note(20000), make_drum_note(20100)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}, {4040, 1}, {17000, 1}};
    std::vector<DrumFill> fills {{3830, 20}, {19990, 20}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          fills,
                          {},
                          {},
                          TrackType::Drums,
                          std::make_shared<SongGlobalData>()};

    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChDrumEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 1);
    BOOST_CHECK_EQUAL(opt_path.score_boost, 150);
}

BOOST_AUTO_TEST_CASE(drum_reverse_squeezes_are_drawn_properly)
{
    std::vector<Note> notes {make_drum_note(0),     make_drum_note(192),
                             make_drum_note(19200), make_drum_note(22232),
                             make_drum_note(22260), make_drum_note(90000),
                             make_drum_note(90100), make_drum_note(90200)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}, {22232, 1}, {22260, 1}};
    std::vector<DrumFill> fills {{19190, 20}, {89990, 20}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          fills,
                          {},
                          {},
                          TrackType::Drums,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChDrumEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.activations.size(), 2);
    BOOST_CHECK_GT(opt_path.activations[0].sp_start.value(), 99.8);
}

BOOST_AUTO_TEST_CASE(
    drum_activations_can_only_happen_two_seconds_after_getting_sp)
{
    std::vector<Note> notes {make_drum_note(0), make_drum_note(192),
                             make_drum_note(800), make_drum_note(1000)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}};
    std::vector<DrumFill> fills {{800, 1}, {1000, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          fills,
                          {},
                          {},
                          TrackType::Drums,
                          std::make_shared<SongGlobalData>()};

    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChDrumEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
}

BOOST_AUTO_TEST_CASE(drum_activation_delay_is_affected_by_speed)
{
    std::vector<Note> notes {make_drum_note(0), make_drum_note(192),
                             make_drum_note(800), make_drum_note(1000)};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}};
    std::vector<DrumFill> fills {{800, 1}, {1000, 1}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          fills,
                          {},
                          {},
                          TrackType::Drums,
                          std::make_shared<SongGlobalData>()};

    ProcessedSong track {note_track,
                         TempoMap().speedup(200),
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChDrumEngine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 200, Second(0.0)};

    const auto opt_path = optimiser.optimal_path();
    BOOST_CHECK_EQUAL(opt_path.score_boost, 50);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(no_overlap_guitar_paths)

BOOST_AUTO_TEST_CASE(simplest_song_where_overlap_matters)
{
    std::vector<Note> notes {
        make_note(0), make_note(192),
        make_chord(384, {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}}),
        make_note(3456), make_note(4224)};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {3456, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         Gh1Engine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {{points.cbegin() + 2,
                                           points.cbegin() + 3, Beat {0.0},
                                           Beat {2.0}, Beat {18.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 150);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(partial_overlap_doesnt_work)
{
    std::vector<Note> notes {
        make_note(0),
        make_note(192),
        make_chord(
            384,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}}),
        make_note(3456),
        make_note(3648),
        make_note(4224),
        make_chord(
            4416,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}})};
    std::vector<StarPower> phrases {
        {0, 50}, {192, 50}, {3456, 200}, {4224, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         Gh1Engine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cbegin() + 2, points.cbegin() + 2, Beat {0.0}, Beat {2.0},
         Beat {18.0}},
        {points.cbegin() + 6, points.cbegin() + 6, Beat {0.0}, Beat {23.0},
         Beat {39.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 300);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_CASE(compressed_whammy_considered_even_with_maxable_sp)
{
    std::vector<Note> notes {
        make_note(0, 3072),
        make_note(9600),
        make_note(10368),
        make_chord(
            11136,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}}),
        make_chord(
            15744,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}}),
        make_note(15936),
        make_note(23616),
        make_chord(
            24384,
            {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}, {FIVE_FRET_YELLOW, 0}})};
    std::vector<StarPower> phrases {
        {0, 50}, {9600, 800}, {15936, 50}, {23616, 50}};
    NoteTrack note_track {notes,
                          phrases,
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         Gh1Engine(),
                         {},
                         {}};
    Optimiser optimiser {&track, &term_bool, 100, Second(0.0)};
    const auto& points = track.points();
    std::vector<Activation> optimal_acts {
        {points.cend() - 5, points.cend() - 4, Beat {0.0}, Beat {54.0},
         Beat {83.0}},
        {points.cend() - 1, points.cend() - 1, Beat {0.0}, Beat {127.0},
         Beat {143.0}}};
    const auto opt_path = optimiser.optimal_path();

    BOOST_CHECK_EQUAL(opt_path.score_boost, 450);
    BOOST_CHECK_EQUAL_COLLECTIONS(opt_path.activations.cbegin(),
                                  opt_path.activations.cend(),
                                  optimal_acts.cbegin(), optimal_acts.cend());
}

BOOST_AUTO_TEST_SUITE_END()
