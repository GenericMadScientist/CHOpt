/*
 *  chopt - Star Power optimiser for Clone Hero
 *  Copyright (C) 2020  Raymond Wright
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "catch.hpp"

#include "chart.hpp"

TEST_CASE("Chart reads resolution and offset", "[Song]")
{
    SECTION("Defaults are 192 Res and 0 Offset")
    {
        auto text = "[Song]\n{\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n";
        const auto chart = Chart(text);
        const auto DEFAULT_RESOLUTION = 192.F;

        REQUIRE(chart.get_resolution() == DEFAULT_RESOLUTION);
        REQUIRE(chart.get_offset() == 0.F);
    }
    SECTION("Defaults are overriden by specified values")
    {
        auto text = "[Song]\n{\nResolution = 200\nOffset = "
                    "100\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n";
        const auto chart = Chart(text);
        const auto RESOLUTION = 200.F;

        REQUIRE(chart.get_resolution() == RESOLUTION);
        REQUIRE(chart.get_offset() == 100.F);
    }
}

TEST_CASE("Chart reads sync track correctly", "[SyncTrack]")
{
    auto text = "[Song]\n{\n}\n[SyncTrack]\n{\n0 = B 200000\n0 = TS 4\n768 = "
                "TS 4 1\n}\n[Events]\n{\n}\n";
    const auto chart = Chart(text);
    const auto time_sigs = std::vector<TimeSignature>({{0, 4, 4}, {768, 4, 2}});
    const auto bpms = std::vector<BPM>({{0, 200000}});

    REQUIRE(chart.get_time_sigs() == time_sigs);
    REQUIRE(chart.get_bpms() == bpms);
}

TEST_CASE("Chart reads events correctly", "[Events]")
{
    auto text = "[Song]\n{\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n768 = E "
                "\"section intro\"\n}\n";
    const auto chart = Chart(text);
    const auto sections = std::vector<Section>({{768, "intro"}});

    REQUIRE(chart.get_sections() == sections);
}

TEST_CASE("Chart reads easy note track correctly", "[Easy]")
{
    auto text = "[Song]\n{\n}\n[SyncTrack]\n{\n}\n[Events]\n{\n}\n[EasySingle]"
                "\n{\n768 = N 0 0\n768 = S 2 100\n}\n";
    const auto chart = Chart(text);
    const auto note_track
        = NoteTrack {{{768, 0, NoteColour::Green}}, {{768, 100}}, {}};

    REQUIRE(chart.get_note_track(Difficulty::Easy) == note_track);
}
