/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021 Raymond Wright
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

#include <array>

#include "catch.hpp"

#include "timeconverter.hpp"

static bool operator==(const BPM& lhs, const BPM& rhs)
{
    return std::tie(lhs.position, lhs.bpm) == std::tie(rhs.position, rhs.bpm);
}

static bool operator==(const TimeSignature& lhs, const TimeSignature& rhs)
{
    return std::tie(lhs.position, lhs.numerator, lhs.denominator)
        == std::tie(rhs.position, rhs.numerator, rhs.denominator);
}

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

    SECTION("BPMs must not be zero or negative")
    {
        REQUIRE_THROWS_AS(([&] {
                              return SyncTrack {{}, {{192, 0}}};
                          })(),
                          ParseError);
        REQUIRE_THROWS_AS(([&] {
                              return SyncTrack {{}, {{192, -1}}};
                          })(),
                          ParseError);
    }

    SECTION("Time Signatures must be positive/positive")
    {
        REQUIRE_THROWS_AS(([&] {
                              return SyncTrack {{{0, 0, 4}}, {}};
                          })(),
                          ParseError);
        REQUIRE_THROWS_AS(([&] {
                              return SyncTrack {{{0, -1, 4}}, {}};
                          })(),
                          ParseError);
        REQUIRE_THROWS_AS(([&] {
                              return SyncTrack {{{0, 4, 0}}, {}};
                          })(),
                          ParseError);
        REQUIRE_THROWS_AS(([&] {
                              return SyncTrack {{{0, 4, -1}}, {}};
                          })(),
                          ParseError);
    }
}

TEST_CASE("Speedup returns the correct SyncTrack")
{
    const SyncTrack sync_track {{{0, 4, 4}}, {{0, 120000}, {192, 240000}}};
    const std::vector<BPM> expected_bpms {{0, 180000}, {192, 360000}};
    const std::vector<TimeSignature> expected_tses {{0, 4, 4}};

    const auto speedup = sync_track.speedup(150);

    REQUIRE(speedup.bpms() == expected_bpms);
    REQUIRE(speedup.time_sigs() == expected_tses);
}

// Last checked: 24.0.1555-master
TEST_CASE("Beats to seconds conversion", "Beats<->S")
{
    SyncTrack track {{{0, 4, 4}}, {{0, 150000}, {800, 200000}}};
    TimeConverter converter {track, 200, ChGuitarEngine(), {}};
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.0};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 1.9};

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.beats_to_seconds(Beat(beats.at(i))).value()
                == Approx(seconds.at(i)));
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.seconds_to_beats(Second(seconds.at(i))).value()
                == Approx(beats.at(i)));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Beats to measures conversion", "Beats<->Measures")
{
    SyncTrack track {{{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}}, {}};
    TimeConverter converter {track, 200, ChGuitarEngine(), {}};
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.5, 6.5};
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.beats_to_measures(Beat(beats.at(i))).value()
                == Approx(measures.at(i)));
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.measures_to_beats(Measure(measures.at(i))).value()
                == Approx(beats.at(i)));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Measures to seconds conversion", "Measures<->S")
{
    SyncTrack track {{{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}},
                     {{0, 150000}, {800, 200000}}};
    TimeConverter converter {track, 200, ChGuitarEngine(), {}};
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 2.05, 2.35};

    for (auto i = 0U; i < measures.size(); ++i) {
        REQUIRE(converter.measures_to_seconds(Measure(measures.at(i))).value()
                == Approx(seconds.at(i)));
    }

    for (auto i = 0U; i < measures.size(); ++i) {
        REQUIRE(converter.seconds_to_measures(Second(seconds.at(i))).value()
                == Approx(measures.at(i)));
    }
}
