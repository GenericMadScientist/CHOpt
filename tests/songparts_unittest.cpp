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
    std::vector<SightRead::Note> notes {make_note(768), make_note(384)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::Note> sorted_notes {make_note(384), make_note(768)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  sorted_notes.cbegin(), sorted_notes.cend());
}

BOOST_AUTO_TEST_CASE(notes_of_the_same_colour_and_position_are_merged)
{
    std::vector<SightRead::Note> notes {make_note(768, 0), make_note(768, 768)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::Note> required_notes {make_note(768, 768)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());

    std::vector<SightRead::Note> second_notes {make_note(768, 768),
                                               make_note(768, 0)};
    SightRead::NoteTrack second_track {
        second_notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::Note> second_required_notes {make_note(768, 0)};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        second_track.notes().cbegin(), second_track.notes().cend(),
        second_required_notes.cbegin(), second_required_notes.cend());
}

BOOST_AUTO_TEST_CASE(notes_of_different_colours_are_dealt_with_separately)
{
    std::vector<SightRead::Note> notes {
        make_note(768, 0, SightRead::FIVE_FRET_GREEN),
        make_note(768, 0, SightRead::FIVE_FRET_RED),
        make_note(768, 768, SightRead::FIVE_FRET_GREEN)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::Note> required_notes {make_chord(
        768,
        {{SightRead::FIVE_FRET_GREEN, 768}, {SightRead::FIVE_FRET_RED, 0}})};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());
}

BOOST_AUTO_TEST_CASE(open_and_non_open_notes_of_same_pos_and_length_are_merged)
{
    std::vector<SightRead::Note> notes {
        make_note(768, 0, SightRead::FIVE_FRET_GREEN),
        make_note(768, 1, SightRead::FIVE_FRET_RED),
        make_note(768, 0, SightRead::FIVE_FRET_OPEN)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::Note> required_notes {make_chord(
        768, {{SightRead::FIVE_FRET_RED, 1}, {SightRead::FIVE_FRET_OPEN, 0}})};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  required_notes.cbegin(),
                                  required_notes.cend());
}

BOOST_AUTO_TEST_CASE(resolution_is_positive)
{
    SightRead::SongGlobalData data;
    BOOST_CHECK_THROW([&] { data.resolution(0); }(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(empty_sp_phrases_are_culled)
{
    std::vector<SightRead::Note> notes {make_note(768)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {100}},
        {SightRead::Tick {700}, SightRead::Tick {100}},
        {SightRead::Tick {1000}, SightRead::Tick {100}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::StarPower> required_phrases {
        {SightRead::Tick {700}, SightRead::Tick {100}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_are_sorted)
{
    std::vector<SightRead::Note> notes {make_note(768), make_note(1000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {1000}, SightRead::Tick {1}},
        {SightRead::Tick {768}, SightRead::Tick {1}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::StarPower> required_phrases {
        {SightRead::Tick {768}, SightRead::Tick {1}},
        {SightRead::Tick {1000}, SightRead::Tick {1}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_do_not_overlap)
{
    std::vector<SightRead::Note> notes {make_note(768), make_note(1000),
                                        make_note(1500)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {768}, SightRead::Tick {1000}},
        {SightRead::Tick {900}, SightRead::Tick {150}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::StarPower> required_phrases {
        {SightRead::Tick {768}, SightRead::Tick {282}},
        {SightRead::Tick {1050}, SightRead::Tick {718}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        track.sp_phrases().cbegin(), track.sp_phrases().cend(),
        required_phrases.cbegin(), required_phrases.cend());
}

BOOST_AUTO_TEST_CASE(solos_are_sorted)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(768)};
    std::vector<SightRead::Solo> solos {
        {SightRead::Tick {768}, SightRead::Tick {868}, 100},
        {SightRead::Tick {0}, SightRead::Tick {100}, 100}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    track.solos(solos);
    std::vector<SightRead::Solo> required_solos {
        {SightRead::Tick {0}, SightRead::Tick {100}, 100},
        {SightRead::Tick {768}, SightRead::Tick {868}, 100}};
    std::vector<SightRead::Solo> solo_output
        = track.solos(SightRead::DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(solo_output.cbegin(), solo_output.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(solos_do_take_into_account_drum_settings)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(0, SightRead::DRUM_DOUBLE_KICK),
        make_drum_note(192, SightRead::DRUM_DOUBLE_KICK)};
    std::vector<SightRead::Solo> solos {
        {SightRead::Tick {0}, SightRead::Tick {1}, 200},
        {SightRead::Tick {192}, SightRead::Tick {193}, 100}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    track.solos(solos);
    std::vector<SightRead::Solo> required_solos {
        {SightRead::Tick {0}, SightRead::Tick {1}, 100}};
    std::vector<SightRead::Solo> solo_output
        = track.solos({false, false, true, false});

    BOOST_CHECK_EQUAL_COLLECTIONS(solo_output.cbegin(), solo_output.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_SUITE(automatic_drum_activation_zone_generation_is_correct)

BOOST_AUTO_TEST_CASE(automatic_zones_are_created)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(768), make_drum_note(1536), make_drum_note(2304),
        make_drum_note(3072), make_drum_note(3840)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {384}, SightRead::Tick {384}},
        {SightRead::Tick {3456}, SightRead::Tick {384}}};

    track.generate_drum_fills({});

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_have_250ms_of_leniency)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(672), make_drum_note(3936), make_drum_note(6815),
        make_drum_note(10081)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {384}, SightRead::Tick {384}},
        {SightRead::Tick {3456}, SightRead::Tick {384}}};

    track.generate_drum_fills({});

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_handle_skipped_measures_correctly)
{
    std::vector<SightRead::Note> notes {make_drum_note(768),
                                        make_drum_note(4608)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {384}, SightRead::Tick {384}},
        {SightRead::Tick {4224}, SightRead::Tick {384}}};

    track.generate_drum_fills({});

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(the_last_automatic_zone_exists_even_if_the_note_is_early)
{
    std::vector<SightRead::Note> notes {make_drum_note(760)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {384}, SightRead::Tick {384}}};

    track.generate_drum_fills({});

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(automatic_zones_are_half_a_measure_according_to_seconds)
{
    std::vector<SightRead::Note> notes {make_drum_note(768)};
    SightRead::TempoMap tempo_map {
        {}, {{SightRead::Tick {576}, 40000}}, {}, 192};

    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::Drums, global_data};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {576}, SightRead::Tick {192}}};

    track.generate_drum_fills(tempo_map);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(fill_ends_remain_snapped_to_measure)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(758),  make_drum_note(770),  make_drum_note(3830),
        make_drum_note(3860), make_drum_note(6900), make_drum_note(6924)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {384}, SightRead::Tick {384}},
        {SightRead::Tick {3456}, SightRead::Tick {384}},
        {SightRead::Tick {6528}, SightRead::Tick {384}}};

    track.generate_drum_fills({});

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(base_score_for_average_multiplier_is_correct)

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_without_sustains)
{
    std::vector<SightRead::Note> notes {
        make_note(192),
        make_chord(
            384,
            {{SightRead::FIVE_FRET_GREEN, 0}, {SightRead::FIVE_FRET_RED, 0}})};

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 150);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_with_sustains)
{
    std::vector<SightRead::Note> notes_one {make_note(192, 192)};
    std::vector<SightRead::Note> notes_two {make_note(192, 92)};
    std::vector<SightRead::Note> notes_three {make_note(192, 93)};

    SightRead::NoteTrack track_one {
        notes_one,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    SightRead::NoteTrack track_two {
        notes_two,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    SightRead::NoteTrack track_three {
        notes_three,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};

    BOOST_CHECK_EQUAL(track_one.base_score(), 75);
    BOOST_CHECK_EQUAL(track_two.base_score(), 62);
    BOOST_CHECK_EQUAL(track_three.base_score(), 63);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_songs_with_chord_sustains)
{
    std::vector<SightRead::Note> notes {
        make_note(192, 192), make_note(192, 192, SightRead::FIVE_FRET_RED)};

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 125);
}

BOOST_AUTO_TEST_CASE(base_score_is_correct_for_other_resolutions)
{
    std::vector<SightRead::Note> notes {make_note(192, 192)};

    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, make_resolution(480)};

    BOOST_CHECK_EQUAL(track.base_score(), 60);
}

BOOST_AUTO_TEST_CASE(fractional_ticks_from_multiple_holds_are_added_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0, 100), make_note(192, 100)};

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 127);
}

BOOST_AUTO_TEST_CASE(disjoint_chords_are_handled_correctly)
{
    std::vector<SightRead::Note> notes {
        make_note(0, 384), make_note(0, 384, SightRead::FIVE_FRET_RED),
        make_note(0, 192, SightRead::FIVE_FRET_YELLOW)};

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 275);
}

BOOST_AUTO_TEST_CASE(base_score_is_correctly_handled_with_open_note_merging)
{
    std::vector<SightRead::Note> notes {
        make_note(0, 0), make_note(0, 0, SightRead::FIVE_FRET_OPEN)};

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};

    BOOST_CHECK_EQUAL(track.base_score(), 50);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(base_score_is_correct_for_drums)

BOOST_AUTO_TEST_CASE(all_kicks_gives_correct_answer)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_KICK),
        make_drum_note(384, SightRead::DRUM_DOUBLE_KICK)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    SightRead::DrumSettings settings {true, false, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 150);
}

BOOST_AUTO_TEST_CASE(only_single_kicks_gives_correct_answer)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_KICK),
        make_drum_note(384, SightRead::DRUM_DOUBLE_KICK)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    SightRead::DrumSettings settings {false, false, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 100);
}

BOOST_AUTO_TEST_CASE(no_kicks_gives_correct_answer)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_KICK),
        make_drum_note(384, SightRead::DRUM_DOUBLE_KICK)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    SightRead::DrumSettings settings {false, true, true, false};

    BOOST_CHECK_EQUAL(track.base_score(settings), 50);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(trim_sustains_is_correct)
{
    const std::vector<SightRead::Note> notes {
        make_note(0, 65), make_note(200, 70), make_note(400, 140)};
    const SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, make_resolution(200)};
    const auto new_track = track.trim_sustains();
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].lengths[0], SightRead::Tick {0});
    BOOST_CHECK_EQUAL(new_notes[1].lengths[0], SightRead::Tick {70});
    BOOST_CHECK_EQUAL(new_notes[2].lengths[0], SightRead::Tick {140});
    BOOST_CHECK_EQUAL(new_track.base_score(), 177);
}

BOOST_AUTO_TEST_SUITE(snap_chords_is_correct)

BOOST_AUTO_TEST_CASE(no_snapping)
{
    const std::vector<SightRead::Note> notes {
        make_note(0, 0, SightRead::FIVE_FRET_GREEN),
        make_note(5, 0, SightRead::FIVE_FRET_RED)};
    const SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, make_resolution(480)};
    auto new_track = track.snap_chords(SightRead::Tick {0});
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes[0].position, SightRead::Tick {0});
    BOOST_CHECK_EQUAL(new_notes[1].position, SightRead::Tick {5});
}

BOOST_AUTO_TEST_CASE(hmx_gh_snapping)
{
    const std::vector<SightRead::Note> notes {
        make_note(0, 0, SightRead::FIVE_FRET_GREEN),
        make_note(5, 0, SightRead::FIVE_FRET_RED)};
    const SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, make_resolution(480)};
    auto new_track = track.snap_chords(SightRead::Tick {10});
    const auto& new_notes = new_track.notes();

    BOOST_CHECK_EQUAL(new_notes.size(), 1);
    BOOST_CHECK_EQUAL(new_notes[0].position, SightRead::Tick {0});
    BOOST_CHECK_EQUAL(new_notes[0].colours(), 1 | 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(disable_dynamics_is_correct)
{
    const std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_RED, SightRead::FLAGS_GHOST),
        make_drum_note(384, SightRead::DRUM_RED, SightRead::FLAGS_ACCENT)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    track.disable_dynamics();

    const std::vector<SightRead::Note> new_notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_RED),
        make_drum_note(384, SightRead::DRUM_RED)};
    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  new_notes.cbegin(), new_notes.cend());
}
