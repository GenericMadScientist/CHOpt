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

#include "catch.hpp"

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
