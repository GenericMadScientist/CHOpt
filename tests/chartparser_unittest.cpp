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

BOOST_AUTO_TEST_CASE(chart_to_song_has_correct_value_for_is_from_midi)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);

    BOOST_TEST(!song.global_data().is_from_midi());
}

BOOST_AUTO_TEST_SUITE(chart_reads_resolution)

BOOST_AUTO_TEST_CASE(default_is_192_resolution)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    const auto global_data = ChartParser({}).parse(chart).global_data();
    const auto resolution = global_data.resolution();

    BOOST_CHECK_EQUAL(resolution, 192);
}

BOOST_AUTO_TEST_CASE(default_is_overriden_by_specified_value)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    ChartSection header {
        "Song", {{"Resolution", "200"}, {"Offset", "100"}}, {}, {}, {}, {}, {}};
    std::vector<ChartSection> sections {header, expert_single};
    const Chart chart {sections};

    const auto global_data = ChartParser({}).parse(chart).global_data();
    const auto resolution = global_data.resolution();

    BOOST_CHECK_EQUAL(resolution, 200);
}

BOOST_AUTO_TEST_CASE(bad_values_are_ignored)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    ChartSection header {"Song", {{"Resolution", "\"480\""}}, {}, {}, {}, {},
                         {}};
    std::vector<ChartSection> sections {header, expert_single};
    const Chart chart {sections};

    const auto global_data = ChartParser({}).parse(chart).global_data();
    const auto resolution = global_data.resolution();

    BOOST_CHECK_EQUAL(resolution, 192);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(chart_header_values_besides_resolution_are_discarded)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    ChartSection header_section {"Song",
                                 {{"Name", "\"TestName\""},
                                  {"Artist", "\"GMS\""},
                                  {"Charter", "\"NotGMS\""}},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {}};
    std::vector<ChartSection> sections {header_section, expert_single};
    const Chart chart {sections};
    const auto song = ChartParser({}).parse(chart);

    BOOST_CHECK_NE(song.global_data().name(), "TestName");
    BOOST_CHECK_NE(song.global_data().artist(), "GMS");
    BOOST_CHECK_NE(song.global_data().charter(), "NotGMS");
}

BOOST_AUTO_TEST_CASE(ini_values_are_used_for_converting_from_chart_files)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    ChartSection header_section {"Song", {}, {}, {}, {}, {}, {}};
    std::vector<ChartSection> sections {header_section, expert_single};
    const Chart chart {sections};
    const IniValues ini {"TestName", "GMS", "NotGMS"};
    const auto song = ChartParser(ini).parse(chart);

    BOOST_CHECK_EQUAL(song.global_data().name(), "TestName");
    BOOST_CHECK_EQUAL(song.global_data().artist(), "GMS");
    BOOST_CHECK_EQUAL(song.global_data().charter(), "NotGMS");
}

BOOST_AUTO_TEST_CASE(chart_reads_sync_track_correctly)
{
    ChartSection sync_track {"SyncTrack", {}, {{0, 200000}},           {},
                             {},          {}, {{0, 4, 2}, {768, 4, 1}}};
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {sync_track, expert_single};
    const Chart chart {sections};
    std::vector<TimeSignature> time_sigs {{Tick {0}, 4, 4}, {Tick {768}, 4, 2}};
    std::vector<BPM> bpms {{Tick {0}, 200000}};

    const auto global_data = ChartParser({}).parse(chart).global_data();
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
    ChartSection sync_track {"SyncTrack", {}, {}, {}, {}, {}, {{0, 4, 32}}};
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {sync_track, expert_single};
    const Chart chart {sections};
    const ChartParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.parse(chart); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(chart_reads_easy_note_track_correctly)
{
    ChartSection easy_single {"EasySingle",    {}, {}, {}, {{768, 0, 0}},
                              {{768, 2, 100}}, {}};
    std::vector<ChartSection> sections {easy_single};
    const Chart chart {sections};
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN)};
    std::vector<StarPower> sp_phrases {{Tick {768}, Tick {100}}};

    const auto song = ChartParser({}).parse(chart);
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
    ChartSection expert_single {
        "ExpertSingle", {}, {}, {}, {{768, 0, 0}, {768, 13, 0}}, {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN)};

    const auto song = ChartParser({}).parse(chart);
    const auto& parsed_notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_are_read_correctly_from_chart)
{
    ChartSection expert_single {"ExpertSingle",  {}, {}, {}, {{768, 0, 0}},
                                {{768, 1, 100}}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    NoteTrack note_track {{make_note(768, 0, FIVE_FRET_GREEN)},
                          {{Tick {768}, Tick {100}}},
                          {},
                          {},
                          {},
                          {},
                          TrackType::FiveFret,
                          std::make_shared<SongGlobalData>()};

    const auto song = ChartParser({}).parse(chart);

    BOOST_TEST(song.track(Instrument::Guitar, Difficulty::Expert)
                   .sp_phrases()
                   .empty());
}

BOOST_AUTO_TEST_SUITE(chart_does_not_need_sections_in_usual_order)

BOOST_AUTO_TEST_CASE(non_note_sections_need_not_be_present)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    const ChartParser parser {{}};

    BOOST_CHECK_NO_THROW([&] { return parser.parse(chart); }());
}

BOOST_AUTO_TEST_CASE(at_least_one_nonempty_note_section_must_be_present)
{
    ChartSection expert_single {"ExpertSingle",  {}, {}, {}, {},
                                {{768, 2, 100}}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    const ChartParser parser {{}};

    BOOST_CHECK_THROW([&] { return parser.parse({}); }(), ParseError);
    BOOST_CHECK_THROW([&] { return parser.parse(chart); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(non_note_sections_can_be_in_any_order)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    ChartSection sync_track {"SyncTrack", {}, {{0, 200000}}, {}, {}, {}, {}};
    ChartSection header {"Song", {{"Resolution", "200"}}, {}, {}, {}, {}, {}};
    std::vector<ChartSection> sections {sync_track, expert_single, header};
    const Chart chart {sections};
    std::vector<Note> notes {make_note(768)};
    std::vector<BPM> expected_bpms {{Tick {0}, 200000}};

    const auto song = ChartParser({}).parse(chart);
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
    ChartSection expert_single_one {"ExpertSingle", {}, {}, {},
                                    {{768, 0, 0}},  {}, {}};
    ChartSection expert_single_two {"ExpertSingle", {}, {}, {},
                                    {{768, 1, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single_one, expert_single_two};
    const Chart chart {sections};
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_GREEN)};

    const auto song = ChartParser({}).parse(chart);
    const auto& parsed_notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(leading_empty_sections_are_ignored)
{
    ChartSection expert_single_one {"ExpertSingle", {}, {}, {}, {}, {}, {}};
    ChartSection expert_single_two {"ExpertSingle", {}, {}, {},
                                    {{768, 1, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single_one, expert_single_two};
    const Chart chart {sections};
    std::vector<Note> notes {make_note(768, 0, FIVE_FRET_RED)};

    const auto song = ChartParser({}).parse(chart);
    const auto& parsed_notes
        = song.track(Instrument::Guitar, Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(solos_are_read_properly)

BOOST_AUTO_TEST_CASE(expected_solos_are_read_properly)
{
    ChartSection expert_single {
        "ExpertSingle",
        {},
        {},
        {{0, "solo"}, {200, "soloend"}, {300, "solo"}, {400, "soloend"}},
        {{100, 0, 0}, {300, 0, 0}, {400, 0, 0}},
        {},
        {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    std::vector<Solo> required_solos {{Tick {0}, Tick {200}, 100},
                                      {Tick {300}, Tick {400}, 200}};

    const auto song = ChartParser({}).parse(chart);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(chords_are_not_counted_double)
{
    ChartSection expert_single {"ExpertSingle",
                                {},
                                {},
                                {{0, "solo"}, {200, "soloend"}},
                                {{100, 0, 0}, {100, 1, 0}},
                                {},
                                {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    std::vector<Solo> required_solos {{Tick {0}, Tick {200}, 100}};

    const auto song = ChartParser({}).parse(chart);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(empty_solos_are_ignored)
{
    ChartSection expert_single {
        "ExpertSingle", {}, {}, {{100, "solo"}, {200, "soloend"}},
        {{0, 0, 0}},    {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);

    BOOST_TEST(song.track(Instrument::Guitar, Difficulty::Expert)
                   .solos(DrumSettings::default_settings())
                   .empty());
}

BOOST_AUTO_TEST_CASE(repeated_solo_starts_and_ends_dont_matter)
{
    ChartSection expert_single {
        "ExpertSingle",
        {},
        {},
        {{0, "solo"}, {100, "solo"}, {200, "soloend"}, {300, "soloend"}},
        {{100, 0, 0}},
        {},
        {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    std::vector<Solo> required_solos {{Tick {0}, Tick {200}, 100}};

    const auto song = ChartParser({}).parse(chart);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(solo_markers_are_sorted)
{
    ChartSection expert_single {
        "ExpertSingle", {}, {}, {{384, "soloend"}, {0, "solo"}},
        {{192, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    std::vector<Solo> required_solos {{Tick {0}, Tick {384}, 100}};

    const auto song = ChartParser({}).parse(chart);
    const auto parsed_solos = song.track(Instrument::Guitar, Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  required_solos.cbegin(),
                                  required_solos.cend());
}

BOOST_AUTO_TEST_CASE(solos_with_no_soloend_event_are_ignored)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {{0, "solo"}},
                                {{192, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);

    BOOST_TEST(song.track(Instrument::Guitar, Difficulty::Expert)
                   .solos(DrumSettings::default_settings())
                   .empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(instruments_returns_the_supported_instruments)
{
    ChartSection guitar {"ExpertSingle", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    ChartSection bass {"ExpertDoubleBass", {}, {}, {}, {}, {}, {}};
    ChartSection drums {"ExpertDrums", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    std::vector<ChartSection> sections {guitar, bass, drums};
    const Chart chart {sections};
    std::vector<Instrument> instruments {Instrument::Guitar, Instrument::Drums};

    const auto parsed_instruments = ChartParser({}).parse(chart).instruments();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_instruments.cbegin(),
                                  parsed_instruments.cend(),
                                  instruments.cbegin(), instruments.cend());
}

BOOST_AUTO_TEST_CASE(difficulties_returns_the_difficulties_for_an_instrument)
{
    ChartSection guitar {"ExpertSingle", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    ChartSection hard_guitar {"HardSingle", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    ChartSection drums {"ExpertDrums", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    std::vector<ChartSection> sections {guitar, hard_guitar, drums};
    const Chart chart {sections};
    std::vector<Difficulty> guitar_difficulties {Difficulty::Hard,
                                                 Difficulty::Expert};
    std::vector<Difficulty> drum_difficulties {Difficulty::Expert};

    const auto song = ChartParser({}).parse(chart);
    const auto guitar_diffs = song.difficulties(Instrument::Guitar);
    const auto drum_diffs = song.difficulties(Instrument::Drums);

    BOOST_CHECK_EQUAL_COLLECTIONS(guitar_diffs.cbegin(), guitar_diffs.cend(),
                                  guitar_difficulties.cbegin(),
                                  guitar_difficulties.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(drum_diffs.cbegin(), drum_diffs.cend(),
                                  drum_difficulties.cbegin(),
                                  drum_difficulties.cend());
}

BOOST_AUTO_TEST_SUITE(other_five_fret_instruments_are_read_from_chart)

BOOST_AUTO_TEST_CASE(guitar_coop_is_read)
{
    ChartSection expert_double {"ExpertDoubleGuitar", {}, {}, {},
                                {{192, 0, 0}},        {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);

    BOOST_CHECK_NO_THROW([&] {
        return song.track(Instrument::GuitarCoop, Difficulty::Expert);
    }());
}

BOOST_AUTO_TEST_CASE(bass_is_read)
{
    ChartSection expert_double {"ExpertDoubleBass", {}, {}, {},
                                {{192, 0, 0}},      {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Bass, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(rhythm_is_read)
{
    ChartSection expert_double {"ExpertDoubleRhythm", {}, {}, {},
                                {{192, 0, 0}},        {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Rhythm, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(keys_is_read)
{
    ChartSection expert_double {"ExpertKeyboard", {}, {}, {},
                                {{192, 0, 0}},    {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);

    BOOST_CHECK_NO_THROW(
        [&] { return song.track(Instrument::Keys, Difficulty::Expert); }());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(six_fret_instruments_are_read_correctly_from_chart)

BOOST_AUTO_TEST_CASE(six_fret_guitar_is_read_correctly)
{
    ChartSection expert_double {"ExpertGHLGuitar",          {}, {}, {},
                                {{192, 0, 0}, {384, 3, 0}}, {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};
    std::vector<Note> notes {make_ghl_note(192, 0, SIX_FRET_WHITE_LOW),
                             make_ghl_note(384, 0, SIX_FRET_BLACK_LOW)};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::GHLGuitar, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_bass_is_read_correctly)
{
    ChartSection expert_double {
        "ExpertGHLBass", {}, {}, {}, {{192, 0, 0}, {384, 3, 0}}, {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};
    std::vector<Note> notes {make_ghl_note(192, 0, SIX_FRET_WHITE_LOW),
                             make_ghl_note(384, 0, SIX_FRET_BLACK_LOW)};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::GHLBass, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(drum_notes_are_read_correctly_from_chart)
{
    ChartSection expert_drums {
        "ExpertDrums",
        {},
        {},
        {},
        {{192, 1, 0}, {384, 2, 0}, {384, 66, 0}, {768, 32, 0}},
        {},
        {}};
    std::vector<ChartSection> sections {expert_drums};
    const Chart chart {sections};
    std::vector<Note> notes {make_drum_note(192, DRUM_RED),
                             make_drum_note(384, DRUM_YELLOW, FLAGS_CYMBAL),
                             make_drum_note(768, DRUM_DOUBLE_KICK)};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(dynamics_are_read_correctly_from_chart)
{
    ChartSection expert_drums {
        "ExpertDrums",
        {},
        {},
        {},
        {{192, 1, 0}, {192, 34, 0}, {384, 1, 0}, {384, 40, 0}},
        {},
        {}};
    std::vector<ChartSection> sections {expert_drums};
    const Chart chart {sections};
    std::vector<Note> notes {make_drum_note(192, DRUM_RED, FLAGS_ACCENT),
                             make_drum_note(384, DRUM_RED, FLAGS_GHOST)};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(drum_solos_are_read_correctly_from_chart)
{
    ChartSection expert_drums {"ExpertDrums",
                               {},
                               {},
                               {{0, "solo"}, {200, "soloend"}},
                               {{192, 1, 0}, {192, 2, 0}},
                               {},
                               {}};
    std::vector<ChartSection> sections {expert_drums};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.solos(DrumSettings::default_settings())[0].value,
                      200);
}

BOOST_AUTO_TEST_CASE(fifth_lane_notes_are_read_correctly_from_chart)
{
    ChartSection expert_drums {"ExpertDrums",
                               {},
                               {},
                               {},
                               {{192, 5, 0}, {384, 4, 0}, {384, 5, 0}},
                               {},
                               {}};
    std::vector<ChartSection> sections {expert_drums};
    const Chart chart {sections};
    std::vector<Note> notes {make_drum_note(192, DRUM_GREEN),
                             make_drum_note(384, DRUM_GREEN),
                             make_drum_note(384, DRUM_BLUE)};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(invalid_drum_notes_are_ignored)
{
    ChartSection drums {
        "ExpertDrums", {}, {}, {}, {{192, 1, 0}, {192, 6, 0}}, {}, {}};
    std::vector<ChartSection> sections {drums};
    const Chart chart {sections};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.notes().size(), 1);
}

BOOST_AUTO_TEST_CASE(drum_fills_are_read_from_chart)
{
    ChartSection drums {"ExpertDrums",  {}, {}, {}, {{192, 1, 0}},
                        {{192, 64, 1}}, {}};
    std::vector<ChartSection> sections {drums};
    const Chart chart {sections};
    const DrumFill fill {Tick {192}, Tick {1}};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.drum_fills().size(), 1);
    BOOST_CHECK_EQUAL(track.drum_fills()[0], fill);
}

BOOST_AUTO_TEST_CASE(disco_flips_are_read_from_chart)
{
    ChartSection drums {
        "ExpertDrums", {}, {}, {{100, "mix_3_drums0d"}, {105, "mix_3_drums0"}},
        {{192, 1, 0}}, {}, {}};
    std::vector<ChartSection> sections {drums};
    const Chart chart {sections};
    const DiscoFlip flip {Tick {100}, Tick {5}};

    const auto song = ChartParser({}).parse(chart);
    const auto& track = song.track(Instrument::Drums, Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.disco_flips().size(), 1);
    BOOST_CHECK_EQUAL(track.disco_flips()[0], flip);
}
