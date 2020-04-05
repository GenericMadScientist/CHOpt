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

#include <array>

#include "catch.hpp"

#include "chart.hpp"
#include "time.hpp"

TEST_CASE("Beat operations", "Beat")
{
    SECTION(".value() works correctly")
    {
        REQUIRE(Beat(1.0).value() == Approx(1.0));
        REQUIRE(Beat(-1.0).value() == Approx(-1.0));
    }

    SECTION(".to_measure() works correctly")
    {
        REQUIRE(Beat(2.0).to_measure(4.0) == Measure(0.5));
        REQUIRE(Beat(8.0).to_measure(2.0) == Measure(4.0));
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

    SECTION("== and != work correctly")
    {
        REQUIRE(Beat(1.0) == Beat(1.0));
        REQUIRE(!(Beat(-1.0) == Beat(1.0)));
        REQUIRE(Beat(-1.0) != Beat(1.0));
        REQUIRE(!(Beat(1.0) != Beat(1.0)));
    }

    SECTION("+= and + work correctly")
    {
        Beat lhs {1.0};
        Beat rhs {0.5};
        lhs += rhs;

        REQUIRE(lhs == Beat(1.5));
        REQUIRE(Beat(1.0) + Beat(0.5) == Beat(1.5));
    }

    SECTION("-= and - work correctly")
    {
        Beat lhs {1.0};
        Beat rhs {0.5};
        lhs -= rhs;

        REQUIRE(lhs == Beat(0.5));
        REQUIRE(Beat(1.0) - Beat(0.5) == Beat(0.5));
    }

    SECTION("*= and * work correctly")
    {
        Beat lhs {1.0};
        double rhs {0.5};
        lhs *= rhs;

        REQUIRE(lhs == Beat(0.5));
        REQUIRE(Beat(1.0) * 0.5 == Beat(0.5));
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
        REQUIRE(Measure(1.0).to_beat(4.0) == Beat(4.0));
        REQUIRE(Measure(2.0).to_beat(3.0) == Beat(6.0));
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

    SECTION("== and != work correctly")
    {
        REQUIRE(Measure(1.0) == Measure(1.0));
        REQUIRE(!(Measure(-1.0) == Measure(1.0)));
        REQUIRE(Measure(-1.0) != Measure(1.0));
        REQUIRE(!(Measure(1.0) != Measure(1.0)));
    }

    SECTION("+= and + work correctly")
    {
        Measure lhs {1.0};
        Measure rhs {0.5};
        lhs += rhs;

        REQUIRE(lhs == Measure(1.5));
        REQUIRE(Measure(1.0) + Measure(0.5) == Measure(1.5));
    }

    SECTION("-= and - work correctly")
    {
        Measure lhs {1.0};
        Measure rhs {0.5};
        lhs -= rhs;

        REQUIRE(lhs == Measure(0.5));
        REQUIRE(Measure(1.0) - Measure(0.5) == Measure(0.5));
    }

    SECTION("*= and * work correctly")
    {
        Measure lhs {1.0};
        double rhs {0.5};
        lhs *= rhs;

        REQUIRE(lhs == Measure(0.5));
        REQUIRE(Measure(1.0) * 0.5 == Measure(0.5));
    }

    SECTION("/ works correctly")
    {
        REQUIRE(Measure(1.0) / Measure(2.0) == Approx(0.5));
        REQUIRE(Measure(2.0) / Measure(1.0) == Approx(2.0));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Beats to measures conversion", "Beats<->Measures")
{
    const auto track = SyncTrack({{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}}, {});
    const auto header = SongHeader(0.F, 200);
    const auto converter = TimeConverter(track, header);
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
