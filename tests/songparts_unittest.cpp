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

#include <boost/test/unit_test.hpp>

#include "test_helpers.hpp"

BOOST_AUTO_TEST_SUITE(note_track_ctor_maintains_invariants)

BOOST_AUTO_TEST_CASE(notes_are_sorted)
{
    std::vector<Note<NoteColour>> notes {{768}, {384}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<Note<NoteColour>> sorted_notes {{384}, {768}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  sorted_notes.cbegin(), sorted_notes.cend());
}

BOOST_AUTO_TEST_CASE(notes_of_the_same_colour_and_position_are_merged)
{
    std::vector<Note<NoteColour>> notes {{768, 0}, {768, 768}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<Note<NoteColour>> required_notes {{768, 768}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());

    std::vector<Note<NoteColour>> second_notes {{768, 768}, {768, 0}};
    NoteTrack<NoteColour> second_track {second_notes, {}, {}, {}, {}, {}, 192};
    std::vector<Note<NoteColour>> second_required_notes {{768, 0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        second_track.notes().cbegin(), second_track.notes().cend(),
        second_required_notes.cbegin(), second_required_notes.cend());
}

BOOST_AUTO_TEST_CASE(notes_of_different_colours_are_dealt_with_separately)
{
    std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green},
                                         {768, 0, NoteColour::Red},
                                         {768, 768, NoteColour::Green}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<Note<NoteColour>> required_notes {{768, 768, NoteColour::Green},
                                                  {768, 0, NoteColour::Red}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());
}

BOOST_AUTO_TEST_CASE(open_and_non_open_notes_of_same_pos_and_length_are_merged)
{
    std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green},
                                         {768, 1, NoteColour::Red},
                                         {768, 0, NoteColour::Open}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<Note<NoteColour>> required_notes {{768, 1, NoteColour::Red},
                                                  {768, 0, NoteColour::Open}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());
}

BOOST_AUTO_TEST_CASE(resolution_is_positive)
{
    std::vector<Note<NoteColour>> notes {{768}};
    BOOST_CHECK_THROW(
        [&] { return NoteTrack<NoteColour>(notes, {}, {}, {}, {}, {}, 0); }(),
        ParseError);
}

BOOST_AUTO_TEST_CASE(empty_sp_phrases_are_culled)
{
    std::vector<Note<NoteColour>> notes {{768}};
    std::vector<StarPower> phrases {{0, 100}, {700, 100}, {1000, 100}};
    NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    std::vector<StarPower> required_phrases {{700, 100}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_are_sorted)
{
    std::vector<Note<NoteColour>> notes {{768}, {1000}};
    std::vector<StarPower> phrases {{1000, 1}, {768, 1}};
    NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    std::vector<StarPower> required_phrases {{768, 1}, {1000, 1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_do_not_overlap)
{
    std::vector<Note<NoteColour>> notes {{768}, {1000}, {1500}};
    std::vector<StarPower> phrases {{768, 1000}, {900, 150}};
    NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    std::vector<StarPower> required_phrases {{768, 282}, {1050, 718}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(solos_are_sorted)
{
    std::vector<Note<NoteColour>> notes {{0}, {768}};
    std::vector<Solo> solos {{768, 868, 100}, {0, 100, 100}};
    NoteTrack<NoteColour> track {notes, {}, solos, {}, {}, {}, 192};
    std::vector<Solo> required_solos {{0, 100, 100}, {768, 868, 100}};
    std::vector<Solo> solo_output
        = track.solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(solo_output.cbegin(), solo_output.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(solos_do_take_into_account_drum_settings)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red},
        {0, 0, DrumNoteColour::DoubleKick},
        {192, 0, DrumNoteColour::DoubleKick}};
    std::vector<Solo> solos {{0, 1, 200}, {192, 193, 100}};
    NoteTrack<DrumNoteColour> track {notes, {}, solos, {}, {}, {}, 192};
    std::vector<Solo> required_solos {{0, 1, 100}};
    std::vector<Solo> solo_output = track.solos({false, false, true, false});

    BOOST_CHECK_EQUAL_COLLECTIONS(solo_output.cbegin(), solo_output.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_SUITE(automatic_drum_activation_zone_generation_is_correct)

BOOST_AUTO_TEST_CASE(automatic_zones_are_created)
{
    std::vector<Note<DrumNoteColour>> notes {
        {768}, {1536}, {2304}, {3072}, {3840}};
    TimeConverter converter {{}, 192, ChDrumEngine(), {}};

    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<DrumFill> fills {{384, 384}, {3456, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_have_250ms_of_leniency)
{
    std::vector<Note<DrumNoteColour>> notes {{672}, {3936}, {6815}, {10081}};
    TimeConverter converter {{}, 192, ChDrumEngine(), {}};

    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<DrumFill> fills {{384, 384}, {3456, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_handle_skipped_measures_correctly)
{
    std::vector<Note<DrumNoteColour>> notes {{768}, {4608}};
    TimeConverter converter {{}, 192, ChDrumEngine(), {}};

    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<DrumFill> fills {{384, 384}, {4224, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(the_last_automatic_zone_exists_even_if_the_note_is_early)
{
    std::vector<Note<DrumNoteColour>> notes {{760}};
    TimeConverter converter {{}, 192, ChDrumEngine(), {}};

    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<DrumFill> fills {{384, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_are_half_a_measure_according_to_seconds)
{
    std::vector<Note<DrumNoteColour>> notes {{768}};
    SyncTrack sync_track {{}, {{576, 40000}}};
    TimeConverter converter {sync_track, 192, ChDrumEngine(), {}};

    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<DrumFill> fills {{576, 192}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(fill_ends_remain_snapped_to_measure)
{
    std::vector<Note<DrumNoteColour>> notes {{758},  {770},  {3830},
                                             {3860}, {6900}, {6924}};
    TimeConverter converter {{}, 192, ChDrumEngine(), {}};

    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    std::vector<DrumFill> fills {{384, 384}, {3456, 384}, {6528, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(base_score_for_average_multiplier_is_correct)

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_without_sustains)
{
    std::vector<Note<NoteColour>> notes {
        {192}, {384}, {384, 0, NoteColour::Red}};

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

    BOOST_CHECK_EQUAL(track.base_score(), 150);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_with_sustains)
{
    std::vector<Note<NoteColour>> notes_one {{192, 192}};
    std::vector<Note<NoteColour>> notes_two {{192, 92}};
    std::vector<Note<NoteColour>> notes_three {{192, 93}};

    NoteTrack<NoteColour> track_one {notes_one, {}, {}, {}, {}, {}, 192};
    NoteTrack<NoteColour> track_two {notes_two, {}, {}, {}, {}, {}, 192};
    NoteTrack<NoteColour> track_three {notes_three, {}, {}, {}, {}, {}, 192};

    BOOST_CHECK_EQUAL(track_one.base_score(), 75);
    BOOST_CHECK_EQUAL(track_two.base_score(), 62);
    BOOST_CHECK_EQUAL(track_three.base_score(), 63);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_with_chord_sustains)
{
    std::vector<Note<NoteColour>> notes {{192, 192},
                                         {192, 192, NoteColour::Red}};

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

    BOOST_CHECK_EQUAL(track.base_score(), 125);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_other_resolutions)
{
    std::vector<Note<NoteColour>> notes {{192, 192}};

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 480};

    BOOST_CHECK_EQUAL(track.base_score(), 60);
}

BOOST_AUTO_TEST_CASE(fractional_ticks_from_multiple_holds_are_added_correctly)
{
    std::vector<Note<NoteColour>> notes {{0, 100}, {192, 100}};

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

    BOOST_CHECK_EQUAL(track.base_score(), 127);
}

BOOST_AUTO_TEST_CASE(disjoint_chords_are_handled_correctly)
{
    std::vector<Note<NoteColour>> notes {
        {0, 384}, {0, 384, NoteColour::Red}, {0, 192, NoteColour::Yellow}};

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

    BOOST_CHECK_EQUAL(track.base_score(), 275);
}

BOOST_AUTO_TEST_CASE(base_score_is_correctly_handled_with_open_note_merging)
{
    std::vector<Note<NoteColour>> notes {{0, 0}, {0, 0, NoteColour::Open}};

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

    BOOST_CHECK_EQUAL(track.base_score(), 100);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(base_score_is_correct_for_drums)

BOOST_AUTO_TEST_CASE(all_kicks_gives_correct_answer)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red},
        {192, 0, DrumNoteColour::Kick},
        {384, 0, DrumNoteColour::DoubleKick}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    DrumSettings settings {true, false, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 150);
}

BOOST_AUTO_TEST_CASE(only_single_kicks_gives_correct_answer)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red},
        {192, 0, DrumNoteColour::Kick},
        {384, 0, DrumNoteColour::DoubleKick}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    DrumSettings settings {false, false, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 100);
}

BOOST_AUTO_TEST_CASE(no_kicks_gives_correct_answer)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red},
        {192, 0, DrumNoteColour::Kick},
        {384, 0, DrumNoteColour::DoubleKick}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    DrumSettings settings {false, true, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 50);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(trim_sustains_is_correct)
{
    const std::vector<Note<NoteColour>> notes {{0, 65}, {200, 70}, {400, 140}};
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 200};
    const auto new_track = track.trim_sustains();
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].length, 0);
    BOOST_CHECK_EQUAL(new_notes[1].length, 70);
    BOOST_CHECK_EQUAL(new_notes[2].length, 140);
    BOOST_CHECK_EQUAL(new_track.base_score(), 177);
}

BOOST_AUTO_TEST_SUITE(snap_chords_is_correct)

BOOST_AUTO_TEST_CASE(no_snapping)
{
    const std::vector<Note<NoteColour>> notes {{0, 0, NoteColour::Green},
                                               {5, 0, NoteColour::Red}};
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 480};
    auto new_track = track.snap_chords(0);
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].position, 0);
    BOOST_CHECK_EQUAL(new_notes[1].position, 5);
}

BOOST_AUTO_TEST_CASE(hmx_gh_snapping)
{
    const std::vector<Note<NoteColour>> notes {{0, 0, NoteColour::Green},
                                               {5, 0, NoteColour::Red}};
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 480};
    auto new_track = track.snap_chords(10);
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].position, 0);
    BOOST_CHECK_EQUAL(new_notes[1].position, 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(disable_dynamics_is_correct)
{
    const std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red},
        {192, 0, DrumNoteColour::RedGhost},
        {384, 0, DrumNoteColour::RedAccent}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    track.disable_dynamics();

    const std::vector<Note<DrumNoteColour>> new_notes {
        {0, 0, DrumNoteColour::Red},
        {192, 0, DrumNoteColour::Red},
        {384, 0, DrumNoteColour::Red}};
    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  new_notes.cbegin(), new_notes.cend());
}
