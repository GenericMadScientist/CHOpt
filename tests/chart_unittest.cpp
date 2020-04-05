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
TEST_CASE("NoteTrack ctor maintains invariants", "NoteTrack")
{
    SECTION("Notes are sorted")
    {
        const auto notes = std::vector<Note>({{768}, {384}});
        const auto track = NoteTrack(notes, {}, {});
        const auto sorted_notes = std::vector<Note>({{384}, {768}});

        REQUIRE(track.notes() == sorted_notes);
    }

    SECTION("Notes of the same colour and position are merged")
    {
        const auto notes = std::vector<Note>({{768, 0}, {768, 768}});
        const auto track = NoteTrack(notes, {}, {});
        const auto required_notes = std::vector<Note>({{768, 768}});

        REQUIRE(track.notes() == required_notes);

        const auto second_notes = std::vector<Note>({{768, 768}, {768, 0}});
        const auto second_track = NoteTrack(second_notes, {}, {});
        const auto second_required_notes = std::vector<Note>({{768, 0}});

        REQUIRE(second_track.notes() == second_required_notes);
    }

    SECTION("Notes of different colours are dealt with separately")
    {
        const auto notes = std::vector<Note>({{768, 0, NoteColour::Green},
                                              {768, 0, NoteColour::Red},
                                              {768, 768, NoteColour::Green}});
        const auto track = NoteTrack(notes, {}, {});
        const auto required_notes = std::vector<Note>(
            {{768, 768, NoteColour::Green}, {768, 0, NoteColour::Red}});

        REQUIRE(track.notes() == required_notes);
    }

    SECTION("Empty SP phrases are culled")
    {
        const auto notes = std::vector<Note>({{768}});
        const auto phrases
            = std::vector<StarPower>({{0, 100}, {700, 100}, {1000, 100}});
        const auto track = NoteTrack(notes, phrases, {});
        const auto required_phrases = std::vector<StarPower>({{700, 100}});

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases are sorted")
    {
        const auto notes = std::vector<Note>({{768}, {1000}});
        const auto phrases = std::vector<StarPower>({{1000, 1}, {768, 1}});
        const auto track = NoteTrack(notes, phrases, {});
        const auto required_phrases
            = std::vector<StarPower>({{768, 1}, {1000, 1}});

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases do not overlap")
    {
        const auto notes = std::vector<Note>({{768}, {1000}});
        const auto phrases = std::vector<StarPower>({{768, 1000}, {900, 150}});
        const auto track = NoteTrack(notes, phrases, {});
        const auto required_phrases
            = std::vector<StarPower>({{768, 132}, {900, 150}});

        REQUIRE(track.sp_phrases() == required_phrases);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("SyncTrack ctor maintains invariants", "SyncTrack")
{
    SECTION("TimeSignatures are sorted by position")
    {
        const auto track = SyncTrack({{0, 4, 4}, {2000, 3, 3}, {1000, 2, 2}});
        const auto expected_tses = std::vector<TimeSignature>(
            {{0, 4, 4}, {1000, 2, 2}, {2000, 3, 3}});

        REQUIRE(track.time_sigs() == expected_tses);
    }

    SECTION("No two TimeSignatures have the same position")
    {
        const auto track = SyncTrack({{0, 4, 4}, {0, 3, 4}});
        const auto expected_tses = std::vector<TimeSignature>({{0, 3, 4}});

        REQUIRE(track.time_sigs() == expected_tses);
    }

    SECTION("time_sigs() is never empty")
    {
        const auto track = SyncTrack();
        const auto expected_tses = std::vector<TimeSignature>({{0, 4, 4}});

        REQUIRE(track.time_sigs() == expected_tses);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads resolution", "Song")
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

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads sync track correctly", "SyncTrack")
{
    const char* text
        = "[Song]\n{\n}\n[SyncTrack]\n{\n0 = B 200000\n0 = TS 4\n768 = "
          "TS 4 1\n}\n[Events]\n{\n}\n[ExpertSingle]\n{\n768 = N 0 0\n}";
    const auto sync_track = Chart::parse_chart(text).sync_track();
    const auto time_sigs = std::vector<TimeSignature>({{0, 4, 4}, {768, 4, 2}});

    REQUIRE(sync_track.time_sigs() == time_sigs);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart reads easy note track correctly", "Easy")
{
    const char* text
        = "[Song]\n{\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[EasySingle]"
          "\n{\n768 = N 0 0\n768 = S 2 100\n}\n";
    const auto chart = Chart::parse_chart(text);
    const auto note_track
        = NoteTrack {{{768, 0, NoteColour::Green}}, {{768, 100}}, {}};

    REQUIRE(chart.note_track(Difficulty::Easy) == note_track);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart skips UTF-8 BOM", "BOM")
{
    const char* text
        = "\xEF\xBB\xBF[Song]\n{\nOffset = "
          "100\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[ExpertSingle]\n{"
          "\n768 = N 0 0\n}";
    const auto resolution = Chart::parse_chart(text).resolution();

    REQUIRE(resolution == 192);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart can end without a newline", "End-NL")
{
    const char* text
        = "\xEF\xBB\xBF[Song]\n{\nOffset = "
          "100\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[ExpertSingle]\n{"
          "\n768 = N 0 0\n}";
    const auto resolution = Chart::parse_chart(text).resolution();

    REQUIRE(resolution == 192);
}

// Last checked: 24.0.1555-master
TEST_CASE("Chart does not need sections in usual order", "Section order")
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
        const auto notes = std::vector<Note>({{768}});

        REQUIRE(chart.resolution() == 200);
        REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Only first non-empty part of note sections matter", "Note split")
{
    SECTION("Later non-empty sections are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n768 = N 0 0\n}\n[ExpertSingle]\n{\n768 = N "
              "1 0\n}";
        const auto chart = Chart::parse_chart(text);
        const auto notes = std::vector<Note>({{768, 0, NoteColour::Green}});

        REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
    }

    SECTION("Leading empty sections are ignored")
    {
        const char* text
            = "[ExpertSingle]\n{\n}\n[ExpertSingle]\n{\n768 = N 1 0\n}";
        const auto chart = Chart::parse_chart(text);
        const auto notes = std::vector<Note>({{768, 0, NoteColour::Red}});

        REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
    }
}

// Last checked: 23.2.2
TEST_CASE("Notes should be sorted", "NoteSort")
{
    const char* text = "[ExpertSingle]\n{\n768 = N 0 0\n384 = N 0 0\n}";
    const auto chart = Chart::parse_chart(text);
    const auto notes = std::vector<Note>(
        {{384, 0, NoteColour::Green}, {768, 0, NoteColour::Green}});

    REQUIRE(chart.note_track(Difficulty::Expert).notes() == notes);
}

// Last checked: 24.0.1555-master
TEST_CASE("Notes with extra spaces", "NoteSpaceSkip")
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
        const auto required_notes = std::vector<Note>({{768}});

        REQUIRE(chart.note_track(Difficulty::Expert).notes() == required_notes);
    }
}
