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
    std::vector<Note> notes {make_note(768), make_note(384)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<Note> sorted_notes {make_note(384), make_note(768)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  sorted_notes.cbegin(), sorted_notes.cend());
}

BOOST_AUTO_TEST_CASE(notes_of_the_same_colour_and_position_are_merged)
{
    std::vector<Note> notes {make_note(768, 0), make_note(768, 768)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<Note> required_notes {make_note(768, 768)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());

    std::vector<Note> second_notes {make_note(768, 768), make_note(768, 0)};
    NoteTrack second_track {second_notes,
                            {},
                            {},
                            {},
                            {},
                            {},
                            TrackType::FiveFret,
                            std::make_shared<SongGlobalData>()};
    std::vector<Note> second_required_notes {make_note(768, 0)};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        second_track.notes().cbegin(), second_track.notes().cend(),
        second_required_notes.cbegin(), second_required_notes.cend());
}

BOOST_AUTO_TEST_CASE(notes_of_different_colours_are_dealt_with_separately)
{
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN),
                             make_note(768, 0, FIVE_FRET_RED),
                             make_note(768, 768, FIVE_FRET_GREEN)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<Note> required_notes {
        make_chord(768, {{FIVE_FRET_GREEN, 768}, {FIVE_FRET_RED, 0}})};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());
}

BOOST_AUTO_TEST_CASE(open_and_non_open_notes_of_same_pos_and_length_are_merged)
{
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN),
                             make_note(768, 1, FIVE_FRET_RED),
                             make_note(768, 0, FIVE_FRET_OPEN)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<Note> required_notes {
        make_chord(768, {{FIVE_FRET_RED, 1}, {FIVE_FRET_OPEN, 0}})};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());
}

BOOST_AUTO_TEST_CASE(resolution_is_positive)
{
    SongGlobalData data;
    BOOST_CHECK_THROW([&] { data.resolution(0); }(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(empty_sp_phrases_are_culled)
{
    std::vector<Note> notes {make_note(768)};
    std::vector<StarPower> phrases {{0, 100}, {700, 100}, {1000, 100}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<StarPower> required_phrases {{700, 100}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_are_sorted)
{
    std::vector<Note> notes {make_note(768), make_note(1000)};
    std::vector<StarPower> phrases {{1000, 1}, {768, 1}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<StarPower> required_phrases {{768, 1}, {1000, 1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_do_not_overlap)
{
    std::vector<Note> notes {make_note(768), make_note(1000), make_note(1500)};
    std::vector<StarPower> phrases {{768, 1000}, {900, 150}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<StarPower> required_phrases {{768, 282}, {1050, 718}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(solos_are_sorted)
{
    std::vector<Note> notes {make_note(0), make_note(768)};
    std::vector<Solo> solos {{768, 868, 100}, {0, 100, 100}};
    NoteTrack track {notes,
                     {},
                     solos,
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
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
    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(0, DRUM_DOUBLE_KICK),
                             make_drum_note(192, DRUM_DOUBLE_KICK)};
    std::vector<Solo> solos {{0, 1, 200}, {192, 193, 100}};
    NoteTrack track {notes,
                     {},
                     solos,
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    std::vector<Solo> required_solos {{0, 1, 100}};
    std::vector<Solo> solo_output = track.solos({false, false, true, false});

    BOOST_CHECK_EQUAL_COLLECTIONS(solo_output.cbegin(), solo_output.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_SUITE(automatic_drum_activation_zone_generation_is_correct)

BOOST_AUTO_TEST_CASE(automatic_zones_are_created)
{
    std::vector<Note> notes {make_drum_note(768), make_drum_note(1536),
                             make_drum_note(2304), make_drum_note(3072),
                             make_drum_note(3840)};
    TimeConverter converter {{}, ChDrumEngine(), {}};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    std::vector<DrumFill> fills {{384, 384}, {3456, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_have_250ms_of_leniency)
{
    std::vector<Note> notes {make_drum_note(672), make_drum_note(3936),
                             make_drum_note(6815), make_drum_note(10081)};
    TimeConverter converter {{}, ChDrumEngine(), {}};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    std::vector<DrumFill> fills {{384, 384}, {3456, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_handle_skipped_measures_correctly)
{
    std::vector<Note> notes {make_drum_note(768), make_drum_note(4608)};
    TimeConverter converter {{}, ChDrumEngine(), {}};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    std::vector<DrumFill> fills {{384, 384}, {4224, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(the_last_automatic_zone_exists_even_if_the_note_is_early)
{
    std::vector<Note> notes {make_drum_note(760)};
    TimeConverter converter {{}, ChDrumEngine(), {}};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    std::vector<DrumFill> fills {{384, 384}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_are_half_a_measure_according_to_seconds)
{
    std::vector<Note> notes {make_drum_note(768)};
    TempoMap tempo_map {{}, {{Tick {576}, 40000}}, 192};
    TimeConverter converter {tempo_map, ChDrumEngine(), {}};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    std::vector<DrumFill> fills {{576, 192}};

    track.generate_drum_fills(converter);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(fill_ends_remain_snapped_to_measure)
{
    std::vector<Note> notes {make_drum_note(758),  make_drum_note(770),
                             make_drum_note(3830), make_drum_note(3860),
                             make_drum_note(6900), make_drum_note(6924)};
    TimeConverter converter {{}, ChDrumEngine(), {}};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
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
    std::vector<Note> notes {
        make_note(192),
        make_chord(384, {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}})};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 150);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_with_sustains)
{
    std::vector<Note> notes_one {make_note(192, 192)};
    std::vector<Note> notes_two {make_note(192, 92)};
    std::vector<Note> notes_three {make_note(192, 93)};

    NoteTrack track_one {notes_one,
                         {},
                         {},
                         {},
                         {},
                         {},
                         TrackType::FiveFret,
                         std::make_shared<SongGlobalData>()};
    NoteTrack track_two {notes_two,
                         {},
                         {},
                         {},
                         {},
                         {},
                         TrackType::FiveFret,
                         std::make_shared<SongGlobalData>()};
    NoteTrack track_three {notes_three,
                           {},
                           {},
                           {},
                           {},
                           {},
                           TrackType::FiveFret,
                           std::make_shared<SongGlobalData>()};

    BOOST_CHECK_EQUAL(track_one.base_score(), 75);
    BOOST_CHECK_EQUAL(track_two.base_score(), 62);
    BOOST_CHECK_EQUAL(track_three.base_score(), 63);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_with_chord_sustains)
{
    std::vector<Note> notes {make_note(192, 192),
                             make_note(192, 192, FIVE_FRET_RED)};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 125);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_other_resolutions)
{
    std::vector<Note> notes {make_note(192, 192)};

    NoteTrack track {
        notes, {}, {}, {}, {}, {}, TrackType::FiveFret, make_resolution(480)};

    BOOST_CHECK_EQUAL(track.base_score(), 60);
}

BOOST_AUTO_TEST_CASE(fractional_ticks_from_multiple_holds_are_added_correctly)
{
    std::vector<Note> notes {make_note(0, 100), make_note(192, 100)};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 127);
}

BOOST_AUTO_TEST_CASE(disjoint_chords_are_handled_correctly)
{
    std::vector<Note> notes {make_note(0, 384),
                             make_note(0, 384, FIVE_FRET_RED),
                             make_note(0, 192, FIVE_FRET_YELLOW)};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 275);
}

BOOST_AUTO_TEST_CASE(base_score_is_correctly_handled_with_open_note_merging)
{
    std::vector<Note> notes {make_note(0, 0), make_note(0, 0, FIVE_FRET_OPEN)};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 50);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(base_score_is_correct_for_drums)

BOOST_AUTO_TEST_CASE(all_kicks_gives_correct_answer)
{
    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(192, DRUM_KICK),
                             make_drum_note(384, DRUM_DOUBLE_KICK)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    DrumSettings settings {true, false, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 150);
}

BOOST_AUTO_TEST_CASE(only_single_kicks_gives_correct_answer)
{
    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(192, DRUM_KICK),
                             make_drum_note(384, DRUM_DOUBLE_KICK)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    DrumSettings settings {false, false, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 100);
}

BOOST_AUTO_TEST_CASE(no_kicks_gives_correct_answer)
{
    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(192, DRUM_KICK),
                             make_drum_note(384, DRUM_DOUBLE_KICK)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    DrumSettings settings {false, true, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 50);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(trim_sustains_is_correct)
{
    const std::vector<Note> notes {make_note(0, 65), make_note(200, 70),
                                   make_note(400, 140)};
    const NoteTrack track {
        notes, {}, {}, {}, {}, {}, TrackType::FiveFret, make_resolution(200)};
    const auto new_track = track.trim_sustains();
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].lengths[0], 0);
    BOOST_CHECK_EQUAL(new_notes[1].lengths[0], 70);
    BOOST_CHECK_EQUAL(new_notes[2].lengths[0], 140);
    BOOST_CHECK_EQUAL(new_track.base_score(), 177);
}

BOOST_AUTO_TEST_SUITE(snap_chords_is_correct)

BOOST_AUTO_TEST_CASE(no_snapping)
{
    const std::vector<Note> notes {make_note(0, 0, FIVE_FRET_GREEN),
                                   make_note(5, 0, FIVE_FRET_RED)};
    const NoteTrack track {
        notes, {}, {}, {}, {}, {}, TrackType::FiveFret, make_resolution(480)};
    auto new_track = track.snap_chords(0);
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].position, 0);
    BOOST_CHECK_EQUAL(new_notes[1].position, 5);
}

BOOST_AUTO_TEST_CASE(hmx_gh_snapping)
{
    const std::vector<Note> notes {make_note(0, 0, FIVE_FRET_GREEN),
                                   make_note(5, 0, FIVE_FRET_RED)};
    const NoteTrack track {
        notes, {}, {}, {}, {}, {}, TrackType::FiveFret, make_resolution(480)};
    auto new_track = track.snap_chords(10);
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].position, 0);
    BOOST_CHECK_EQUAL(new_notes[1].position, 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(disable_dynamics_is_correct)
{
    const std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                                   make_drum_note(192, DRUM_RED, FLAGS_GHOST),
                                   make_drum_note(384, DRUM_RED, FLAGS_ACCENT)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    track.disable_dynamics();

    const std::vector<Note> new_notes {make_drum_note(0, DRUM_RED),
                                       make_drum_note(192, DRUM_RED),
                                       make_drum_note(384, DRUM_RED)};
    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  new_notes.cbegin(), new_notes.cend());
}
