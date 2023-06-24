/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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

#include "midiparser.hpp"
#include "test_helpers.hpp"

namespace {
MetaEvent part_event(std::string_view name)
{
    std::vector<std::uint8_t> bytes {name.cbegin(), name.cend()};
    return MetaEvent {3, bytes};
}
}

BOOST_AUTO_TEST_CASE(midi_to_song_has_correct_value_for_is_from_midi)
{
    const Midi midi {192, {}};

    const auto song = MidiParser({}).from_midi(midi);

    BOOST_TEST(song.global_data().is_from_midi());
}

BOOST_AUTO_TEST_SUITE(midi_resolution_is_read_correctly)

BOOST_AUTO_TEST_CASE(midi_resolution_is_read)
{
    const Midi midi {200, {}};

    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_EQUAL(song.global_data().resolution(), 200);
}

BOOST_AUTO_TEST_CASE(resolution_gt_zero_invariant_is_upheld)
{
    const Midi midi {0, {}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(first_track_is_read_correctly)

BOOST_AUTO_TEST_CASE(tempos_are_read_correctly)
{
    MidiTrack tempo_track {{{0, {MetaEvent {0x51, {6, 0x1A, 0x80}}}},
                            {1920, {MetaEvent {0x51, {4, 0x93, 0xE0}}}}}};
    const Midi midi {192, {tempo_track}};
    const std::vector<SightRead::BPM> bpms {{SightRead::Tick {0}, 150000},
                                            {SightRead::Tick {1920}, 200000}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& tempo_map = song.global_data().tempo_map();

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.bpms().cbegin(),
                                  tempo_map.bpms().cend(), bpms.cbegin(),
                                  bpms.cend());
}

BOOST_AUTO_TEST_CASE(too_short_tempo_events_cause_an_exception)
{
    MidiTrack tempo_track {{{0, {MetaEvent {0x51, {6, 0x1A}}}}}};
    const Midi midi {192, {tempo_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_CASE(time_signatures_are_read_correctly)
{
    MidiTrack ts_track {{{0, {MetaEvent {0x58, {6, 2, 24, 8}}}},
                         {1920, {MetaEvent {0x58, {3, 3, 24, 8}}}}}};
    const Midi midi {192, {ts_track}};
    const std::vector<SightRead::TimeSignature> tses {
        {SightRead::Tick {0}, 6, 4}, {SightRead::Tick {1920}, 3, 8}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& tempo_map = song.global_data().tempo_map();

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.time_sigs().cbegin(),
                                  tempo_map.time_sigs().cend(), tses.cbegin(),
                                  tses.cend());
}

BOOST_AUTO_TEST_CASE(time_signatures_with_large_denominators_cause_an_exception)
{
    MidiTrack ts_track {{{0, {MetaEvent {0x58, {6, 32, 24, 8}}}}}};
    const Midi midi {192, {ts_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_CASE(too_short_time_sig_events_cause_an_exception)
{
    MidiTrack ts_track {{{0, {MetaEvent {0x58, {6}}}}}};
    const Midi midi {192, {ts_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_CASE(song_name_is_not_read_from_midi)
{
    MidiTrack name_track {{{0, {MetaEvent {1, {72, 101, 108, 108, 111}}}}}};
    const Midi midi {192, {name_track}};

    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_NE(song.global_data().name(), "Hello");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(ini_values_are_used_when_converting_mid_files)
{
    const Midi midi {192, {}};
    const IniValues ini {"TestName", "GMS", "NotGMS"};

    const auto song = MidiParser(ini).from_midi(midi);

    BOOST_CHECK_EQUAL(song.global_data().name(), "TestName");
    BOOST_CHECK_EQUAL(song.global_data().artist(), "GMS");
    BOOST_CHECK_EQUAL(song.global_data().charter(), "NotGMS");
}

BOOST_AUTO_TEST_SUITE(notes_are_read_from_mids_correctly)

BOOST_AUTO_TEST_CASE(notes_of_every_difficulty_are_read)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {768, {MidiEvent {0x90, {84, 64}}}},
                           {768, {MidiEvent {0x90, {72, 64}}}},
                           {768, {MidiEvent {0x90, {60, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}},
                           {960, {MidiEvent {0x80, {84, 0}}}},
                           {960, {MidiEvent {0x80, {72, 0}}}},
                           {960, {MidiEvent {0x80, {60, 0}}}}}};
    const Midi midi {192, {note_track}};
    const std::vector<Note> green_note {make_note(768, 192, FIVE_FRET_GREEN)};
    const std::array<Difficulty, 4> diffs {Difficulty::Easy, Difficulty::Medium,
                                           Difficulty::Hard,
                                           Difficulty::Expert};

    const auto song = MidiParser({}).from_midi(midi);

    for (auto diff : diffs) {
        const auto& notes = song.track(Instrument::Guitar, diff).notes();
        BOOST_CHECK_EQUAL_COLLECTIONS(notes.cbegin(), notes.cend(),
                                      green_note.cbegin(), green_note.cend());
    }
}

BOOST_AUTO_TEST_CASE(notes_are_read_from_part_guitar)
{
    MidiTrack other_track {{{768, {MidiEvent {0x90, {96, 64}}}},
                            {960, {MidiEvent {0x80, {96, 0}}}}}};
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {97, 64}}}},
                           {960, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {other_track, note_track}};

    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_EQUAL(
        song.track(Instrument::Guitar, Difficulty::Expert).notes()[0].colours(),
        1 << FIVE_FRET_RED);
}

BOOST_AUTO_TEST_CASE(part_guitar_event_need_not_be_the_first_event)
{
    MidiTrack note_track {
        {{0, {MetaEvent {0x7F, {0x05, 0x0F, 0x09, 0x08, 0x40}}}},
         {0, {part_event("PART GUITAR")}},
         {768, {MidiEvent {0x90, {97, 64}}}},
         {960, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_EQUAL(
        song.track(Instrument::Guitar, Difficulty::Expert).notes()[0].colours(),
        1 << FIVE_FRET_RED);
}

BOOST_AUTO_TEST_CASE(guitar_notes_are_also_read_from_t1_gems)
{
    MidiTrack other_track {{{768, {MidiEvent {0x90, {96, 64}}}},
                            {960, {MidiEvent {0x80, {96, 0}}}}}};
    MidiTrack note_track {{{0, {part_event("T1 GEMS")}},
                           {768, {MidiEvent {0x90, {97, 64}}}},
                           {960, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {other_track, note_track}};

    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_EQUAL(
        song.track(Instrument::Guitar, Difficulty::Expert).notes()[0].colours(),
        1 << FIVE_FRET_RED);
}

BOOST_AUTO_TEST_CASE(note_on_events_must_have_a_corresponding_note_off_event)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {960, {MidiEvent {0x80, {96, 64}}}},
                           {1152, {MidiEvent {0x90, {96, 64}}}}}};
    const Midi midi {192, {note_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_CASE(corresponding_note_off_events_are_after_note_on_events)
{
    MidiTrack note_track {{
        {0, {part_event("PART GUITAR")}},
        {480, {MidiEvent {0x80, {96, 64}}}},
        {480, {MidiEvent {0x90, {96, 64}}}},
        {960, {MidiEvent {0x80, {96, 64}}}},
        {960, {MidiEvent {0x90, {96, 64}}}},
        {1440, {MidiEvent {0x80, {96, 64}}}},
    }};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 2);
    BOOST_CHECK_EQUAL(notes[0].lengths[0], SightRead::Tick {480});
}

BOOST_AUTO_TEST_CASE(note_on_events_with_velocity_zero_count_as_note_off_events)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {960, {MidiEvent {0x90, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_NO_THROW([&] { return parser.from_midi(midi); }());
}

BOOST_AUTO_TEST_CASE(
    note_on_events_with_no_intermediate_note_off_events_are_not_merged)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {769, {MidiEvent {0x90, {96, 64}}}},
                           {800, {MidiEvent {0x80, {96, 64}}}},
                           {801, {MidiEvent {0x80, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 2);
}

BOOST_AUTO_TEST_CASE(each_note_on_event_consumes_the_following_note_off_event)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {769, {MidiEvent {0x90, {96, 64}}}},
                           {800, {MidiEvent {0x80, {96, 64}}}},
                           {1000, {MidiEvent {0x80, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 2);
    BOOST_CHECK_GT(notes[1].lengths[0], SightRead::Tick {0});
}

BOOST_AUTO_TEST_CASE(note_off_events_can_be_zero_ticks_after_the_note_on_events)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {768, {MidiEvent {0x80, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 1);
}

BOOST_AUTO_TEST_CASE(
    parseerror_thrown_if_note_on_has_no_corresponding_note_off_track)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {96, 64}}}}}};
    const Midi midi {192, {note_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_CASE(open_notes_are_read_correctly)
{
    MidiTrack note_track {
        {{0, {part_event("PART GUITAR")}},
         {768, {MidiEvent {0x90, {96, 64}}}},
         {768, {SysexEvent {{0x50, 0x53, 0, 0, 3, 1, 1, 0xF7}}}},
         {770, {SysexEvent {{0x50, 0x53, 0, 0, 3, 1, 0, 0xF7}}}},
         {960, {MidiEvent {0x90, {96, 0}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_EQUAL(
        song.track(Instrument::Guitar, Difficulty::Expert).notes()[0].colours(),
        1 << FIVE_FRET_OPEN);
}

BOOST_AUTO_TEST_CASE(parseerror_thrown_if_open_note_ons_have_no_note_offs)
{
    MidiTrack note_track {
        {{0, {part_event("PART GUITAR")}},
         {768, {MidiEvent {0x90, {96, 64}}}},
         {768, {SysexEvent {{0x50, 0x53, 0, 0, 3, 1, 1, 0xF7}}}},
         {960, {MidiEvent {0x90, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

// Note that a note at the very end of a solo event is not considered part of
// the solo for a .mid, but it is for a .chart.
BOOST_AUTO_TEST_CASE(solos_are_read_from_mids_correctly)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {103, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {900, {MidiEvent {0x90, {97, 64}}}},
                           {900, {MidiEvent {0x80, {103, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}},
                           {960, {MidiEvent {0x80, {97, 64}}}}}};
    const Midi midi {192, {note_track}};
    const std::vector<Solo> solos {
        {SightRead::Tick {768}, SightRead::Tick {900}, 100}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto parsed_solos
        = song.track(Instrument::Guitar, Difficulty::Expert)
              .solos(SightRead::DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  solos.cbegin(), solos.cend());
}

BOOST_AUTO_TEST_SUITE(star_power_is_read)

BOOST_AUTO_TEST_CASE(a_single_phrase_is_read)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {116, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {900, {MidiEvent {0x80, {116, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const std::vector<StarPower> sp_phrases {
        {SightRead::Tick {768}, SightRead::Tick {132}}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& parsed_sp
        = song.track(Instrument::Guitar, Difficulty::Expert).sp_phrases();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_sp.cbegin(), parsed_sp.cend(),
                                  sp_phrases.cbegin(), sp_phrases.cend());
}

BOOST_AUTO_TEST_CASE(note_off_event_is_required_for_every_phrase)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {116, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const MidiParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.from_midi(midi); }(),
                      SightRead::ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(mids_with_multiple_solos_and_no_sp_have_solos_read_as_sp)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {103, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {800, {MidiEvent {0x80, {96, 64}}}},
                           {900, {MidiEvent {0x80, {103, 64}}}},
                           {950, {MidiEvent {0x90, {103, 64}}}},
                           {960, {MidiEvent {0x90, {97, 64}}}},
                           {1000, {MidiEvent {0x80, {97, 64}}}},
                           {1000, {MidiEvent {0x80, {103, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Guitar, Difficulty::Expert);

    BOOST_TEST(
        track.solos(SightRead::DrumSettings::default_settings()).empty());
    BOOST_CHECK_EQUAL(track.sp_phrases().size(), 2);
}

// This should be done by NoteTrack's trim_sustains method.
BOOST_AUTO_TEST_CASE(short_midi_sustains_are_not_trimmed)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}},
                           {100, {MidiEvent {0x90, {96, 64}}}},
                           {170, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {200, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].lengths[0], SightRead::Tick {65});
    BOOST_CHECK_EQUAL(notes[1].lengths[0], SightRead::Tick {70});
}

BOOST_AUTO_TEST_SUITE(midi_hopos_and_taps)

BOOST_AUTO_TEST_CASE(automatically_set_based_on_distance)
{
    MidiTrack note_track {{
        {0, {part_event("PART GUITAR")}},
        {0, {MidiEvent {0x90, {96, 64}}}},
        {1, {MidiEvent {0x80, {96, 0}}}},
        {161, {MidiEvent {0x90, {97, 64}}}},
        {162, {MidiEvent {0x80, {97, 0}}}},
        {323, {MidiEvent {0x90, {98, 64}}}},
        {324, {MidiEvent {0x80, {98, 0}}}},
    }};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags, FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[2].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(does_not_do_it_on_same_note)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {1, {MidiEvent {0x80, {96, 0}}}},
                           {161, {MidiEvent {0x90, {96, 64}}}},
                           {162, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(forcing_is_handled_correctly)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {0, {MidiEvent {0x90, {101, 64}}}},
                           {1, {MidiEvent {0x80, {96, 0}}}},
                           {1, {MidiEvent {0x80, {101, 0}}}},
                           {161, {MidiEvent {0x90, {97, 64}}}},
                           {161, {MidiEvent {0x90, {102, 64}}}},
                           {162, {MidiEvent {0x80, {97, 0}}}},
                           {162, {MidiEvent {0x80, {102, 0}}}}}};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags,
                      FLAGS_FORCE_HOPO | FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[1].flags,
                      FLAGS_FORCE_STRUM | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(chords_are_not_hopos_due_to_proximity)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {1, {MidiEvent {0x80, {96, 0}}}},
                           {161, {MidiEvent {0x90, {97, 64}}}},
                           {161, {MidiEvent {0x90, {98, 64}}}},
                           {162, {MidiEvent {0x80, {97, 0}}}},
                           {162, {MidiEvent {0x80, {98, 0}}}}}};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(chords_can_be_forced)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {1, {MidiEvent {0x80, {96, 0}}}},
                           {161, {MidiEvent {0x90, {97, 64}}}},
                           {161, {MidiEvent {0x90, {98, 64}}}},
                           {161, {MidiEvent {0x90, {101, 64}}}},
                           {162, {MidiEvent {0x80, {97, 0}}}},
                           {162, {MidiEvent {0x80, {98, 0}}}},
                           {162, {MidiEvent {0x80, {101, 0}}}}}};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags,
                      FLAGS_FORCE_HOPO | FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(taps_are_read)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {0, {MidiEvent {0x90, {104, 64}}}},
                           {1, {MidiEvent {0x80, {96, 0}}}},
                           {1, {MidiEvent {0x80, {104, 0}}}}}};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags, FLAGS_TAP | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(taps_take_precedence_over_hopos)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {1, {MidiEvent {0x80, {96, 0}}}},
                           {161, {MidiEvent {0x90, {97, 64}}}},
                           {161, {MidiEvent {0x90, {104, 64}}}},
                           {162, {MidiEvent {0x80, {97, 0}}}},
                           {162, {MidiEvent {0x80, {104, 0}}}}}};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_TAP | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(chords_can_be_taps)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {1, {MidiEvent {0x80, {96, 0}}}},
                           {160, {MidiEvent {0x90, {97, 64}}}},
                           {160, {MidiEvent {0x90, {98, 64}}}},
                           {160, {MidiEvent {0x90, {104, 64}}}},
                           {161, {MidiEvent {0x80, {97, 0}}}},
                           {161, {MidiEvent {0x80, {98, 0}}}},
                           {161, {MidiEvent {0x80, {104, 0}}}}}};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_TAP | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(other_resolutions_are_handled_correctly)
{
    MidiTrack note_track {{
        {0, {part_event("PART GUITAR")}},
        {0, {MidiEvent {0x90, {96, 64}}}},
        {1, {MidiEvent {0x80, {96, 0}}}},
        {65, {MidiEvent {0x90, {97, 64}}}},
        {66, {MidiEvent {0x80, {97, 0}}}},
        {131, {MidiEvent {0x90, {98, 64}}}},
        {132, {MidiEvent {0x80, {98, 0}}}},
    }};
    const Midi midi {192, {note_track}};

    const auto song = MidiParser({}).from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[2].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(custom_hopo_threshold_is_handled_correctly)
{
    MidiTrack note_track {{
        {0, {part_event("PART GUITAR")}},
        {0, {MidiEvent {0x90, {96, 64}}}},
        {1, {MidiEvent {0x80, {96, 0}}}},
        {161, {MidiEvent {0x90, {97, 64}}}},
        {162, {MidiEvent {0x80, {97, 0}}}},
        {323, {MidiEvent {0x90, {98, 64}}}},
        {324, {MidiEvent {0x80, {98, 0}}}},
    }};
    const Midi midi {480, {note_track}};

    const auto song = MidiParser({})
                          .hopo_threshold({HopoThresholdType::HopoFrequency,
                                           SightRead::Tick {240}})
                          .from_midi(midi);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[2].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(not_done_on_drums)
{
    MidiTrack note_track {{{0, {part_event("PART DRUMS")}},
                           {0, {MidiEvent {0x90, {97, 64}}}},
                           {1, {MidiEvent {0x80, {97, 64}}}},
                           {161, {MidiEvent {0x90, {98, 64}}}},
                           {162, {MidiEvent {0x80, {98, 0}}}}}};
    const Midi midi {480, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& notes
        = song.track(Instrument::Drums, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags, FLAGS_DRUMS);
    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_DRUMS | FLAGS_CYMBAL);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(other_five_fret_instruments_are_read_from_mid)

BOOST_AUTO_TEST_CASE(guitar_coop_is_read)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR COOP")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_NO_THROW([&] {
        return song.track(Instrument::GuitarCoop, Difficulty::Expert);
    }());
}

BOOST_AUTO_TEST_CASE(bass_is_read)
{
    MidiTrack note_track {{{0, {part_event("PART BASS")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Bass, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(rhythm_is_read)
{
    MidiTrack note_track {{{0, {part_event("PART RHYTHM")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Rhythm, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(keys_is_read)
{
    MidiTrack note_track {{{0, {part_event("PART KEYS")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Keys, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(six_fret_instruments_are_read_correctly_from_mid)

BOOST_AUTO_TEST_CASE(six_fret_guitar_is_read_correctly)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR GHL")}},
                           {0, {MidiEvent {0x90, {94, 64}}}},
                           {65, {MidiEvent {0x80, {94, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::GHLGuitar, Difficulty::Expert);

    std::vector<Note> notes {make_ghl_note(0, 65, SIX_FRET_OPEN)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_bass_is_read_correctly)
{
    MidiTrack note_track {{{0, {part_event("PART BASS GHL")}},
                           {0, {MidiEvent {0x90, {94, 64}}}},
                           {65, {MidiEvent {0x80, {94, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::GHLBass, Difficulty::Expert);

    std::vector<Note> notes {make_ghl_note(0, 65, SIX_FRET_OPEN)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_rhythm_is_read_correctly)
{
    MidiTrack note_track {{{0, {part_event("PART RHYTHM GHL")}},
                           {0, {MidiEvent {0x90, {94, 64}}}},
                           {65, {MidiEvent {0x80, {94, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::GHLRhythm, Difficulty::Expert);

    std::vector<Note> notes {make_ghl_note(0, 65, SIX_FRET_OPEN)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_guitar_coop_is_read_correctly)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR COOP GHL")}},
                           {0, {MidiEvent {0x90, {94, 64}}}},
                           {65, {MidiEvent {0x80, {94, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track
        = song.track(Instrument::GHLGuitarCoop, Difficulty::Expert);

    std::vector<Note> notes {make_ghl_note(0, 65, SIX_FRET_OPEN)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(drums_are_read_correctly_from_mid)
{
    MidiTrack note_track {{{0, {part_event("PART DRUMS")}},
                           {0, {MidiEvent {0x90, {98, 64}}}},
                           {0, {MidiEvent {0x90, {110, 64}}}},
                           {65, {MidiEvent {0x80, {98, 0}}}},
                           {65, {MidiEvent {0x80, {110, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<Note> notes {make_drum_note(0, DRUM_YELLOW)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(double_kicks_are_read_correctly_from_mid)
{
    MidiTrack note_track {{{0, {part_event("PART DRUMS")}},
                           {0, {MidiEvent {0x90, {95, 64}}}},
                           {65, {MidiEvent {0x80, {95, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<Note> notes {make_drum_note(0, DRUM_DOUBLE_KICK)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(drum_fills_are_read_correctly_from_mid)
{
    MidiTrack note_track {{{0, {part_event("PART DRUMS")}},
                           {0, {MidiEvent {0x90, {98, 64}}}},
                           {45, {MidiEvent {0x90, {120, 64}}}},
                           {65, {MidiEvent {0x80, {98, 0}}}},
                           {75, {MidiEvent {0x80, {120, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<DrumFill> fills {{SightRead::Tick {45}, SightRead::Tick {30}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(disco_flips_are_read_correctly_from_mid)
{
    MidiTrack note_track {
        {{0, {part_event("PART DRUMS")}},
         {15,
          {MetaEvent {1,
                      {0x5B, 0x6D, 0x69, 0x78, 0x20, 0x33, 0x20, 0x64, 0x72,
                       0x75, 0x6D, 0x73, 0x30, 0x64, 0x5D}}}},
         {45, {MidiEvent {0x90, {98, 64}}}},
         {65, {MidiEvent {0x80, {98, 0}}}},
         {75,
          {MetaEvent {1,
                      {0x5B, 0x6D, 0x69, 0x78, 0x20, 0x33, 0x20, 0x64, 0x72,
                       0x75, 0x6D, 0x73, 0x30, 0x5D}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<DiscoFlip> flips {{SightRead::Tick {15}, SightRead::Tick {60}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.disco_flips().cbegin(),
                                  track.disco_flips().cend(), flips.cbegin(),
                                  flips.cend());
}

BOOST_AUTO_TEST_CASE(missing_disco_flip_end_event_just_ends_at_max_int)
{
    MidiTrack note_track {
        {{0, {part_event("PART DRUMS")}},
         {15,
          {MetaEvent {1,
                      {0x5B, 0x6D, 0x69, 0x78, 0x20, 0x33, 0x20, 0x64, 0x72,
                       0x75, 0x6D, 0x73, 0x30, 0x64, 0x5D}}}},
         {45, {MidiEvent {0x90, {98, 64}}}},
         {65, {MidiEvent {0x80, {98, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<DiscoFlip> flips {
        {SightRead::Tick {15}, SightRead::Tick {2147483632}}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.disco_flips().cbegin(),
                                  track.disco_flips().cend(), flips.cbegin(),
                                  flips.cend());
}

BOOST_AUTO_TEST_CASE(drum_five_lane_to_four_lane_conversion_is_done_from_mid)
{
    MidiTrack note_track {{{0, {part_event("PART DRUMS")}},
                           {0, {MidiEvent {0x90, {101, 64}}}},
                           {1, {MidiEvent {0x80, {101, 0}}}},
                           {2, {MidiEvent {0x90, {100, 64}}}},
                           {3, {MidiEvent {0x80, {100, 0}}}},
                           {4, {MidiEvent {0x90, {101, 64}}}},
                           {4, {MidiEvent {0x90, {100, 64}}}},
                           {5, {MidiEvent {0x80, {101, 0}}}},
                           {5, {MidiEvent {0x80, {100, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<Note> notes {make_drum_note(0, DRUM_GREEN),
                             make_drum_note(2, DRUM_GREEN, FLAGS_CYMBAL),
                             make_drum_note(4, DRUM_BLUE),
                             make_drum_note(4, DRUM_GREEN, FLAGS_CYMBAL)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(dynamics_are_parsed_from_mid)
{
    MidiTrack note_track {
        {{0, {part_event("PART DRUMS")}},
         {0, {MetaEvent {1, {0x5B, 0x45, 0x4E, 0x41, 0x42, 0x4C, 0x45, 0x5F,
                             0x43, 0x48, 0x41, 0x52, 0x54, 0x5F, 0x44, 0x59,
                             0x4E, 0x41, 0x4D, 0x49, 0x43, 0x53, 0x5D}}}},
         {0, {MidiEvent {0x90, {97, 1}}}},
         {1, {MidiEvent {0x80, {97, 0}}}},
         {2, {MidiEvent {0x90, {97, 64}}}},
         {3, {MidiEvent {0x80, {97, 0}}}},
         {4, {MidiEvent {0x90, {97, 127}}}},
         {5, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<Note> notes {make_drum_note(0, DRUM_RED, FLAGS_GHOST),
                             make_drum_note(2, DRUM_RED),
                             make_drum_note(4, DRUM_RED, FLAGS_ACCENT)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(dynamics_not_parsed_from_mid_without_ENABLE_CHART_DYNAMICS)
{
    MidiTrack note_track {{{0, {part_event("PART DRUMS")}},
                           {0, {MidiEvent {0x90, {97, 1}}}},
                           {1, {MidiEvent {0x80, {97, 0}}}},
                           {2, {MidiEvent {0x90, {97, 64}}}},
                           {3, {MidiEvent {0x80, {97, 0}}}},
                           {4, {MidiEvent {0x90, {97, 127}}}},
                           {5, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = MidiParser({}).from_midi(midi);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(2, DRUM_RED),
                             make_drum_note(4, DRUM_RED)};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(instruments_not_permitted_are_dropped_from_midis)
{
    MidiTrack guitar_track {{{0, {part_event("PART GUITAR")}},
                             {768, {MidiEvent {0x90, {97, 64}}}},
                             {960, {MidiEvent {0x80, {97, 0}}}}}};
    MidiTrack bass_track {{{0, {part_event("PART BASS")}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {guitar_track, bass_track}};
    const std::vector<Instrument> expected_instruments {Instrument::Guitar};

    const auto parser = MidiParser({}).permit_instruments({Instrument::Guitar});
    const auto song = parser.from_midi(midi);

    BOOST_CHECK_EQUAL(song.instruments(), expected_instruments);
}

BOOST_AUTO_TEST_CASE(solos_ignored_from_midis_if_not_permitted)
{
    MidiTrack note_track {{{0, {part_event("PART GUITAR")}},
                           {768, {MidiEvent {0x90, {103, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {900, {MidiEvent {0x90, {97, 64}}}},
                           {900, {MidiEvent {0x80, {103, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}},
                           {960, {MidiEvent {0x80, {97, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto parser = MidiParser({}).parse_solos(false);
    const auto song = parser.from_midi(midi);
    const auto parsed_solos
        = song.track(Instrument::Guitar, Difficulty::Expert)
              .solos(SightRead::DrumSettings::default_settings());

    BOOST_CHECK(parsed_solos.empty());
}
