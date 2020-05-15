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
        std::vector<Note> notes {{768}, {384}};
        NoteTrack track {notes, {}, {}};
        std::vector<Note> sorted_notes {{384}, {768}};

        REQUIRE(track.notes() == sorted_notes);
    }

    SECTION("Notes of the same colour and position are merged")
    {
        std::vector<Note> notes {{768, 0}, {768, 768}};
        NoteTrack track {notes, {}, {}};
        std::vector<Note> required_notes {{768, 768}};

        REQUIRE(track.notes() == required_notes);

        std::vector<Note> second_notes {{768, 768}, {768, 0}};
        NoteTrack second_track {second_notes, {}, {}};
        std::vector<Note> second_required_notes {{768, 0}};

        REQUIRE(second_track.notes() == second_required_notes);
    }

    SECTION("Notes of different colours are dealt with separately")
    {
        std::vector<Note> notes {{768, 0, NoteColour::Green},
                                 {768, 0, NoteColour::Red},
                                 {768, 768, NoteColour::Green}};
        NoteTrack track {notes, {}, {}};
        std::vector<Note> required_notes {{768, 768, NoteColour::Green},
                                          {768, 0, NoteColour::Red}};

        REQUIRE(track.notes() == required_notes);
    }

    SECTION("Empty SP phrases are culled")
    {
        std::vector<Note> notes {{768}};
        std::vector<StarPower> phrases {{0, 100}, {700, 100}, {1000, 100}};
        NoteTrack track {notes, phrases, {}};
        std::vector<StarPower> required_phrases {{700, 100}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases are sorted")
    {
        std::vector<Note> notes {{768}, {1000}};
        std::vector<StarPower> phrases {{1000, 1}, {768, 1}};
        NoteTrack track {notes, phrases, {}};
        std::vector<StarPower> required_phrases {{768, 1}, {1000, 1}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases do not overlap")
    {
        std::vector<Note> notes {{768}, {1000}};
        std::vector<StarPower> phrases {{768, 1000}, {900, 150}};
        NoteTrack track {notes, phrases, {}};
        std::vector<StarPower> required_phrases {{768, 132}, {900, 150}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("Solos are sorted")
    {
        std::vector<Note> notes {{0}, {768}};
        std::vector<Solo> solos {{768, 868, 100}, {0, 100, 100}};
        NoteTrack track {notes, {}, solos};
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
    NoteTrack note_track {{{768, 0, NoteColour::Green}}, {{768, 100}}, {}};

    REQUIRE(chart.note_track(Difficulty::Easy) == note_track);
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
        std::vector<Note> notes {{768}};
        std::vector<BPM> bpms {{0, 200000}};

        REQUIRE(chart.resolution() == 200);
        REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
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
        std::vector<Note> notes {{768, 0, NoteColour::Green}};

        REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
    }

    SECTION("Leading empty sections are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n}\n[ExpertSingle]\n{\n768 = N 1 0\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Note> notes {{768, 0, NoteColour::Red}};

        REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
    }
}

// Last checked: 23.2.2
TEST_CASE("Notes should be sorted")
{
    const char* text = "[ExpertSingle]\n{\n768 = N 0 0\n384 = N 0 0\n}";
    const auto chart = Chart::parse_chart(text);
    std::vector<Note> notes {{384, 0, NoteColour::Green},
                             {768, 0, NoteColour::Green}};

    REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
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
        std::vector<Note> required_notes {{768}};

        REQUIRE(chart.note_track(Difficulty::Expert).notes() == required_notes);
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

        REQUIRE(chart.note_track(Difficulty::Expert).solos() == required_solos);
    }

    SECTION("Chords are not counted double")
    {
        const char* text = "[ExpertSingle]\n{\n0 = E solo\n100 = N 0 0\n100 = "
                           "N 1 0\n200 = E soloend\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Solo> required_solos {{0, 200, 100}};

        REQUIRE(chart.note_track(Difficulty::Expert).solos() == required_solos);
    }

    SECTION("Empty solos are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n0 = N 0 0\n100 = E solo\n200 = E soloend\n}";
        const auto chart = Chart::parse_chart(text);

        REQUIRE(chart.note_track(Difficulty::Expert).solos().empty());
    }

    SECTION("Repeated solo starts and ends don't matter")
    {
        const char* text = "[ExpertSingle]\n{\n0 = E solo\n100 = E solo\n100 = "
                           "N 0 0\n200 = E soloend\n300 = E soloend\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Solo> required_solos {{0, 200, 100}};

        REQUIRE(chart.note_track(Difficulty::Expert).solos() == required_solos);
    }

    SECTION("Solo markers are sorted")
    {
        const char* text
            = "[ExpertSingle]\n{\n384 = E soloend\n192 = N 0 0\n0 = E solo\n}";
        const auto chart = Chart::parse_chart(text);
        std::vector<Solo> required_solos {{0, 384, 100}};

        REQUIRE(chart.note_track(Difficulty::Expert).solos() == required_solos);
    }
}
