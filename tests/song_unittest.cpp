/*
 * CHOpt - Star Power optimiser for Clone Hero
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

#include "catch.hpp"

#include "song.hpp"

// Last checked: 23.2.2
TEST_CASE("NoteTrack ctor maintains invariants")
{
    SECTION("Notes are sorted")
    {
        std::vector<Note<NoteColour>> notes {{768}, {384}};
        NoteTrack<NoteColour> track {notes, {}, {}};
        std::vector<Note<NoteColour>> sorted_notes {{384}, {768}};

        REQUIRE(track.notes() == sorted_notes);
    }

    SECTION("Notes of the same colour and position are merged")
    {
        std::vector<Note<NoteColour>> notes {{768, 0}, {768, 768}};
        NoteTrack<NoteColour> track {notes, {}, {}};
        std::vector<Note<NoteColour>> required_notes {{768, 768}};

        REQUIRE(track.notes() == required_notes);

        std::vector<Note<NoteColour>> second_notes {{768, 768}, {768, 0}};
        NoteTrack<NoteColour> second_track {second_notes, {}, {}};
        std::vector<Note<NoteColour>> second_required_notes {{768, 0}};

        REQUIRE(second_track.notes() == second_required_notes);
    }

    SECTION("Notes of different colours are dealt with separately")
    {
        std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green},
                                             {768, 0, NoteColour::Red},
                                             {768, 768, NoteColour::Green}};
        NoteTrack<NoteColour> track {notes, {}, {}};
        std::vector<Note<NoteColour>> required_notes {
            {768, 768, NoteColour::Green}, {768, 0, NoteColour::Red}};

        REQUIRE(track.notes() == required_notes);
    }

    SECTION("Empty SP phrases are culled")
    {
        std::vector<Note<NoteColour>> notes {{768}};
        std::vector<StarPower> phrases {{0, 100}, {700, 100}, {1000, 100}};
        NoteTrack<NoteColour> track {notes, phrases, {}};
        std::vector<StarPower> required_phrases {{700, 100}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases are sorted")
    {
        std::vector<Note<NoteColour>> notes {{768}, {1000}};
        std::vector<StarPower> phrases {{1000, 1}, {768, 1}};
        NoteTrack<NoteColour> track {notes, phrases, {}};
        std::vector<StarPower> required_phrases {{768, 1}, {1000, 1}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases do not overlap")
    {
        std::vector<Note<NoteColour>> notes {{768}, {1000}};
        std::vector<StarPower> phrases {{768, 1000}, {900, 150}};
        NoteTrack<NoteColour> track {notes, phrases, {}};
        std::vector<StarPower> required_phrases {{768, 132}, {900, 150}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("Solos are sorted")
    {
        std::vector<Note<NoteColour>> notes {{0}, {768}};
        std::vector<Solo> solos {{768, 868, 100}, {0, 100, 100}};
        NoteTrack<NoteColour> track {notes, {}, solos};
        std::vector<Solo> required_solos {{0, 100, 100}, {768, 868, 100}};

        REQUIRE(track.solos() == required_solos);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("SyncTrack ctor maintains invariants")
{
    SECTION("BPMs are sorted by position")
    {
        SyncTrack track {{}, {{0, 150000}, {2000, 200000}, {1000, 225000}}};
        std::vector<BPM> expected_bpms {
            {0, 150000}, {1000, 225000}, {2000, 200000}};

        REQUIRE(track.bpms() == expected_bpms);
    }

    SECTION("No two BPMs have the same position")
    {
        SyncTrack track {{}, {{0, 150000}, {0, 200000}}};
        std::vector<BPM> expected_bpms {{0, 200000}};

        REQUIRE(track.bpms() == expected_bpms);
    }

    SECTION("bpms() is never empty")
    {
        SyncTrack track;
        std::vector<BPM> expected_bpms {{0, 120000}};

        REQUIRE(track.bpms() == expected_bpms);
    }

    SECTION("TimeSignatures are sorted by position")
    {
        SyncTrack track {{{0, 4, 4}, {2000, 3, 3}, {1000, 2, 2}}, {}};
        std::vector<TimeSignature> expected_tses {
            {0, 4, 4}, {1000, 2, 2}, {2000, 3, 3}};

        REQUIRE(track.time_sigs() == expected_tses);
    }

    SECTION("No two TimeSignatures have the same position")
    {
        SyncTrack track {{{0, 4, 4}, {0, 3, 4}}, {}};
        std::vector<TimeSignature> expected_tses {{0, 3, 4}};

        REQUIRE(track.time_sigs() == expected_tses);
    }

    SECTION("time_sigs() is never empty")
    {
        SyncTrack track;
        std::vector<TimeSignature> expected_tses {{0, 4, 4}};

        REQUIRE(track.time_sigs() == expected_tses);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads resolution")
{
    ChartSection sync_track {"SyncTrack", {}, {}, {}, {}, {}, {}};
    ChartSection events {"Events", {}, {}, {}, {}, {}, {}};
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};

    SECTION("Default is 192 Res")
    {
        std::vector<ChartSection> sections {expert_single};
        const Chart chart {sections};

        const auto resolution = Song::from_chart(chart).resolution();

        REQUIRE(resolution == 192);
    }

    SECTION("Default is overriden by specified value")
    {
        ChartSection header {
            "Song", {{"Resolution", "200"}, {"Offset", "100"}}, {}, {}, {}, {},
            {}};
        std::vector<ChartSection> sections {header, expert_single};
        const Chart chart {sections};

        const auto resolution = Song::from_chart(chart).resolution();

        REQUIRE(resolution == 200);
    }
}

TEST_CASE("Chart reads song header correctly")
{
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};

    SECTION("Default values are correct")
    {
        std::vector<ChartSection> sections {expert_single};
        const Chart chart {sections};
        const auto song = Song::from_chart(chart);
        const auto& header = song.song_header();

        REQUIRE(header.name == "Unknown Song");
        REQUIRE(header.artist == "Unknown Artist");
        REQUIRE(header.charter == "Unknown Charter");
    }

    SECTION("Read values are correct")
    {
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
        const auto song = Song::from_chart(chart);
        const auto& header = song.song_header();

        REQUIRE(header.name == "TestName");
        REQUIRE(header.artist == "GMS");
        REQUIRE(header.charter == "NotGMS");
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads sync track correctly")
{
    ChartSection sync_track {"SyncTrack", {}, {{0, 200000}},           {},
                             {},          {}, {{0, 4, 2}, {768, 4, 1}}};
    ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                {{768, 0, 0}},  {}, {}};
    std::vector<ChartSection> sections {sync_track, expert_single};
    const Chart chart {sections};
    std::vector<TimeSignature> time_sigs {{0, 4, 4}, {768, 4, 2}};
    std::vector<BPM> bpms {{0, 200000}};

    const auto chart_sync_track = Song::from_chart(chart).sync_track();

    REQUIRE(chart_sync_track.time_sigs() == time_sigs);
    REQUIRE(chart_sync_track.bpms() == bpms);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads easy note track correctly")
{
    ChartSection easy_single {"EasySingle",    {}, {}, {}, {{768, 0, 0}},
                              {{768, 2, 100}}, {}};
    std::vector<ChartSection> sections {easy_single};
    const Chart chart {sections};
    NoteTrack<NoteColour> note_track {
        {{768, 0, NoteColour::Green}}, {{768, 100}}, {}};

    const auto song = Song::from_chart(chart);

    REQUIRE(song.guitar_note_track(Difficulty::Easy) == note_track);
}

TEST_CASE("SP phrases are read correctly from Chart")
{
    ChartSection expert_single {"ExpertSingle",  {}, {}, {}, {{768, 0, 0}},
                                {{768, 1, 100}}, {}};
    std::vector<ChartSection> sections {expert_single};
    const Chart chart {sections};
    NoteTrack<NoteColour> note_track {
        {{768, 0, NoteColour::Green}}, {{768, 100}}, {}};

    const auto song = Song::from_chart(chart);

    REQUIRE(song.guitar_note_track(Difficulty::Expert).sp_phrases().empty());
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart does not need sections in usual order")
{
    SECTION("Non note sections need not be present")
    {
        ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                    {{768, 0, 0}},  {}, {}};
        std::vector<ChartSection> sections {expert_single};
        const Chart chart {sections};

        REQUIRE_NOTHROW([&] { return Song::from_chart(chart); }());
    }

    SECTION("At least one non-empty note section must be present")
    {
        ChartSection expert_single {"ExpertSingle",  {}, {}, {}, {},
                                    {{768, 2, 100}}, {}};
        std::vector<ChartSection> sections {expert_single};
        const Chart chart {sections};

        REQUIRE_THROWS([] { return Song::from_chart({}); }());
        REQUIRE_THROWS([&] { return Song::from_chart(chart); }());
    }

    SECTION("Non note sections can be in any order")
    {
        ChartSection expert_single {"ExpertSingle", {}, {}, {},
                                    {{768, 0, 0}},  {}, {}};
        ChartSection sync_track {"SyncTrack", {}, {{0, 200000}}, {}, {},
                                 {},          {}};
        ChartSection header {"Song", {{"Resolution", "200"}}, {}, {}, {}, {},
                             {}};
        std::vector<ChartSection> sections {sync_track, expert_single, header};
        const Chart chart {sections};
        std::vector<Note<NoteColour>> notes {{768}};
        std::vector<BPM> bpms {{0, 200000}};

        const auto song = Song::from_chart(chart);

        REQUIRE(song.resolution() == 200);
        REQUIRE(song.guitar_note_track(Difficulty::Expert).notes() == notes);
        REQUIRE(song.sync_track().bpms() == bpms);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Only first non-empty part of note sections matter")
{
    SECTION("Later non-empty sections are ignored")
    {
        ChartSection expert_single_one {"ExpertSingle", {}, {}, {},
                                        {{768, 0, 0}},  {}, {}};
        ChartSection expert_single_two {"ExpertSingle", {}, {}, {},
                                        {{768, 1, 0}},  {}, {}};
        std::vector<ChartSection> sections {expert_single_one,
                                            expert_single_two};
        const Chart chart {sections};
        std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green}};

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).notes() == notes);
    }

    SECTION("Leading empty sections are ignored")
    {
        ChartSection expert_single_one {"ExpertSingle", {}, {}, {}, {}, {}, {}};
        ChartSection expert_single_two {"ExpertSingle", {}, {}, {},
                                        {{768, 1, 0}},  {}, {}};
        std::vector<ChartSection> sections {expert_single_one,
                                            expert_single_two};
        const Chart chart {sections};
        std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Red}};

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).notes() == notes);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Solos are read properly")
{
    SECTION("Expected solos are read properly")
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

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }

    SECTION("Chords are not counted double")
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

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }

    SECTION("Empty solos are ignored")
    {
        ChartSection expert_single {
            "ExpertSingle", {}, {}, {{100, "solo"}, {200, "soloend"}},
            {{0, 0, 0}},    {}, {}};
        std::vector<ChartSection> sections {expert_single};
        const Chart chart {sections};

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).solos().empty());
    }

    SECTION("Repeated solo starts and ends don't matter")
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

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }

    SECTION("Solo markers are sorted")
    {
        ChartSection expert_single {
            "ExpertSingle", {}, {}, {{384, "soloend"}, {0, "solo"}},
            {{192, 0, 0}},  {}, {}};
        std::vector<ChartSection> sections {expert_single};
        const Chart chart {sections};
        std::vector<Solo> required_solos {{0, 384, 100}};

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }

    SECTION("Solos with no soloend event are ignored")
    {
        ChartSection expert_single {"ExpertSingle", {}, {}, {{0, "solo"}},
                                    {{192, 0, 0}},  {}, {}};
        std::vector<ChartSection> sections {expert_single};
        const Chart chart {sections};

        const auto song = Song::from_chart(chart);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).solos().empty());
    }
}

TEST_CASE("instruments returns the supported instruments")
{
    ChartSection guitar {"ExpertSingle", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    ChartSection bass {"ExpertDoubleBass", {}, {}, {}, {}, {}, {}};
    ChartSection drums {"ExpertDrums", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    std::vector<ChartSection> sections {guitar, bass, drums};
    const Chart chart {sections};
    std::vector<Instrument> instruments {Instrument::Guitar, Instrument::Drums};

    const auto song = Song::from_chart(chart);

    REQUIRE(song.instruments() == instruments);
}

TEST_CASE("difficulties returns the difficulties for an instrument")
{
    ChartSection guitar {"ExpertSingle", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    ChartSection hard_guitar {"HardSingle", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    ChartSection drums {"ExpertDrums", {}, {}, {}, {{192, 0, 0}}, {}, {}};
    std::vector<ChartSection> sections {guitar, hard_guitar, drums};
    const Chart chart {sections};
    std::vector<Difficulty> guitar_difficulties {Difficulty::Hard,
                                                 Difficulty::Expert};
    std::vector<Difficulty> drum_difficulties {Difficulty::Expert};

    const auto song = Song::from_chart(chart);

    REQUIRE(song.difficulties(Instrument::Guitar) == guitar_difficulties);
    REQUIRE(song.difficulties(Instrument::Drums) == drum_difficulties);
}

TEST_CASE("Other 5 fret instruments are read from Chart")
{
    SECTION("Guitar Co-op is read")
    {
        ChartSection expert_double {"ExpertDoubleGuitar", {}, {}, {},
                                    {{192, 0, 0}},        {}, {}};
        std::vector<ChartSection> sections {expert_double};
        const Chart chart {sections};

        const auto song = Song::from_chart(chart);

        REQUIRE_NOTHROW(
            [&] { return song.guitar_coop_note_track(Difficulty::Expert); }());
    }

    SECTION("Bass is read")
    {
        ChartSection expert_double {"ExpertDoubleBass", {}, {}, {},
                                    {{192, 0, 0}},      {}, {}};
        std::vector<ChartSection> sections {expert_double};
        const Chart chart {sections};

        const auto song = Song::from_chart(chart);

        REQUIRE_NOTHROW(
            [&] { return song.bass_note_track(Difficulty::Expert); }());
    }

    SECTION("Rhythm is read")
    {
        ChartSection expert_double {"ExpertDoubleRhythm", {}, {}, {},
                                    {{192, 0, 0}},        {}, {}};
        std::vector<ChartSection> sections {expert_double};
        const Chart chart {sections};

        const auto song = Song::from_chart(chart);

        REQUIRE_NOTHROW(
            [&] { return song.rhythm_note_track(Difficulty::Expert); }());
    }

    SECTION("Keys is read")
    {
        ChartSection expert_double {"ExpertKeyboard", {}, {}, {},
                                    {{192, 0, 0}},    {}, {}};
        std::vector<ChartSection> sections {expert_double};
        const Chart chart {sections};

        const auto song = Song::from_chart(chart);

        REQUIRE_NOTHROW(
            [&] { return song.keys_note_track(Difficulty::Expert); }());
    }
}

TEST_CASE("6 fret instruments are read correctly from .chart")
{
    SECTION("6 fret guitar is read correctly")
    {
        ChartSection expert_double {"ExpertGHLGuitar",          {}, {}, {},
                                    {{192, 0, 0}, {384, 3, 0}}, {}, {}};
        std::vector<ChartSection> sections {expert_double};
        const Chart chart {sections};
        std::vector<Note<GHLNoteColour>> notes {
            {192, 0, GHLNoteColour::WhiteLow},
            {384, 0, GHLNoteColour::BlackLow}};

        const auto song = Song::from_chart(chart);
        const auto& track = song.ghl_guitar_note_track(Difficulty::Expert);

        REQUIRE(track.notes() == notes);
    }

    SECTION("6 fret bass is read correctly")
    {
        ChartSection expert_double {
            "ExpertGHLBass", {}, {}, {}, {{192, 0, 0}, {384, 3, 0}}, {}, {}};
        std::vector<ChartSection> sections {expert_double};
        const Chart chart {sections};
        std::vector<Note<GHLNoteColour>> notes {
            {192, 0, GHLNoteColour::WhiteLow},
            {384, 0, GHLNoteColour::BlackLow}};

        const auto song = Song::from_chart(chart);
        const auto& track = song.ghl_bass_note_track(Difficulty::Expert);

        REQUIRE(track.notes() == notes);
    }
}

TEST_CASE("Drums are read correctly from .chart")
{
    ChartSection expert_drums {
        "ExpertDrums",
        {},
        {},
        {},
        {{192, 1, 0}, {384, 2, 0}, {384, 66, 0}, {576, 5, 0}},
        {},
        {}};
    std::vector<ChartSection> sections {expert_drums};
    const Chart chart {sections};
    std::vector<Note<DrumNoteColour>> notes {
        {192, 0, DrumNoteColour::Red}, {384, 0, DrumNoteColour::YellowCymbal}};

    const auto song = Song::from_chart(chart);
    const auto& track = song.drum_note_track(Difficulty::Expert);

    REQUIRE(track.notes() == notes);
}

TEST_CASE("Midi resolution is read correctly")
{
    SECTION("Midi's resolution is read")
    {
        const Midi midi {200, {}};

        const auto song = Song::from_midi(midi);

        REQUIRE(song.resolution() == 200);
    }

    SECTION("Resolution > 0 invariant is upheld")
    {
        const Midi midi {0, {}};

        REQUIRE_THROWS([&] { return Song::from_midi(midi); }());
    }
}

TEST_CASE("First track is read correctly")
{
    SECTION("Tempos are read correctly")
    {
        MidiTrack tempo_track {{{0, {MetaEvent {0x51, {6, 0x1A, 0x80}}}},
                                {1920, {MetaEvent {0x51, {4, 0x93, 0xE0}}}}}};
        const Midi midi {192, {tempo_track}};
        SyncTrack tempos {{}, {{0, 150000}, {1920, 200000}}};

        const auto song = Song::from_midi(midi);
        const auto& sync_track = song.sync_track();

        REQUIRE(sync_track.bpms() == tempos.bpms());
        REQUIRE(sync_track.time_sigs() == tempos.time_sigs());
    }

    SECTION("Time signatures are read correctly")
    {
        MidiTrack ts_track {{{0, {MetaEvent {0x58, {6, 2, 24, 8}}}},
                             {1920, {MetaEvent {0x58, {3, 3, 24, 8}}}}}};
        const Midi midi {192, {ts_track}};
        SyncTrack tses {{{0, 6, 4}, {1920, 3, 8}}, {}};

        const auto song = Song::from_midi(midi);
        const auto& sync_track = song.sync_track();

        REQUIRE(sync_track.bpms() == tses.bpms());
        REQUIRE(sync_track.time_sigs() == tses.time_sigs());
    }

    SECTION("Song name is read correctly")
    {
        MidiTrack name_track {{{0, {MetaEvent {1, {72, 101, 108, 108, 111}}}}}};
        const Midi midi {192, {name_track}};

        const auto song = Song::from_midi(midi);

        REQUIRE(song.song_header().name == "Hello");
    }

    SECTION("Default song header is correct")
    {
        const Midi midi {192, {{}}};

        const auto song = Song::from_midi(midi);

        REQUIRE(song.song_header().name == "Unknown Song");
        REQUIRE(song.song_header().artist == "Unknown Artist");
        REQUIRE(song.song_header().charter == "Unknown Charter");
    }
}

TEST_CASE("Notes are read correctly")
{
    SECTION("Notes of every difficulty are read")
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

        const auto song = Song::from_midi(midi);

        REQUIRE(song.guitar_note_track(Difficulty::Easy).notes() == green_note);
        REQUIRE(song.guitar_note_track(Difficulty::Medium).notes()
                == green_note);
        REQUIRE(song.guitar_note_track(Difficulty::Hard).notes() == green_note);
        REQUIRE(song.guitar_note_track(Difficulty::Expert).notes()
                == green_note);
    }

    SECTION("Notes are read from PART GUITAR")
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

        const auto song = Song::from_midi(midi);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).notes()[0].colour
                == NoteColour::Red);
    }

    SECTION("Guitar notes are also read from T1 GEMS")
    {
        MidiTrack other_track {{{768, {MidiEvent {0x90, {96, 64}}}},
                                {960, {MidiEvent {0x80, {96, 0}}}}}};
        MidiTrack note_track {
            {{0, {MetaEvent {3, {0x54, 0x31, 0x20, 0x47, 0x45, 0x4D, 0x53}}}},
             {768, {MidiEvent {0x90, {97, 64}}}},
             {960, {MidiEvent {0x80, {97, 0}}}}}};
        const Midi midi {192, {other_track, note_track}};

        const auto song = Song::from_midi(midi);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).notes()[0].colour
                == NoteColour::Red);
    }

    SECTION("Note On events must have a corresponding Note Off event")
    {
        MidiTrack note_track {{{0,
                                {MetaEvent {3,
                                            {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                             0x55, 0x49, 0x54, 0x41, 0x52}}}},
                               {768, {MidiEvent {0x90, {96, 64}}}},
                               {960, {MidiEvent {0x80, {96, 64}}}},
                               {1152, {MidiEvent {0x90, {96, 64}}}}}};
        const Midi midi {192, {note_track}};

        REQUIRE_THROWS([&] { return Song::from_midi(midi); }());
    }

    SECTION("Corresponding Note Off events are after Note On events")
    {
        MidiTrack note_track {{
            {0,
             {MetaEvent {3,
                         {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                          0x41, 0x52}}}},
            {480, {MidiEvent {0x90, {96, 64}}}},
            {480, {MidiEvent {0x80, {96, 64}}}},
            {960, {MidiEvent {0x80, {96, 64}}}},
            {960, {MidiEvent {0x90, {96, 64}}}},
            {1440, {MidiEvent {0x80, {96, 64}}}},
        }};
        const Midi midi {480, {note_track}};

        const auto song = Song::from_midi(midi);
        const auto& notes = song.guitar_note_track(Difficulty::Expert).notes();

        REQUIRE(notes.size() == 2);
        REQUIRE(notes[0].length == 480);
    }

    SECTION("Note On events with velocity 0 count as Note Off events")
    {
        MidiTrack note_track {{{0,
                                {MetaEvent {3,
                                            {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                             0x55, 0x49, 0x54, 0x41, 0x52}}}},
                               {768, {MidiEvent {0x90, {96, 64}}}},
                               {960, {MidiEvent {0x90, {96, 0}}}}}};
        const Midi midi {192, {note_track}};

        REQUIRE_NOTHROW([&] { return Song::from_midi(midi); }());
    }

    SECTION("Open notes are read correctly")
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

        const auto song = Song::from_midi(midi);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).notes()[0].colour
                == NoteColour::Open);
    }
}

TEST_CASE("Solos are read")
{
    MidiTrack note_track {{{0,
                            {MetaEvent {3,
                                        {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                         0x55, 0x49, 0x54, 0x41, 0x52}}}},
                           {768, {MidiEvent {0x90, {103, 64}}}},
                           {768, {MidiEvent {0x90, {96, 64}}}},
                           {900, {MidiEvent {0x80, {103, 64}}}},
                           {960, {MidiEvent {0x80, {96, 0}}}}}};
    const Midi midi {192, {note_track}};
    const std::vector<Solo> solos {{768, 900, 100}};

    const auto song = Song::from_midi(midi);

    REQUIRE(song.guitar_note_track(Difficulty::Expert).solos() == solos);
}

TEST_CASE("Star Power is read")
{
    SECTION("A single phrase is read")
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

        const auto song = Song::from_midi(midi);

        REQUIRE(song.guitar_note_track(Difficulty::Expert).sp_phrases()
                == sp_phrases);
    }

    SECTION("A Note Off event is required for every phrase")
    {
        MidiTrack note_track {{{0,
                                {MetaEvent {3,
                                            {0x50, 0x41, 0x52, 0x54, 0x20, 0x47,
                                             0x55, 0x49, 0x54, 0x41, 0x52}}}},
                               {768, {MidiEvent {0x90, {116, 64}}}},
                               {768, {MidiEvent {0x90, {96, 64}}}},
                               {960, {MidiEvent {0x80, {96, 0}}}}}};
        const Midi midi {192, {note_track}};

        REQUIRE_THROWS([&] { return Song::from_midi(midi); }());
    }
}

TEST_CASE("Short midi sustains are trimmed")
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
    const auto song = Song::from_midi(midi);
    const auto& notes = song.guitar_note_track(Difficulty::Expert).notes();

    REQUIRE(notes[0].length == 0);
    REQUIRE(notes[1].length == 70);
}

TEST_CASE("Other 5 fret instruments are read from .mid")
{
    SECTION("Guitar Co-op is read")
    {
        MidiTrack note_track {
            {{0,
              {MetaEvent {3,
                          {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                           0x41, 0x52, 0x20, 0x43, 0x4F, 0x4F, 0x50}}}},
             {0, {MidiEvent {0x90, {96, 64}}}},
             {65, {MidiEvent {0x80, {96, 0}}}}}};
        const Midi midi {192, {note_track}};
        const auto song = Song::from_midi(midi);

        REQUIRE_NOTHROW(
            [&] { return song.guitar_coop_note_track(Difficulty::Expert); }());
    }

    SECTION("Bass is read")
    {
        MidiTrack note_track {
            {{0,
              {MetaEvent {
                  3, {0x50, 0x41, 0x52, 0x54, 0x20, 0x42, 0x41, 0x53, 0x53}}}},
             {0, {MidiEvent {0x90, {96, 64}}}},
             {65, {MidiEvent {0x80, {96, 0}}}}}};
        const Midi midi {192, {note_track}};
        const auto song = Song::from_midi(midi);

        REQUIRE_NOTHROW(
            [&] { return song.bass_note_track(Difficulty::Expert); }());
    }

    SECTION("Rhythm is read")
    {
        MidiTrack note_track {{{0,
                                {MetaEvent {3,
                                            {0x50, 0x41, 0x52, 0x54, 0x20, 0x52,
                                             0x48, 0x59, 0x54, 0x48, 0x4D}}}},
                               {0, {MidiEvent {0x90, {96, 64}}}},
                               {65, {MidiEvent {0x80, {96, 0}}}}}};
        const Midi midi {192, {note_track}};
        const auto song = Song::from_midi(midi);

        REQUIRE_NOTHROW(
            [&] { return song.rhythm_note_track(Difficulty::Expert); }());
    }

    SECTION("Keys is read")
    {
        MidiTrack note_track {
            {{0,
              {MetaEvent {
                  3, {0x50, 0x41, 0x52, 0x54, 0x20, 0x4B, 0x45, 0x59, 0x53}}}},
             {0, {MidiEvent {0x90, {96, 64}}}},
             {65, {MidiEvent {0x80, {96, 0}}}}}};
        const Midi midi {192, {note_track}};
        const auto song = Song::from_midi(midi);

        REQUIRE_NOTHROW(
            [&] { return song.keys_note_track(Difficulty::Expert); }());
    }
}

TEST_CASE("6 fret instruments are read correctly from .mid")
{
    SECTION("6 fret guitar is read correctly")
    {
        MidiTrack note_track {
            {{0,
              {MetaEvent {3,
                          {0x50, 0x41, 0x52, 0x54, 0x20, 0x47, 0x55, 0x49, 0x54,
                           0x41, 0x52, 0x20, 0x47, 0x48, 0x4C}}}},
             {0, {MidiEvent {0x90, {94, 64}}}},
             {65, {MidiEvent {0x80, {94, 0}}}}}};
        const Midi midi {192, {note_track}};
        const auto song = Song::from_midi(midi);
        const auto& track = song.ghl_guitar_note_track(Difficulty::Expert);

        std::vector<Note<GHLNoteColour>> notes {{0, 65, GHLNoteColour::Open}};

        REQUIRE(track.notes() == notes);
    }

    SECTION("6 fret bass is read correctly")
    {
        MidiTrack note_track {
            {{0,
              {MetaEvent {3,
                          {0x50, 0x41, 0x52, 0x54, 0x20, 0x42, 0x41, 0x53, 0x53,
                           0x20, 0x47, 0x48, 0x4C}}}},
             {0, {MidiEvent {0x90, {94, 64}}}},
             {65, {MidiEvent {0x80, {94, 0}}}}}};
        const Midi midi {192, {note_track}};
        const auto song = Song::from_midi(midi);
        const auto& track = song.ghl_bass_note_track(Difficulty::Expert);

        std::vector<Note<GHLNoteColour>> notes {{0, 65, GHLNoteColour::Open}};

        REQUIRE(track.notes() == notes);
    }
}

TEST_CASE("Drums are read correctly from .mid")
{
    MidiTrack note_track {{
        {0,
         {MetaEvent {
             3, {0x50, 0x41, 0x52, 0x54, 0x20, 0x44, 0x52, 0x55, 0x4D, 0x53}}}},
        {0, {MidiEvent {0x90, {98, 64}}}},
        {0, {MidiEvent {0x90, {110, 64}}}},
        {65, {MidiEvent {0x80, {98, 0}}}},
        {65, {MidiEvent {0x80, {110, 0}}}},
    }};
    const Midi midi {192, {note_track}};
    const auto song = Song::from_midi(midi);
    const auto& track = song.drum_note_track(Difficulty::Expert);

    std::vector<Note<DrumNoteColour>> notes {{0, 0, DrumNoteColour::Yellow}};

    REQUIRE(track.notes() == notes);
}
