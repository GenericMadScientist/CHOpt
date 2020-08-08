/*
 * chopt - Star Power optimiser for Clone Hero
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

#include "chart.hpp"

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
    SECTION("Default is 192 Res")
    {
        const char* text = "[Song]\n{\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n["
                           "ExpertSingle]\n{\n768 = N 0 0\n}";
        const auto resolution = Chart::parse_chart(text).resolution();

        REQUIRE(resolution == 192);
    }

    SECTION("Default is overriden by specified value")
    {
        const char* text
            = "[Song]\n{\nResolution = 200\nOffset = "
              "100\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[ExpertSingle]"
              "\n{\n768 = N 0 0\n}";
        const auto resolution = Chart::parse_chart(text).resolution();

        REQUIRE(resolution == 200);
    }
}

TEST_CASE("Chart reads song header correctly")
{
    SECTION("Default values are correct")
    {
        const char* text = "[ExpertSingle]\n{\n768 = N 0 0\n}";
        const auto chart = Chart::parse_chart(text);
        const auto& header = chart.song_header();

        REQUIRE(header.name == "Unknown Song");
        REQUIRE(header.artist == "Unknown Artist");
        REQUIRE(header.charter == "Unknown Charter");
    }

    SECTION("Read values are correct")
    {
        const char* text
            = "[Song]\n{\nName = \"TestName\"\nArtist = \"GMS\"\nCharter = "
              "\"NotGMS\"\n}\n[ExpertSingle]\n{\n768 = N 0 0\n}";
        const auto chart = Chart::parse_chart(text);
        const auto& header = chart.song_header();

        REQUIRE(header.name == "TestName");
        REQUIRE(header.artist == "GMS");
        REQUIRE(header.charter == "NotGMS");
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads sync track correctly")
{
    const char* text
        = "[Song]\n{\n}\n[SyncTrack]\n{\n0 = B 200000\n0 = TS 4\n768 = "
          "TS 4 1\n}\n[Events]\n{\n}\n[ExpertSingle]\n{\n768 = N 0 0\n}";
    const auto sync_track = Chart::parse_chart(text).sync_track();
    std::vector<TimeSignature> time_sigs {{0, 4, 4}, {768, 4, 2}};
    std::vector<BPM> bpms {{0, 200000}};

    REQUIRE(sync_track.time_sigs() == time_sigs);
    REQUIRE(sync_track.bpms() == bpms);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads easy note track correctly")
{
    const char* text
        = "[Song]\n{\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[EasySingle]"
          "\n{\n768 = N 0 0\n768 = S 2 100\n}\n";
    const auto chart = Chart::parse_chart(text);
    NoteTrack<NoteColour> note_track {
        {{768, 0, NoteColour::Green}}, {{768, 100}}, {}};

    REQUIRE(chart.guitar_note_track(Difficulty::Easy) == note_track);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart skips UTF-8 BOM")
{
    const char* text
        = "\xEF\xBB\xBF[Song]\n{\nOffset = "
          "100\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[ExpertSingle]\n{"
          "\n768 = N 0 0\n}";
    const auto resolution = Chart::parse_chart(text).resolution();

    REQUIRE(resolution == 192);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart can end without a newline")
{
    const char* text
        = "\xEF\xBB\xBF[Song]\n{\nOffset = "
          "100\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[ExpertSingle]\n{"
          "\n768 = N 0 0\n}";
    const auto resolution = Chart::parse_chart(text).resolution();

    REQUIRE(resolution == 192);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart does not need sections in usual order")
{
    SECTION("Non note sections need not be present")
    {
        const char* text = "[ExpertSingle]\n{\n768 = N 0 0\n}";
        const auto chart = Chart::parse_chart(text);

        REQUIRE(chart.resolution() == 192);
    }

    SECTION("At least one non-empty note section must be present")
    {
        const char* text = "[ExpertSingle]\n{\n768 = S 2 100\n}";

        REQUIRE_THROWS([] { return Chart::parse_chart(""); }());
        REQUIRE_THROWS([&] { return Chart::parse_chart(text); }());
    }

    SECTION("Non note sections can be in any order")
    {
        const char* text
            = "[SyncTrack]\n{\n0 = B 200000\n}\n[ExpertSingle]\n{\n768 = "
              "N 0 0\n}\n[Song]\n{\nResolution = 200\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Note<NoteColour>> notes {{768}};
        std::vector<BPM> bpms {{0, 200000}};

        REQUIRE(chart.resolution() == 200);
        REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes() == notes);
        REQUIRE(chart.sync_track().bpms() == bpms);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Only first non-empty part of note sections matter")
{
    SECTION("Later non-empty sections are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n768 = N 0 0\n}\n[ExpertSingle]\n{\n768 = N "
              "1 0\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green}};

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes() == notes);
    }

    SECTION("Leading empty sections are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n}\n[ExpertSingle]\n{\n768 = N 1 0\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Red}};

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes() == notes);
    }
}

// Last checked: 23.2.2
TEST_CASE("Notes should be sorted")
{
    const char* text = "[ExpertSingle]\n{\n768 = N 0 0\n384 = N 0 0\n}";
    const auto chart = Chart::parse_chart(text);
    std::vector<Note<NoteColour>> notes {{384, 0, NoteColour::Green},
                                         {768, 0, NoteColour::Green}};

    REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes() == notes);
}

// Last checked: 24.0.1555-master
TEST_CASE("Notes with extra spaces")
{
    SECTION("Leading non-empty sections with notes with spaces cause a throw")
    {
        const char* text = "[ExpertSingle]\n{\n768 = N  0 0\n}";

        REQUIRE_THROWS([&] { return Chart::parse_chart(text); }());
    }

    SECTION("Non-leading non-empty sections with broken notes are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n768 = N 0 0\n}\n[ExpertSingle]\n{\n768 "
              "= N  1 0\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Note<NoteColour>> required_notes {{768}};

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes()
                == required_notes);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Solos are read properly")
{
    SECTION("Expected solos are read properly")
    {
        const char* text = "[ExpertSingle]\n{\n0 = E solo\n100 = N 0 0\n200 = "
                           "E soloend\n300 = E solo\n300 = N 0 0\n400 = N 0 "
                           "0\n400 = E soloend\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Solo> required_solos {{0, 200, 100}, {300, 400, 200}};

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }

    SECTION("Chords are not counted double")
    {
        const char* text = "[ExpertSingle]\n{\n0 = E solo\n100 = N 0 0\n100 = "
                           "N 1 0\n200 = E soloend\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Solo> required_solos {{0, 200, 100}};

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }

    SECTION("Empty solos are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n0 = N 0 0\n100 = E solo\n200 = E soloend\n}";
        const auto chart = Chart::parse_chart(text);

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).solos().empty());
    }

    SECTION("Repeated solo starts and ends don't matter")
    {
        const char* text = "[ExpertSingle]\n{\n0 = E solo\n100 = E solo\n100 = "
                           "N 0 0\n200 = E soloend\n300 = E soloend\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Solo> required_solos {{0, 200, 100}};

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }

    SECTION("Solo markers are sorted")
    {
        const char* text
            = "[ExpertSingle]\n{\n384 = E soloend\n192 = N 0 0\n0 = E solo\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Solo> required_solos {{0, 384, 100}};

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).solos()
                == required_solos);
    }
}

TEST_CASE("Other 5 fret guitar-like instruments are read from .chart")
{
    SECTION("Guitar Co-op is read")
    {
        const char* text = "[ExpertDoubleGuitar]\n{\n192 = N 0 0\n}";
        const auto chart = Chart::parse_chart(text);

        REQUIRE_NOTHROW(
            [&] { return chart.guitar_coop_note_track(Difficulty::Expert); });
    }

    SECTION("Bass is read")
    {
        const char* text = "[ExpertDoubleBass]\n{\n192 = N 0 0\n}";
        const auto chart = Chart::parse_chart(text);

        REQUIRE_NOTHROW(
            [&] { return chart.bass_note_track(Difficulty::Expert); });
    }

    SECTION("Rhythm is read")
    {
        const char* text = "[ExpertDoubleRhythm]\n{\n192 = N 0 0\n}";
        const auto chart = Chart::parse_chart(text);

        REQUIRE_NOTHROW(
            [&] { return chart.rhythm_note_track(Difficulty::Expert); });
    }

    SECTION("Keys is read")
    {
        const char* text = "[ExpertKeyboard]\n{\n192 = N 0 0\n}";
        const auto chart = Chart::parse_chart(text);

        REQUIRE_NOTHROW(
            [&] { return chart.keys_note_track(Difficulty::Expert); });
    }
}

TEST_CASE("6 fret guitar is read correctly")
{
    const char* text = "[ExpertGHLGuitar]\n{\n192 = N 0 0\n384 = N 3 0\n}";
    const auto chart = Chart::parse_chart(text);
    std::vector<Note<GHLNoteColour>> notes {{192, 0, GHLNoteColour::WhiteLow},
                                            {384, 0, GHLNoteColour::BlackLow}};

    const auto track = chart.ghl_guitar_note_track(Difficulty::Expert);

    REQUIRE(track.notes() == notes);
}

TEST_CASE("Midi resolution is read correctly")
{
    SECTION("Midi's resolution is read")
    {
        const Midi midi {200, {}};

        const auto chart = Chart::from_midi(midi);

        REQUIRE(chart.resolution() == 200);
    }

    SECTION("Resolution > 0 invariant is upheld")
    {
        const Midi midi {0, {}};

        REQUIRE_THROWS([&] { return Chart::from_midi(midi); }());
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

        const auto chart = Chart::from_midi(midi);
        const auto& sync_track = chart.sync_track();

        REQUIRE(sync_track.bpms() == tempos.bpms());
        REQUIRE(sync_track.time_sigs() == tempos.time_sigs());
    }

    SECTION("Time signatures are read correctly")
    {
        MidiTrack ts_track {{{0, {MetaEvent {0x58, {6, 2, 24, 8}}}},
                             {1920, {MetaEvent {0x58, {3, 3, 24, 8}}}}}};
        const Midi midi {192, {ts_track}};
        SyncTrack tses {{{0, 6, 4}, {1920, 3, 8}}, {}};

        const auto chart = Chart::from_midi(midi);
        const auto& sync_track = chart.sync_track();

        REQUIRE(sync_track.bpms() == tses.bpms());
        REQUIRE(sync_track.time_sigs() == tses.time_sigs());
    }

    SECTION("Song name is read correctly")
    {
        MidiTrack name_track {{{0, {MetaEvent {1, {72, 101, 108, 108, 111}}}}}};
        const Midi midi {192, {name_track}};

        const auto chart = Chart::from_midi(midi);

        REQUIRE(chart.song_header().name == "Hello");
    }

    SECTION("Default song header is correct")
    {
        const Midi midi {192, {{}}};

        const auto chart = Chart::from_midi(midi);

        REQUIRE(chart.song_header().name == "Unknown Song");
        REQUIRE(chart.song_header().artist == "Unknown Artist");
        REQUIRE(chart.song_header().charter == "Unknown Charter");
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

        const auto chart = Chart::from_midi(midi);

        REQUIRE(chart.guitar_note_track(Difficulty::Easy).notes()
                == green_note);
        REQUIRE(chart.guitar_note_track(Difficulty::Medium).notes()
                == green_note);
        REQUIRE(chart.guitar_note_track(Difficulty::Hard).notes()
                == green_note);
        REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes()
                == green_note);
    }

    SECTION("Notes are only read from PART GUITAR")
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

        const auto chart = Chart::from_midi(midi);

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes()[0].colour
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

        REQUIRE_THROWS([&] { return Chart::from_midi(midi); }());
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

        REQUIRE_NOTHROW([&] { return Chart::from_midi(midi); }());
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

        const auto chart = Chart::from_midi(midi);

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).notes()[0].colour
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

    const auto chart = Chart::from_midi(midi);

    REQUIRE(chart.guitar_note_track(Difficulty::Expert).solos() == solos);
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

        const auto chart = Chart::from_midi(midi);

        REQUIRE(chart.guitar_note_track(Difficulty::Expert).sp_phrases()
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

        REQUIRE_THROWS([&] { return Chart::from_midi(midi); }());
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
    const auto chart = Chart::from_midi(midi);
    const auto& notes = chart.guitar_note_track(Difficulty::Expert).notes();

    REQUIRE(notes[0].length == 0);
    REQUIRE(notes[1].length == 70);
}
