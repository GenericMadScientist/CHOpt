/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022 Raymond Wright
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

#define BOOST_TEST_MODULE Song Tests
#include <boost/test/unit_test.hpp>

#include "song.hpp"

static bool operator!=(const BPM& lhs, const BPM& rhs)
{
    return std::tie(lhs.position, lhs.bpm) != std::tie(rhs.position, rhs.bpm);
}

static std::ostream& operator<<(std::ostream& stream, const BPM& bpm)
{
    stream << "{Pos " << bpm.position << ", BPM " << bpm.bpm << '}';
    return stream;
}

template <typename T>
static bool operator!=(const Note<T>& lhs, const Note<T>& rhs)
{
    return std::tie(lhs.position, lhs.length, lhs.colour)
        != std::tie(rhs.position, rhs.length, rhs.colour);
}

template <typename T>
static std::ostream& operator<<(std::ostream& stream, const Note<T>& note)
{
    stream << "{Pos " << note.position << ", Length " << note.length
           << ", Colour " << static_cast<int>(note.colour) << '}';
    return stream;
}

static bool operator!=(const Solo& lhs, const Solo& rhs)
{
    return std::tie(lhs.start, lhs.end, lhs.value)
        != std::tie(rhs.start, rhs.end, rhs.value);
}

static std::ostream& operator<<(std::ostream& stream, const Solo& solo)
{
    stream << "{Start " << solo.start << ", End " << solo.end << ", Value "
           << solo.value << '}';
    return stream;
}

static bool operator!=(const StarPower& lhs, const StarPower& rhs)
{
    return std::tie(lhs.position, lhs.length)
        != std::tie(rhs.position, rhs.length);
}

static std::ostream& operator<<(std::ostream& stream, const StarPower& sp)
{
    stream << "{Pos " << sp.position << ", Length " << sp.length << '}';
    return stream;
}

static bool operator!=(const TimeSignature& lhs, const TimeSignature& rhs)
{
    return std::tie(lhs.position, lhs.numerator, lhs.denominator)
        != std::tie(rhs.position, rhs.numerator, rhs.denominator);
}

static std::ostream& operator<<(std::ostream& stream, const TimeSignature& ts)
{
    stream << "{Pos " << ts.position << ", " << ts.numerator << '/'
           << ts.denominator << '}';
    return stream;
}

static bool operator==(const DrumFill& lhs, const DrumFill& rhs)
{
    return std::tie(lhs.position, lhs.length)
        == std::tie(rhs.position, rhs.length);
}

static bool operator!=(const DrumFill& lhs, const DrumFill& rhs)
{
    return !(lhs == rhs);
}

static std::ostream& operator<<(std::ostream& stream, const DrumFill& fill)
{
    stream << "{Pos " << fill.position << ", Length " << fill.length << '}';
    return stream;
}

static bool operator==(const DiscoFlip& lhs, const DiscoFlip& rhs)
{
    return std::tie(lhs.position, lhs.length)
        == std::tie(rhs.position, rhs.length);
}

static bool operator!=(const DiscoFlip& lhs, const DiscoFlip& rhs)
{
    return !(lhs == rhs);
}

static std::ostream& operator<<(std::ostream& stream, const DiscoFlip& flip)
{
    stream << "{Pos " << flip.position << ", Length " << flip.length << '}';
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, Difficulty diff)
{
    stream << static_cast<int>(diff);
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, Instrument inst)
{
    stream << static_cast<int>(inst);
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, NoteColour colour)
{
    stream << static_cast<int>(colour);
    return stream;
}

BOOST_AUTO_TEST_CASE(chart_to_song_has_correct_value_for_is_from_midi)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    const auto song = Song::from_chart(chart, {});

    BOOST_TEST(!song.is_from_midi());
}

BOOST_AUTO_TEST_SUITE(chart_reads_resolution)

BOOST_AUTO_TEST_CASE(default_is_192_resolution)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    const auto resolution = Song::from_chart(chart, {}).resolution();

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

    const auto resolution = Song::from_chart(chart, {}).resolution();

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

    const auto resolution = Song::from_chart(chart, {}).resolution();

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
    const auto song = Song::from_chart(chart, {});

    BOOST_CHECK_NE(song.name(), "TestName");
    BOOST_CHECK_NE(song.artist(), "GMS");
    BOOST_CHECK_NE(song.charter(), "NotGMS");
}

BOOST_AUTO_TEST_CASE(ini_values_are_used_for_converting_from_chart_files)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    ChartSection header_section {"Song", {}, {}, {}, {}, {}, {}};
    std::vector<ChartSection> sections {header_section, expert_single};
    const Chart chart {sections};
    const IniValues ini {"TestName", "GMS", "NotGMS"};
    const auto song = Song::from_chart(chart, ini);

    BOOST_CHECK_EQUAL(song.name(), "TestName");
    BOOST_CHECK_EQUAL(song.artist(), "GMS");
    BOOST_CHECK_EQUAL(song.charter(), "NotGMS");
}

BOOST_AUTO_TEST_CASE(chart_reads_sync_track_correctly)
{
    ChartSection sync_track {"SyncTrack", {}, {{0, 200000}},           {},
                             {},          {}, {{0, 4, 2}, {768, 4, 1}}};
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {sync_track, expert_single};
    const Chart chart {sections};
    std::vector<TimeSignature> time_sigs {{0, 4, 4}, {768, 4, 2}};
    std::vector<BPM> bpms {{0, 200000}};

    const auto chart_sync_track = Song::from_chart(chart, {}).sync_track();

    BOOST_CHECK_EQUAL_COLLECTIONS(chart_sync_track.time_sigs().cbegin(),
                                  chart_sync_track.time_sigs().cend(),
                                  time_sigs.cbegin(), time_sigs.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(chart_sync_track.bpms().cbegin(),
                                  chart_sync_track.bpms().cend(), bpms.cbegin(),
                                  bpms.cend());
}

BOOST_AUTO_TEST_CASE(large_time_sig_denominators_cause_an_exception)
{
    ChartSection sync_track {"SyncTrack", {}, {}, {}, {}, {}, {{0, 4, 32}}};
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {sync_track, expert_single};
    const Chart chart {sections};

    BOOST_CHECK_THROW([&] { return Song::from_chart(chart, {}); }(),
                      ParseError);
}

BOOST_AUTO_TEST_CASE(chart_reads_easy_note_track_correctly)
{
    ChartSection easy_single {"EasySingle",    {}, {}, {}, {{768, 0, 0}},
                              {{768, 2, 100}}, {}};
    std::vector<ChartSection> sections {easy_single};
    const Chart chart {sections};
    std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green}};
    std::vector<StarPower> sp_phrases {{768, 100}};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.guitar_note_track(Difficulty::Easy);

    BOOST_CHECK_EQUAL(track.resolution(), 192);
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
    std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green}};

    const auto song = Song::from_chart(chart, {});
    const auto& parsed_notes
        = song.guitar_note_track(Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(sp_phrases_are_read_correctly_from_chart)
{
    ChartSection expert_single {"ExpertSingle",  {}, {}, {}, {{768, 0, 0}},
                                {{768, 1, 100}}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    NoteTrack<NoteColour> note_track {
        {{768, 0, NoteColour::Green}}, {{768, 100}}, {}, {}, {}, {}, 192};

    const auto song = Song::from_chart(chart, {});

    BOOST_TEST(song.guitar_note_track(Difficulty::Expert).sp_phrases().empty());
}

BOOST_AUTO_TEST_SUITE(chart_does_not_need_sections_in_usual_order)

BOOST_AUTO_TEST_CASE(non_note_sections_need_not_be_present)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    BOOST_CHECK_NO_THROW([&] { return Song::from_chart(chart, {}); }());
}

BOOST_AUTO_TEST_CASE(at_least_one_nonempty_note_section_must_be_present)
{
    ChartSection expert_single {"ExpertSingle",  {}, {}, {}, {},
                                {{768, 2, 100}}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};

    BOOST_CHECK_THROW([] { return Song::from_chart({}, {}); }(), ParseError);
    BOOST_CHECK_THROW([&] { return Song::from_chart(chart, {}); }(),
                      ParseError);
}

BOOST_AUTO_TEST_CASE(non_note_sections_can_be_in_any_order)
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    ChartSection sync_track {"SyncTrack", {}, {{0, 200000}}, {}, {}, {}, {}};
    ChartSection header {"Song", {{"Resolution", "200"}}, {}, {}, {}, {}, {}};
    std::vector<ChartSection> sections {sync_track, expert_single, header};
    const Chart chart {sections};
    std::vector<Note<NoteColour>> notes {{768}};
    std::vector<BPM> bpms {{0, 200000}};

    const auto song = Song::from_chart(chart, {});
    const auto& parsed_notes
        = song.guitar_note_track(Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(song.resolution(), 200);
    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_notes.cbegin(), parsed_notes.cend(),
                                  notes.cbegin(), notes.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(song.sync_track().bpms().cbegin(),
                                  song.sync_track().bpms().cend(),
                                  bpms.cbegin(), bpms.cend());
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
    std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green}};

    const auto song = Song::from_chart(chart, {});
    const auto& parsed_notes
        = song.guitar_note_track(Difficulty::Expert).notes();

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
    std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Red}};

    const auto song = Song::from_chart(chart, {});
    const auto& parsed_notes
        = song.guitar_note_track(Difficulty::Expert).notes();

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
    std::vector<Solo> required_solos {{0, 200, 100}, {300, 400, 200}};

    const auto song = Song::from_chart(chart, {});
    const auto parsed_solos = song.guitar_note_track(Difficulty::Expert)
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
    std::vector<Solo> required_solos {{0, 200, 100}};

    const auto song = Song::from_chart(chart, {});
    const auto parsed_solos = song.guitar_note_track(Difficulty::Expert)
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

    const auto song = Song::from_chart(chart, {});

    BOOST_TEST(song.guitar_note_track(Difficulty::Expert)
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
    std::vector<Solo> required_solos {{0, 200, 100}};

    const auto song = Song::from_chart(chart, {});
    const auto parsed_solos = song.guitar_note_track(Difficulty::Expert)
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
    std::vector<Solo> required_solos {{0, 384, 100}};

    const auto song = Song::from_chart(chart, {});
    const auto parsed_solos = song.guitar_note_track(Difficulty::Expert)
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

    const auto song = Song::from_chart(chart, {});

    BOOST_TEST(song.guitar_note_track(Difficulty::Expert)
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

    const auto parsed_instruments = Song::from_chart(chart, {}).instruments();

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

    const auto song = Song::from_chart(chart, {});
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

    const auto song = Song::from_chart(chart, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.guitar_coop_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(bass_is_read)
{
    ChartSection expert_double {"ExpertDoubleBass", {}, {}, {},
                                {{192, 0, 0}},      {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};

    const auto song = Song::from_chart(chart, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.bass_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(rhythm_is_read)
{
    ChartSection expert_double {"ExpertDoubleRhythm", {}, {}, {},
                                {{192, 0, 0}},        {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};

    const auto song = Song::from_chart(chart, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.rhythm_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(keys_is_read)
{
    ChartSection expert_double {"ExpertKeyboard", {}, {}, {},
                                {{192, 0, 0}},    {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};

    const auto song = Song::from_chart(chart, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.keys_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(six_fret_instruments_are_read_correctly_from_chart)

BOOST_AUTO_TEST_CASE(six_fret_guitar_is_read_correctly)
{
    ChartSection expert_double {"ExpertGHLGuitar",          {}, {}, {},
                                {{192, 0, 0}, {384, 3, 0}}, {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};
    std::vector<Note<GHLNoteColour>> notes {{192, 0, GHLNoteColour::WhiteLow},
                                            {384, 0, GHLNoteColour::BlackLow}};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.ghl_guitar_note_track(Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_bass_is_read_correctly)
{
    ChartSection expert_double {
        "ExpertGHLBass", {}, {}, {}, {{192, 0, 0}, {384, 3, 0}}, {}, {}};
    std::vector<ChartSection> sections {expert_double};
    const Chart chart {sections};
    std::vector<Note<GHLNoteColour>> notes {{192, 0, GHLNoteColour::WhiteLow},
                                            {384, 0, GHLNoteColour::BlackLow}};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.ghl_bass_note_track(Difficulty::Expert);

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
    std::vector<Note<DrumNoteColour>> notes {
        {192, 0, DrumNoteColour::Red},
        {384, 0, DrumNoteColour::YellowCymbal},
        {768, 0, DrumNoteColour::DoubleKick}};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

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

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

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
    std::vector<Note<DrumNoteColour>> notes {{192, 0, DrumNoteColour::Green},
                                             {384, 0, DrumNoteColour::Blue},
                                             {384, 0, DrumNoteColour::Green}};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(invalid_drum_notes_are_ignored)
{
    ChartSection drums {
        "ExpertDrums", {}, {}, {}, {{192, 1, 0}, {192, 6, 0}}, {}, {}};
    std::vector<ChartSection> sections {drums};
    const Chart chart {sections};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.notes().size(), 1);
}

BOOST_AUTO_TEST_CASE(drum_fills_are_read_from_chart)
{
    ChartSection drums {"ExpertDrums",  {}, {}, {}, {{192, 1, 0}},
                        {{192, 64, 1}}, {}};
    std::vector<ChartSection> sections {drums};
    const Chart chart {sections};
    const DrumFill fill {192, 1};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

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
    const DiscoFlip flip {100, 5};

    const auto song = Song::from_chart(chart, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    BOOST_CHECK_EQUAL(track.disco_flips().size(), 1);
    BOOST_CHECK_EQUAL(track.disco_flips()[0], flip);
}

BOOST_AUTO_TEST_CASE(midi_to_song_has_correct_value_for_is_from_midi)
{
    const Midi midi {192, {}};

    const auto song = Song::from_midi(midi, {});

    BOOST_TEST(song.is_from_midi());
}

BOOST_AUTO_TEST_SUITE(midi_resolution_is_read_correctly)

BOOST_AUTO_TEST_CASE(midi_resolution_is_read)
{
    const Midi midi {200, {}};

    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_EQUAL(song.resolution(), 200);
}

BOOST_AUTO_TEST_CASE(resolution_gt_zero_invariant_is_upheld)
{
    const Midi midi {0, {}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(first_track_is_read_correctly)

BOOST_AUTO_TEST_CASE(tempos_are_read_correctly)
{
    MidiTrack tempo_track {{{0, {MetaEvent {0x51, {6, 0x1A, 0x80}}}},
                            {1920, {MetaEvent {0x51, {4, 0x93, 0xE0}}}}}};
    const Midi midi {192, {tempo_track}};
    SyncTrack tempos {{}, {{0, 150000}, {1920, 200000}}};

    const auto song = Song::from_midi(midi, {});
    const auto& sync_track = song.sync_track();

    BOOST_CHECK_EQUAL_COLLECTIONS(sync_track.bpms().cbegin(),
                                  sync_track.bpms().cend(),
                                  tempos.bpms().cbegin(), tempos.bpms().cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        tempos.time_sigs().cbegin(), tempos.time_sigs().cend());
}

BOOST_AUTO_TEST_CASE(too_short_tempo_events_cause_an_exception)
{
    MidiTrack tempo_track {{{0, {MetaEvent {0x51, {6, 0x1A}}}}}};
    const Midi midi {192, {tempo_track}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(time_signatures_are_read_correctly)
{
    MidiTrack ts_track {{{0, {MetaEvent {0x58, {6, 2, 24, 8}}}},
                         {1920, {MetaEvent {0x58, {3, 3, 24, 8}}}}}};
    const Midi midi {192, {ts_track}};
    SyncTrack tses {{{0, 6, 4}, {1920, 3, 8}}, {}};

    const auto song = Song::from_midi(midi, {});
    const auto& sync_track = song.sync_track();

    BOOST_CHECK_EQUAL_COLLECTIONS(sync_track.bpms().cbegin(),
                                  sync_track.bpms().cend(),
                                  tses.bpms().cbegin(), tses.bpms().cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        tses.time_sigs().cbegin(), tses.time_sigs().cend());
}

BOOST_AUTO_TEST_CASE(time_signatures_with_large_denominators_cause_an_exception)
{
    MidiTrack ts_track {{{0, {MetaEvent {0x58, {6, 32, 24, 8}}}}}};
    const Midi midi {192, {ts_track}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(too_short_time_sig_events_cause_an_exception)
{
    MidiTrack ts_track {{{0, {MetaEvent {0x58, {6}}}}}};
    const Midi midi {192, {ts_track}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(song_name_is_not_read_from_midi)
{
    MidiTrack name_track {{{0, {MetaEvent {1, {72, 101, 108, 108, 111}}}}}};
    const Midi midi {192, {name_track}};

    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_NE(song.name(), "Hello");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(ini_values_are_used_when_converting_mid_files)
{
    const Midi midi {192, {}};
    const IniValues ini {"TestName", "GMS", "NotGMS"};

    const auto song = Song::from_midi(midi, ini);

    BOOST_CHECK_EQUAL(song.name(), "TestName");
    BOOST_CHECK_EQUAL(song.artist(), "GMS");
    BOOST_CHECK_EQUAL(song.charter(), "NotGMS");
}

BOOST_AUTO_TEST_SUITE(notes_are_read_from_mids_correctly)

BOOST_AUTO_TEST_CASE(notes_of_every_difficulty_are_read)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {768, {MidiEvent {0x90, {84, 64}}}},
                           {768, {MidiEvent {0x90, {72, 64}}}},
                           {768, {MidiEvent {0x90, {60, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}},
                           {960, {MidiEvent {0x80, {84, 0}}}},
                           {960, {MidiEvent {0x80, {72, 0}}}},
                           {960, {MidiEvent {0x80, {60, 0}}}}}};
    const Midi midi {192, {note_track}};
    const std::vector<Note<NoteColour>> green_note {
        {768, 192, NoteColour::Green}};
    const std::array<Difficulty, 4> diffs {Difficulty::Easy, Difficulty::Medium,
                                           Difficulty::Hard,
                                           Difficulty::Expert};

    const auto song = Song::from_midi(midi, {});

    for (auto diff : diffs) {
        const auto& notes = song.guitar_note_track(diff).notes();
        BOOST_CHECK_EQUAL_COLLECTIONS(notes.cbegin(), notes.cend(),
                                      green_note.cbegin(), green_note.cend());
    }
}

BOOST_AUTO_TEST_CASE(notes_are_read_from_part_guitar)
{
    MidiTrack other_track {{{768, {MidiEvent {0x90, {96, 64}}}},
                            {960, {MidiEvent {0x80, {96, 0}}}}}};
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {97, 64}}}},
                           {960, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {other_track, note_track}};

    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_EQUAL(
        song.guitar_note_track(Difficulty::Expert).notes()[0].colour,
        NoteColour::Red);
}

BOOST_AUTO_TEST_CASE(part_guitar_event_need_not_be_the_first_event)
{
    MidiTrack note_track {
        {{0, {MetaEvent {0x7F, {0x05, 0x0F, 0x09, 0x08, 0x40}}}},
         {0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                       0x41, 0x52}}}},
         {768, {MidiEvent {0x90, {97, 64}}}},
         {960, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_EQUAL(
        song.guitar_note_track(Difficulty::Expert).notes()[0].colour,
        NoteColour::Red);
}

BOOST_AUTO_TEST_CASE(guitar_notes_are_also_read_from_t1_gems)
{
    MidiTrack other_track {{{768, {MidiEvent {0x90, {96, 64}}}},
                            {960, {MidiEvent {0x80, {96, 0}}}}}};
    MidiTrack note_track {
        {{0, {MetaEvent {3, {0x54, 0x31, 0x20, 0x47, 0x45, 0x4D, 0x53}}}},
         {768, {MidiEvent {0x90, {97, 64}}}},
         {960, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {other_track, note_track}};

    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_EQUAL(
        song.guitar_note_track(Difficulty::Expert).notes()[0].colour,
        NoteColour::Red);
}

BOOST_AUTO_TEST_CASE(note_on_events_must_have_a_corresponding_note_off_event)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {960, {MidiEvent {0x80, {96, 64}}}},
                           {1152, {MidiEvent {0x90, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(corresponding_note_off_events_are_after_note_on_events)
{
    MidiTrack note_track {{
        {0,
         {MetaEvent {3,
                     {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                      0x41, 0x52}}}},
        {480, {MidiEvent {0x80, {96, 64}}}},
        {480, {MidiEvent {0x90, {96, 64}}}},
        {960, {MidiEvent {0x80, {96, 64}}}},
        {960, {MidiEvent {0x90, {96, 64}}}},
        {1440, {MidiEvent {0x80, {96, 64}}}},
    }};
    const Midi midi {480, {note_track}};

    const auto song = Song::from_midi(midi, {});
    const auto& notes = song.guitar_note_track(Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 2);
    BOOST_CHECK_EQUAL(notes[0].length, 480);
}

BOOST_AUTO_TEST_CASE(note_on_events_with_velocity_zero_count_as_note_off_events)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {960, {MidiEvent {0x90, {96, 0}}}}}};
    const Midi midi {192, {note_track}};

    BOOST_CHECK_NO_THROW([&] { return Song::from_midi(midi, {}); }());
}

BOOST_AUTO_TEST_CASE(
    note_on_events_with_no_intermediate_note_off_events_are_not_merged)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {769, {MidiEvent {0x90, {96, 64}}}},
                           {800, {MidiEvent {0x80, {96, 64}}}},
                           {801, {MidiEvent {0x80, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = Song::from_midi(midi, {});
    const auto& notes = song.guitar_note_track(Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 2);
}

BOOST_AUTO_TEST_CASE(each_note_on_event_consumes_the_following_note_off_event)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {769, {MidiEvent {0x90, {96, 64}}}},
                           {800, {MidiEvent {0x80, {96, 64}}}},
                           {1000, {MidiEvent {0x80, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = Song::from_midi(midi, {});
    const auto& notes = song.guitar_note_track(Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 2);
    BOOST_CHECK_GT(notes[1].length, 0);
}

BOOST_AUTO_TEST_CASE(note_off_events_can_be_zero_ticks_after_the_note_on_events)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {768, {MidiEvent {0x80, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = Song::from_midi(midi, {});
    const auto& notes = song.guitar_note_track(Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes.size(), 1);
}

BOOST_AUTO_TEST_CASE(
    parseerror_thrown_if_note_on_has_no_corresponding_note_off_track)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}}}};
    const Midi midi {192, {note_track}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(open_notes_are_read_correctly)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                       0x41, 0x52}}}},
         {768, {MidiEvent {0x90, {96, 64}}}},
         {768, {SysexEvent {{0x50, 0x53, 0, 0, 3, 1, 1, 0xF7}}}},
         {770, {SysexEvent {{0x50, 0x53, 0, 0, 3, 1, 0, 0xF7}}}},
         {960, {MidiEvent {0x90, {96, 0}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_EQUAL(
        song.guitar_note_track(Difficulty::Expert).notes()[0].colour,
        NoteColour::Open);
}

BOOST_AUTO_TEST_CASE(parseerror_thrown_if_open_note_ons_have_no_note_offs)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                       0x41, 0x52}}}},
         {768, {MidiEvent {0x90, {96, 64}}}},
         {768, {SysexEvent {{0x50, 0x53, 0, 0, 3, 1, 1, 0xF7}}}},
         {960, {MidiEvent {0x90, {96, 0}}}}}};
    const Midi midi {192, {note_track}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

// Note that a note at the very end of a solo event is not considered part of
// the solo for a .mid, but it is for a .chart.
BOOST_AUTO_TEST_CASE(solos_are_read_from_mids_correctly)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {103, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {900, {MidiEvent {0x90, {97, 64}}}},
                           {900, {MidiEvent {0x80, {103, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}},
                           {960, {MidiEvent {0x80, {97, 64}}}}}};
    const Midi midi {192, {note_track}};
    const std::vector<Solo> solos {{768, 900, 100}};

    const auto song = Song::from_midi(midi, {});
    const auto parsed_solos = song.guitar_note_track(Difficulty::Expert)
                                  .solos(DrumSettings::default_settings());

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_solos.cbegin(), parsed_solos.cend(),
                                  solos.cbegin(), solos.cend());
}

BOOST_AUTO_TEST_SUITE(star_power_is_read)

BOOST_AUTO_TEST_CASE(a_single_phrase_is_read)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {116, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {900, {MidiEvent {0x80, {116, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const std::vector<StarPower> sp_phrases {{768, 132}};

    const auto song = Song::from_midi(midi, {});
    const auto& parsed_sp
        = song.guitar_note_track(Difficulty::Expert).sp_phrases();

    BOOST_CHECK_EQUAL_COLLECTIONS(parsed_sp.cbegin(), parsed_sp.cend(),
                                  sp_phrases.cbegin(), sp_phrases.cend());
}

BOOST_AUTO_TEST_CASE(note_off_event_is_required_for_every_phrase)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {116, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};

    BOOST_CHECK_THROW([&] { return Song::from_midi(midi, {}); }(), ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(mids_with_multiple_solos_and_no_sp_have_solos_read_as_sp)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {103, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {800, {MidiEvent {0x80, {96, 64}}}},
                           {900, {MidiEvent {0x80, {103, 64}}}},
                           {950, {MidiEvent {0x90, {103, 64}}}},
                           {960, {MidiEvent {0x90, {97, 64}}}},
                           {1000, {MidiEvent {0x80, {97, 64}}}},
                           {1000, {MidiEvent {0x80, {103, 64}}}}}};
    const Midi midi {192, {note_track}};

    const auto song = Song::from_midi(midi, {});
    const auto& track = song.guitar_note_track(Difficulty::Expert);

    BOOST_TEST(track.solos(DrumSettings::default_settings()).empty());
    BOOST_CHECK_EQUAL(track.sp_phrases().size(), 2);
}

// This should be done by NoteTrack's trim_sustains method.
BOOST_AUTO_TEST_CASE(short_midi_sustains_are_not_trimmed)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}},
                           {100, {MidiEvent {0x90, {96, 64}}}},
                           {170, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {200, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& notes = song.guitar_note_track(Difficulty::Expert).notes();

    BOOST_CHECK_EQUAL(notes[0].length, 65);
    BOOST_CHECK_EQUAL(notes[1].length, 70);
}

BOOST_AUTO_TEST_SUITE(other_five_fret_instruments_are_read_from_mid)

BOOST_AUTO_TEST_CASE(guitar_coop_is_read)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                       0x41, 0x52, 0x20, 0x43, 0x4F, 0x4F, 0x50}}}},
         {0, {MidiEvent {0x90, {96, 64}}}},
         {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.guitar_coop_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(bass_is_read)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x42, 0x41, 0x53, 0x53}}}},
         {0, {MidiEvent {0x90, {96, 64}}}},
         {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.bass_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(rhythm_is_read)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x52,
                                         0x48, 0x59, 0x54, 0x48, 0x4D}}}},
                           {0, {MidiEvent {0x90, {96, 64}}}},
                           {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.rhythm_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_CASE(keys_is_read)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x4B, 0x45, 0x59, 0x53}}}},
         {0, {MidiEvent {0x90, {96, 64}}}},
         {65, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});

    BOOST_CHECK_NO_THROW(
        [&] { return song.keys_note_track(Difficulty::Expert); }());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(six_fret_instruments_are_read_correctly_from_mid)

BOOST_AUTO_TEST_CASE(six_fret_guitar_is_read_correctly)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                       0x41, 0x52, 0x20, 0x47, 0x48, 0x4C}}}},
         {0, {MidiEvent {0x90, {94, 64}}}},
         {65, {MidiEvent {0x80, {94, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.ghl_guitar_note_track(Difficulty::Expert);

    std::vector<Note<GHLNoteColour>> notes {{0, 65, GHLNoteColour::Open}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_bass_is_read_correctly)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {3,
                      {0x50, 0x41, 0x52, 0x54, 0x20, 0x42, 0x41, 0x53, 0x53,
                       0x20, 0x47, 0x48, 0x4C}}}},
         {0, {MidiEvent {0x90, {94, 64}}}},
         {65, {MidiEvent {0x80, {94, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.ghl_bass_note_track(Difficulty::Expert);

    std::vector<Note<GHLNoteColour>> notes {{0, 65, GHLNoteColour::Open}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(drums_are_read_correctly_from_mid)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x44,
                                         0x52, 0x55, 0x4D, 0x53}}}},
                           {0, {MidiEvent {0x90, {98, 64}}}},
                           {0, {MidiEvent {0x90, {110, 64}}}},
                           {65, {MidiEvent {0x80, {98, 0}}}},
                           {65, {MidiEvent {0x80, {110, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<Note<DrumNoteColour>> notes {{0, 0, DrumNoteColour::Yellow}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(double_kicks_are_read_correctly_from_mid)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x44,
                                         0x52, 0x55, 0x4D, 0x53}}}},
                           {0, {MidiEvent {0x90, {95, 64}}}},
                           {65, {MidiEvent {0x80, {95, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::DoubleKick}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(drum_fills_are_read_correctly_from_mid)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x44,
                                         0x52, 0x55, 0x4D, 0x53}}}},
                           {0, {MidiEvent {0x90, {98, 64}}}},
                           {45, {MidiEvent {0x90, {120, 64}}}},
                           {65, {MidiEvent {0x80, {98, 0}}}},
                           {75, {MidiEvent {0x80, {120, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<DrumFill> fills {{45, 30}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.drum_fills().cbegin(),
                                  track.drum_fills().cend(), fills.cbegin(),
                                  fills.cend());
}

BOOST_AUTO_TEST_CASE(disco_flips_are_read_correctly_from_mid)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {
              3,
              {0x50, 0x41, 0x52, 0x54, 0x20, 0x44, 0x52, 0x55, 0x4D, 0x53}}}},
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
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<DiscoFlip> flips {{15, 60}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.disco_flips().cbegin(),
                                  track.disco_flips().cend(), flips.cbegin(),
                                  flips.cend());
}

BOOST_AUTO_TEST_CASE(missing_disco_flip_end_event_just_ends_at_max_int)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {
              3,
              {0x50, 0x41, 0x52, 0x54, 0x20, 0x44, 0x52, 0x55, 0x4D, 0x53}}}},
         {15,
          {MetaEvent {1,
                      {0x5B, 0x6D, 0x69, 0x78, 0x20, 0x33, 0x20, 0x64, 0x72,
                       0x75, 0x6D, 0x73, 0x30, 0x64, 0x5D}}}},
         {45, {MidiEvent {0x90, {98, 64}}}},
         {65, {MidiEvent {0x80, {98, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<DiscoFlip> flips {{15, 2147483632}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.disco_flips().cbegin(),
                                  track.disco_flips().cend(), flips.cbegin(),
                                  flips.cend());
}

BOOST_AUTO_TEST_CASE(drum_five_lane_to_four_lane_conversion_is_done_from_mid)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x44,
                                         0x52, 0x55, 0x4D, 0x53}}}},
                           {0, {MidiEvent {0x90, {101, 64}}}},
                           {1, {MidiEvent {0x80, {101, 0}}}},
                           {2, {MidiEvent {0x90, {100, 64}}}},
                           {3, {MidiEvent {0x80, {100, 0}}}},
                           {4, {MidiEvent {0x90, {101, 64}}}},
                           {4, {MidiEvent {0x90, {100, 64}}}},
                           {5, {MidiEvent {0x80, {101, 0}}}},
                           {5, {MidiEvent {0x80, {100, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Green},
        {2, 0, DrumNoteColour::GreenCymbal},
        {4, 0, DrumNoteColour::Blue},
        {4, 0, DrumNoteColour::GreenCymbal}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(dynamics_are_parsed_from_mid)
{
    MidiTrack note_track {
        {{0,
          {MetaEvent {
              3,
              {0x50, 0x41, 0x52, 0x54, 0x20, 0x44, 0x52, 0x55, 0x4D, 0x53}}}},
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
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<Note<DrumNoteColour>> notes {{0, 0, DrumNoteColour::RedGhost},
                                             {2, 0, DrumNoteColour::Red},
                                             {4, 0, DrumNoteColour::RedAccent}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(dynamics_not_parsed_from_mid_without_ENABLE_CHART_DYNAMICS)
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x44,
                                         0x52, 0x55, 0x4D, 0x53}}}},
                           {0, {MidiEvent {0x90, {97, 1}}}},
                           {1, {MidiEvent {0x80, {97, 0}}}},
                           {2, {MidiEvent {0x90, {97, 64}}}},
                           {3, {MidiEvent {0x80, {97, 0}}}},
                           {4, {MidiEvent {0x90, {97, 127}}}},
                           {5, {MidiEvent {0x80, {97, 0}}}}}};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi, {});
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<Note<DrumNoteColour>> notes {{0, 0, DrumNoteColour::Red},
                                             {2, 0, DrumNoteColour::Red},
                                             {4, 0, DrumNoteColour::Red}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.notes().cbegin(), track.notes().cend(),
                                  notes.cbegin(), notes.cend());
}

BOOST_AUTO_TEST_CASE(unison_phrase_positions_is_correct)
{
    ChartSection guitar {"ExpertSingle",
                         {},
                         {},
                         {},
                         {{768, 0, 0}, {1024, 0, 0}},
                         {{768, 2, 100}, {1024, 2, 100}},
                         {}};
    // Note the first phrase has a different length than the other instruments.
    // It should still be a unison phrase: this happens in Roundabout, with the
    // key phrases being a slightly different length.
    ChartSection bass {"ExpertDoubleBass",
                       {},
                       {},
                       {},
                       {{768, 0, 0}, {2048, 0, 0}},
                       {{768, 2, 99}, {2048, 2, 100}},
                       {}};
    // The 768 phrase is absent for drums: this is to test that unison bonuses
    // can apply when at least 2 instruments have the phrase. This happens with
    // the first phrase on RB3 Last Dance guitar, the phrase is missing on bass.
    ChartSection drums {
        "ExpertDrums",    {}, {}, {}, {{768, 0, 0}, {4096, 0, 0}},
        {{4096, 2, 100}}, {}};
    std::vector<ChartSection> sections {guitar, bass, drums};
    const Chart chart {sections};
    const auto song = Song::from_chart(chart, {});

    const std::vector<int> expected_unison_phrases {768};
    const std::vector<int> unison_phrases = song.unison_phrase_positions();

    BOOST_CHECK_EQUAL_COLLECTIONS(
        unison_phrases.cbegin(), unison_phrases.cend(),
        expected_unison_phrases.cbegin(), expected_unison_phrases.cend());
}
