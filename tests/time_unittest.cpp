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

#include "songparts.hpp"
#include "time.hpp"

TEST_CASE("Beat operations", "Beat")
{
    SECTION(".value() works correctly")
    {
        REQUIRE(Beat(1.0).value() == Approx(1.0));
        REQUIRE(Beat(-1.0).value() == Approx(-1.0));
    }

    SECTION(".to_second() works correctly")
    {
        REQUIRE(Beat(2.0).to_second(120000).value() == Approx(1.0));
        REQUIRE(Beat(8.0).to_second(240000).value() == Approx(2.0));
    }

    SECTION(".to_measure() works correctly")
    {
        REQUIRE(Beat(2.0).to_measure(4.0).value() == Approx(0.5));
        REQUIRE(Beat(8.0).to_measure(2.0).value() == Approx(4.0));
    }

    SECTION("< and > work correctly")
    {
        REQUIRE(Beat(1.0) < Beat(2.0));
        REQUIRE(!(Beat(1.0) > Beat(2.0)));
        REQUIRE(!(Beat(1.0) < Beat(0.5)));
        REQUIRE(Beat(1.0) > Beat(0.5));
        REQUIRE(!(Beat(1.0) < Beat(1.0)));
        REQUIRE(!(Beat(1.0) > Beat(1.0)));
    }

    SECTION("<= and >= work correctly")
    {
        REQUIRE(Beat(1.0) <= Beat(2.0));
        REQUIRE(!(Beat(1.0) >= Beat(2.0)));
        REQUIRE(!(Beat(1.0) <= Beat(0.5)));
        REQUIRE(Beat(1.0) >= Beat(0.5));
        REQUIRE(Beat(1.0) <= Beat(1.0));
        REQUIRE(Beat(1.0) >= Beat(1.0));
    }

    SECTION("+= and + work correctly")
    {
        Beat lhs {1.0};
        Beat rhs {0.5};
        lhs += rhs;

        REQUIRE(lhs.value() == Approx(1.5));
        REQUIRE((Beat(1.0) + Beat(0.5)).value() == Approx(1.5));
    }

    SECTION("-= and - work correctly")
    {
        Beat lhs {1.0};
        Beat rhs {0.5};
        lhs -= rhs;

        REQUIRE(lhs.value() == Approx(0.5));
        REQUIRE((Beat(1.0) - Beat(0.5)).value() == Approx(0.5));
    }

    SECTION("*= and * work correctly")
    {
        Beat lhs {1.0};
        double rhs {0.5};
        lhs *= rhs;

        REQUIRE(lhs.value() == Approx(0.5));
        REQUIRE((Beat(1.0) * 0.5).value() == Approx(0.5));
    }

    SECTION("/ works correctly")
    {
        REQUIRE(Beat(1.0) / Beat(2.0) == Approx(0.5));
        REQUIRE(Beat(2.0) / Beat(1.0) == Approx(2.0));
    }
}

TEST_CASE("Measure operations", "Measure")
{
    SECTION(".value() works correctly")
    {
        REQUIRE(Measure(1.0).value() == Approx(1.0));
        REQUIRE(Measure(-1.0).value() == Approx(-1.0));
    }

    SECTION(".to_beat() works correctly")
    {
        REQUIRE(Measure(1.0).to_beat(4.0).value() == Approx(4.0));
        REQUIRE(Measure(2.0).to_beat(3.0).value() == Approx(6.0));
    }

    SECTION("< and > work correctly")
    {
        REQUIRE(Measure(1.0) < Measure(2.0));
        REQUIRE(!(Measure(1.0) > Measure(2.0)));
        REQUIRE(!(Measure(1.0) < Measure(0.5)));
        REQUIRE(Measure(1.0) > Measure(0.5));
        REQUIRE(!(Measure(1.0) < Measure(1.0)));
        REQUIRE(!(Measure(1.0) > Measure(1.0)));
    }

    SECTION("<= and >= work correctly")
    {
        REQUIRE(Measure(1.0) <= Measure(2.0));
        REQUIRE(!(Measure(1.0) >= Measure(2.0)));
        REQUIRE(!(Measure(1.0) <= Measure(0.5)));
        REQUIRE(Measure(1.0) >= Measure(0.5));
        REQUIRE(Measure(1.0) <= Measure(1.0));
        REQUIRE(Measure(1.0) >= Measure(1.0));
    }

    SECTION("+= and + work correctly")
    {
        Measure lhs {1.0};
        Measure rhs {0.5};
        lhs += rhs;

        REQUIRE(lhs.value() == Approx(1.5));
        REQUIRE((Measure(1.0) + Measure(0.5)).value() == Approx(1.5));
    }

    SECTION("-= and - work correctly")
    {
        Measure lhs {1.0};
        Measure rhs {0.5};
        lhs -= rhs;

        REQUIRE(lhs.value() == Approx(0.5));
        REQUIRE((Measure(1.0) - Measure(0.5)).value() == Approx(0.5));
    }

    SECTION("*= and * work correctly")
    {
        Measure lhs {1.0};
        double rhs {0.5};
        lhs *= rhs;

        REQUIRE(lhs.value() == Approx(0.5));
        REQUIRE((Measure(1.0) * 0.5).value() == Approx(0.5));
    }

    SECTION("/ works correctly")
    {
        REQUIRE(Measure(1.0) / Measure(2.0) == Approx(0.5));
        REQUIRE(Measure(2.0) / Measure(1.0) == Approx(2.0));
    }
}

TEST_CASE("Second operations", "Second")
{
    SECTION(".value() works correctly")
    {
        REQUIRE(Second(1.0).value() == Approx(1.0));
        REQUIRE(Second(-1.0).value() == Approx(-1.0));
    }

    SECTION(".to_beat() works correctly")
    {
        REQUIRE(Second(1.0).to_beat(120000).value() == Approx(2.0));
        REQUIRE(Second(2.0).to_beat(240000).value() == Approx(8.0));
    }

    SECTION("< and > work correctly")
    {
        REQUIRE(Second(1.0) < Second(2.0));
        REQUIRE(!(Second(1.0) > Second(2.0)));
        REQUIRE(!(Second(1.0) < Second(0.5)));
        REQUIRE(Second(1.0) > Second(0.5));
        REQUIRE(!(Second(1.0) < Second(1.0)));
        REQUIRE(!(Second(1.0) > Second(1.0)));
    }

    SECTION("<= and >= work correctly")
    {
        REQUIRE(Second(1.0) <= Second(2.0));
        REQUIRE(!(Second(1.0) >= Second(2.0)));
        REQUIRE(!(Second(1.0) <= Second(0.5)));
        REQUIRE(Second(1.0) >= Second(0.5));
        REQUIRE(Second(1.0) <= Second(1.0));
        REQUIRE(Second(1.0) >= Second(1.0));
    }

    SECTION("+= and + work correctly")
    {
        Second lhs {1.0};
        Second rhs {0.5};
        lhs += rhs;

        REQUIRE(lhs.value() == Approx(1.5));
        REQUIRE((Second(1.0) + Second(0.5)).value() == Approx(1.5));
    }

    SECTION("-= and - work correctly")
    {
        Second lhs {1.0};
        Second rhs {0.5};
        lhs -= rhs;

        REQUIRE(lhs.value() == Approx(0.5));
        REQUIRE((Second(1.0) - Second(0.5)).value() == Approx(0.5));
    }

    SECTION("*= and * work correctly")
    {
        Second lhs {1.0};
        double rhs {0.5};
        lhs *= rhs;

        REQUIRE(lhs.value() == Approx(0.5));
        REQUIRE((Second(1.0) * 0.5).value() == Approx(0.5));
    }

    SECTION("/ works correctly")
    {
        REQUIRE(Second(1.0) / Second(2.0) == Approx(0.5));
        REQUIRE(Second(2.0) / Second(1.0) == Approx(2.0));
    }
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
