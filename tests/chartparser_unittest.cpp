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

#include "chartparser.hpp"
#include "test_helpers.hpp"

namespace {
std::string section_string(const std::string& section_type,
                           const std::vector<NoteEvent>& note_events,
                           const std::vector<SpecialEvent>& special_events = {},
                           const std::vector<Event>& events = {})
{
    using namespace std::string_literals;

    auto section = "["s + section_type + "]\n{\n";
    for (const auto& note : note_events) {
        section += "    ";
        section += std::to_string(note.position);
        section += " = N ";
        section += std::to_string(note.fret);
        section += ' ';
        section += std::to_string(note.length);
        section += '\n';
    }
    for (const auto& event : special_events) {
        section += "    ";
        section += std::to_string(event.position);
        section += " = S ";
        section += std::to_string(event.key);
        section += ' ';
        section += std::to_string(event.length);
        section += '\n';
    }
    for (const auto& event : events) {
        section += "    ";
        section += std::to_string(event.position);
        section += " = E ";
        section += event.data;
        section += '\n';
    }
    section += '}';
    return section;
}

std::string
header_string(const std::map<std::string, std::string>& key_value_pairs)
{
    using namespace std::string_literals;

    auto section = "[Song]\n{\n"s;
    for (const auto& [key, value] : key_value_pairs) {
        section += "    ";
        section += key;
        section += " = ";
        section += value;
        section += '\n';
    }
    section += '}';
    return section;
}

std::string sync_track_string(const std::vector<BpmEvent>& bpm_events,
                              const std::vector<TimeSigEvent>& ts_events)
{
    using namespace std::string_literals;

    auto section = "[SyncTrack]\n{\n"s;
    for (const auto& bpm : bpm_events) {
        section += "    ";
        section += std::to_string(bpm.position);
        section += " = B ";
        section += std::to_string(bpm.bpm);
        section += '\n';
    }
    for (const auto& ts : ts_events) {
        section += "    ";
        section += std::to_string(ts.position);
        section += " = TS ";
        section += std::to_string(ts.numerator);
        section += ' ';
        section += std::to_string(ts.denominator);
        section += '\n';
    }
    section += '}';
    return section;
}
}

BOOST_AUTO_TEST_CASE(chart_to_song_has_correct_value_for_is_from_midi)
{
    const auto chart_file = section_string("ExpertSingle", {{768, 0, 0}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_TEST(!song.global_data().is_from_midi());
}

BOOST_AUTO_TEST_SUITE(chart_reads_resolution)

BOOST_AUTO_TEST_CASE(default_is_192_resolution)
{
    const auto chart_file = section_string("ExpertSingle", {{768, 0, 0}});

    const auto global_data = ChartParser({}).parse(chart_file).global_data();
    const auto resolution = global_data.resolution();

    BOOST_CHECK_EQUAL(resolution, 192);
}

BOOST_AUTO_TEST_CASE(default_is_overriden_by_specified_value)
{
    const auto header
        = header_string({{"Resolution", "200"}, {"Offset", "100"}});
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto chart_file = header + '\n' + guitar_track;

    const auto global_data = ChartParser({}).parse(chart_file).global_data();
    const auto resolution = global_data.resolution();

    BOOST_CHECK_EQUAL(resolution, 200);
}

BOOST_AUTO_TEST_CASE(bad_values_are_ignored)
{
    const auto header = header_string({{"Resolution", "\"480\""}});
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto chart_file = header + '\n' + guitar_track;

    const auto global_data = ChartParser({}).parse(chart_file).global_data();
    const auto resolution = global_data.resolution();

    BOOST_CHECK_EQUAL(resolution, 192);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(chart_header_values_besides_resolution_are_discarded)
{
    const auto header = header_string({{"Name", "\"TestName\""},
                                       {"Artist", "\"GMS\""},
                                       {"Charter", "\"NotGMS\""}});
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto chart_file = header + '\n' + guitar_track;

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_CHECK_NE(song.global_data().name(), "TestName");
    BOOST_CHECK_NE(song.global_data().artist(), "GMS");
    BOOST_CHECK_NE(song.global_data().charter(), "NotGMS");
}

BOOST_AUTO_TEST_CASE(ini_values_are_used_for_converting_from_chart_files)
{
    const auto header = header_string({});
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto chart_file = header + '\n' + guitar_track;
    const IniValues ini {"TestName", "GMS", "NotGMS"};

    const auto song = ChartParser(ini).parse(chart_file);

    BOOST_CHECK_EQUAL(song.global_data().name(), "TestName");
    BOOST_CHECK_EQUAL(song.global_data().artist(), "GMS");
    BOOST_CHECK_EQUAL(song.global_data().charter(), "NotGMS");
}

BOOST_AUTO_TEST_CASE(chart_reads_sync_track_correctly)
{
    const auto sync_track
        = sync_track_string({{0, 200000}}, {{0, 4, 2}, {768, 4, 1}});
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto chart_file = sync_track + '\n' + guitar_track;

    std::vector<TimeSignature> time_sigs {{Tick {0}, 4, 4}, {Tick {768}, 4, 2}};
    std::vector<BPM> bpms {{Tick {0}, 200000}};

    const auto global_data = ChartParser({}).parse(chart_file).global_data();
    const auto& tempo_map = global_data.tempo_map();

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.time_sigs().cbegin(),
                                  tempo_map.time_sigs().cend(),
                                  time_sigs.cbegin(), time_sigs.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.bpms().cbegin(),
                                  tempo_map.bpms().cend(), bpms.cbegin(),
                                  bpms.cend());
}

BOOST_AUTO_TEST_CASE(large_time_sig_denominators_cause_an_exception)
{
    const auto sync_track = sync_track_string({}, {{0, 4, 32}});
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto chart_file = sync_track + '\n' + guitar_track;

    const ChartParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.parse(chart_file); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(easy_note_track_read_correctly)
{
    const auto chart_file
        = section_string("EasySingle", {{768, 0, 0}}, {{768, 2, 100}});
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN)};
    std::vector<StarPower> sp_phrases {{Tick {768}, Tick {100}}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Guitar, Difficulty::Easy);

    BOOST_CHECK_EQUAL(track.global_data().resolution(), 192);
    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(track.sp_phrases().cbegin(),
                                  track.sp_phrases().cend(),
                                  sp_phrases.cbegin(), sp_phrases.cend());
}

BOOST_AUTO_TEST_CASE(invalid_note_values_are_ignored)
{
    const auto chart_file
        = section_string("ExpertSingle", {{768, 0, 0}, {768, 13, 0}});
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& parsed_notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(non_sp_phrase_special_events_are_ignored)
{
    const auto chart_file
        = section_string("ExpertSingle", {{768, 0, 0}}, {{768, 1, 100}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_TEST(song.track(Instrument::Guitar, Difficulty::Expert)
                   .sp_phrases()
                   .empty());
}

BOOST_AUTO_TEST_SUITE(chart_does_not_need_sections_in_usual_order)

BOOST_AUTO_TEST_CASE(non_note_sections_need_not_be_present)
{
    const auto chart_file = section_string("ExpertSingle", {{768, 0, 0}});

    const ChartParser parser {{}};

    BOOST_CHECK_NO_THROW([&] { return parser.parse(chart_file); }());
}

BOOST_AUTO_TEST_CASE(at_least_one_nonempty_note_section_must_be_present)
{
    const auto chart_file = section_string("ExpertSingle", {}, {{768, 2, 100}});

    const ChartParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.parse({}); }(), ParseError);
    BOOST_CHECK_THROW([&] { return parser.parse(chart_file); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(non_note_sections_can_be_in_any_order)
{
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto sync_track = sync_track_string({{0, 200000}}, {});
    const auto header = header_string({{"Resolution", "200"}});
    const auto chart_file = sync_track + '\n' + guitar_track + '\n' + header;

    std::vector<Note> notes {make_note(768)};
    std::vector<BPM> expected_bpms {{Tick {0}, 200000}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& parsed_notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();
    const auto bpms = song.global_data().tempo_map().bpms();

    BOOST_CHECK_EQUAL(song.global_data().resolution(), 200);
    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(bpms.cbegin(), bpms.cend(),
                                  expected_bpms.cbegin(), expected_bpms.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(only_first_nonempty_part_of_note_sections_matter)

BOOST_AUTO_TEST_CASE(later_nonempty_sections_are_ignored)
{
    const auto guitar_track_one = section_string("ExpertSingle", {{768, 0, 0}});
    const auto guitar_track_two = section_string("ExpertSingle", {{768, 1, 0}});
    const auto chart_file = guitar_track_one + '\n' + guitar_track_two;
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& parsed_notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(leading_empty_sections_are_ignored)
{
    const auto guitar_track_one = section_string("ExpertSingle", {});
    const auto guitar_track_two = section_string("ExpertSingle", {{768, 1, 0}});
    const auto chart_file = guitar_track_one + '\n' + guitar_track_two;
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_RED)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& parsed_notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(solos_are_read_properly)

BOOST_AUTO_TEST_CASE(expected_solos_are_read_properly)
{
    const auto chart_file = section_string(
        "ExpertSingle", {{100, 0, 0}, {300, 0, 0}, {400, 0, 0}}, {},
        {{0, "solo"}, {200, "soloend"}, {300, "solo"}, {400, "soloend"}});
    std::vector<Solo> required_solos {{Tick {0}, Tick {200}, 100},
                                      {Tick {300}, Tick {400}, 200}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(chords_are_not_counted_double)
{
    const auto chart_file
        = section_string("ExpertSingle", {{100, 0, 0}, {100, 1, 0}}, {},
                         {{0, "solo"}, {200, "soloend"}});
    std::vector<Solo> required_solos {{Tick {0}, Tick {200}, 100}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(empty_solos_are_ignored)
{
    const auto chart_file = section_string("ExpertSingle", {{0, 0, 0}}, {},
                                           {{100, "solo"}, {200, "soloend"}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_TEST(song.track(Instrument::Guitar, Difficulty::Expert)
                   .solos(DrumSettings::default_settings())
                   .empty());
}

BOOST_AUTO_TEST_CASE(repeated_solo_starts_and_ends_dont_matter)
{
    const auto chart_file = section_string(
        "ExpertSingle", {{100, 0, 0}}, {},
        {{0, "solo"}, {100, "solo"}, {200, "soloend"}, {300, "soloend"}});
    std::vector<Solo> required_solos {{Tick {0}, Tick {200}, 100}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(solo_markers_are_sorted)
{
    const auto chart_file = section_string("ExpertSingle", {{192, 0, 0}}, {},
                                           {{384, "soloend"}, {0, "solo"}});
    std::vector<Solo> required_solos {{Tick {0}, Tick {384}, 100}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(solos_with_no_soloend_event_are_ignored)
{
    const auto chart_file
        = section_string("ExpertSingle", {{192, 0, 0}}, {}, {{0, "solo"}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_TEST(song.track(Instrument::Guitar, Difficulty::Expert)
                   .solos(DrumSettings::default_settings())
                   .empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(other_five_fret_instruments_are_read_from_chart)

BOOST_AUTO_TEST_CASE(guitar_coop_is_read)
{
    const auto chart_file = section_string("ExpertDoubleGuitar", {{192, 0, 0}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_CHECK_NO_THROW([&] {
        return song.track(Instrument::GuitarCoop, Difficulty::Expert);
    }());
}

BOOST_AUTO_TEST_CASE(bass_is_read)
{
    const auto chart_file = section_string("ExpertDoubleBass", {{192, 0, 0}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Bass, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(rhythm_is_read)
{
    const auto chart_file = section_string("ExpertDoubleRhythm", {{192, 0, 0}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Rhythm, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(keys_is_read)
{
    const auto chart_file = section_string("ExpertKeyboard", {{192, 0, 0}});

    const auto song = ChartParser({}).parse(chart_file);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Keys, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(six_fret_instruments_are_read_correctly_from_chart)

BOOST_AUTO_TEST_CASE(six_fret_guitar_is_read_correctly)
{
    const auto chart_file
        = section_string("ExpertGHLGuitar", {{192, 0, 0}, {384, 3, 0}});
    const std::vector<Note> notes {make_ghl_note(192, 0, SIX_FRET_WHITE_LOW),
                                   make_ghl_note(384, 0, SIX_FRET_BLACK_LOW)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::GHLGuitar, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_bass_is_read_correctly)
{
    const auto chart_file
        = section_string("ExpertGHLBass", {{192, 0, 0}, {384, 3, 0}});
    const std::vector<Note> notes {make_ghl_note(192, 0, SIX_FRET_WHITE_LOW),
                                   make_ghl_note(384, 0, SIX_FRET_BLACK_LOW)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::GHLBass, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(drum_notes_are_read_correctly_from_chart)
{
    const auto chart_file = section_string(
        "ExpertDrums", {{192, 1, 0}, {384, 2, 0}, {384, 66, 0}, {768, 32, 0}});
    std::vector<Note> notes {make_drum_note(192, DRUM_RED),
                             make_drum_note(384, DRUM_YELLOW, FLAGS_CYMBAL),
                             make_drum_note(768, DRUM_DOUBLE_KICK)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(dynamics_are_read_correctly_from_chart)
{
    const auto chart_file = section_string(
        "ExpertDrums", {{192, 1, 0}, {192, 34, 0}, {384, 1, 0}, {384, 40, 0}});
    std::vector<Note> notes {make_drum_note(192, DRUM_RED, FLAGS_ACCENT),
                             make_drum_note(384, DRUM_RED, FLAGS_GHOST)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(drum_solos_are_read_correctly_from_chart)
{
    const auto chart_file
        = section_string("ExpertDrums", {{192, 1, 0}, {192, 2, 0}}, {},
                         {{0, "solo"}, {200, "soloend"}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.solos(DrumSettings::default_settings())[0].value,
                      200);
}

BOOST_AUTO_TEST_CASE(fifth_lane_notes_are_read_correctly_from_chart)
{
    const auto chart_file = section_string(
        "ExpertDrums", {{192, 5, 0}, {384, 4, 0}, {384, 5, 0}});
    const std::vector<Note> notes {make_drum_note(192, DRUM_GREEN),
                                   make_drum_note(384, DRUM_GREEN),
                                   make_drum_note(384, DRUM_BLUE)};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(invalid_drum_notes_are_ignored)
{
    const auto chart_file
        = section_string("ExpertDrums", {{192, 1, 0}, {192, 6, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.notes().size(), 1);
}

BOOST_AUTO_TEST_CASE(drum_fills_are_read_from_chart)
{
    const auto chart_file
        = section_string("ExpertDrums", {{192, 1, 0}}, {{192, 64, 1}});
    const DrumFill fill {Tick {192}, Tick {1}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.drum_fills().size(), 1);
    BOOST_CHECK_EQUAL(track.drum_fills()[0], fill);
}

BOOST_AUTO_TEST_CASE(disco_flips_are_read_from_chart)
{
    const auto chart_file
        = section_string("ExpertDrums", {{192, 1, 0}}, {},
                         {{100, "mix_3_drums0d"}, {105, "mix_3_drums0"}});
    const DiscoFlip flip {Tick {100}, Tick {5}};

    const auto song = ChartParser({}).parse(chart_file);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.disco_flips().size(), 1);
    BOOST_CHECK_EQUAL(track.disco_flips()[0], flip);
}

BOOST_AUTO_TEST_CASE(instruments_not_permitted_are_dropped_from_charts)
{
    const auto guitar_track = section_string("ExpertSingle", {{768, 0, 0}});
    const auto bass_track = section_string("ExpertDoubleBass", {{192, 0, 0}});
    const auto chart_file = guitar_track + '\n' + bass_track;
    const std::vector<Instrument> expected_instruments {Instrument::Guitar};

    const auto parser
        = ChartParser({}).permit_instruments({Instrument::Guitar});
    const auto song = parser.parse(chart_file);

    BOOST_CHECK_EQUAL(song.instruments(), expected_instruments);
}

BOOST_AUTO_TEST_CASE(solos_ignored_from_charts_if_not_permitted)
{
    const auto chart_file = section_string(
        "ExpertSingle", {{100, 0, 0}, {300, 0, 0}, {400, 0, 0}}, {},
        {{0, "solo"}, {200, "soloend"}, {300, "solo"}, {400, "soloend"}});

    const auto parser = ChartParser({}).parse_solos(false);
    const auto song = parser.parse(chart_file);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK(parsed_solos.empty());
}

BOOST_AUTO_TEST_SUITE(hopos_and_taps)

BOOST_AUTO_TEST_CASE(automatically_set_based_on_distance)
{
    const auto chart_file
        = section_string("ExpertSingle", {{0, 0, 0}, {65, 1, 0}, {131, 2, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags, FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[2].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(does_not_do_it_on_same_note)
{
    const auto chart_file
        = section_string("ExpertSingle", {{0, 0, 0}, {65, 0, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(forcing_is_handled_correctly)
{
    const auto chart_file = section_string(
        "ExpertSingle", {{0, 0, 0}, {0, 5, 0}, {65, 1, 0}, {65, 5, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(chords_are_not_hopos_due_to_proximity)
{
    const auto chart_file
        = section_string("ExpertSingle", {{0, 0, 0}, {64, 1, 0}, {64, 2, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(chords_can_be_forced)
{
    const auto chart_file = section_string(
        "ExpertSingle", {{0, 0, 0}, {64, 1, 0}, {64, 2, 0}, {64, 5, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(taps_are_read)
{
    const auto chart_file
        = section_string("ExpertSingle", {{0, 0, 0}, {0, 6, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags, FLAGS_TAP | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(taps_take_precedence_over_hopos)
{
    const auto chart_file
        = section_string("ExpertSingle", {{0, 0, 0}, {64, 1, 0}, {64, 6, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_TAP | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(chords_can_be_taps)
{
    const auto chart_file = section_string(
        "ExpertSingle", {{0, 0, 0}, {64, 1, 0}, {64, 2, 0}, {64, 6, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_TAP | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(other_resolutions_are_handled_correctly)
{
    const auto header = header_string({{"Resolution", "480"}});
    const auto guitar_track
        = section_string("ExpertSingle", {{0, 0, 0}, {162, 1, 0}, {325, 2, 0}});
    const auto chart_file = header + '\n' + guitar_track;

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[2].flags, FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(custom_hopo_threshold_is_handled_correctly)
{
    const auto chart_file
        = section_string("ExpertSingle", {{0, 0, 0}, {65, 1, 0}, {131, 2, 0}});

    const auto song
        = ChartParser({})
              .hopo_threshold({HopoThresholdType::HopoFrequency, Tick {96}})
              .parse(chart_file);
    const auto notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
    BOOST_CHECK_EQUAL(notes[2].flags, FLAGS_HOPO | FLAGS_FIVE_FRET_GUITAR);
}

BOOST_AUTO_TEST_CASE(not_done_on_drums)
{
    const auto chart_file = section_string(
        "ExpertDrums", {{0, 0, 0}, {65, 1, 0}, {131, 2, 0}, {131, 6, 0}});

    const auto song = ChartParser({}).parse(chart_file);
    const auto notes
        = song.track(Instrument::Drums, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].flags, FLAGS_DRUMS);
    BOOST_CHECK_EQUAL(notes[1].flags, FLAGS_DRUMS);
    BOOST_CHECK_EQUAL(notes[2].flags, FLAGS_DRUMS);
}

BOOST_AUTO_TEST_SUITE_END()
