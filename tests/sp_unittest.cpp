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

#include "sp.hpp"

TEST_CASE("SpBar methods", "SpBar")
{
    SECTION("add_phrase() works correctly")
    {
        SpBar sp_bar {0.0, 0.25};
        sp_bar.add_phrase();

        REQUIRE(sp_bar == SpBar {0.25, 0.5});

        sp_bar = {0.8, 1.0};
        sp_bar.add_phrase();

        REQUIRE(sp_bar == SpBar {1.0, 1.0});
    }

    SECTION("full_enough_to_activate() works correctly")
    {
        SpBar sp_bar {0.49, 0.49};

        REQUIRE(!sp_bar.full_enough_to_activate());

        sp_bar = {0.0, 0.5};

        REQUIRE(sp_bar.full_enough_to_activate());
    }
}
