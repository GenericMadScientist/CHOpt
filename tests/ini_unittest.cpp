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

#include "ini.hpp"

TEST_CASE("parse_ini parses the song name correctly")
{
    SECTION("Default value is Unknown Song")
    {
        const char* text = "";

        const auto ini_values = parse_ini(text);

        REQUIRE(ini_values.name == "Unknown Song");
    }

    SECTION("song value is read with no spaces around the =")
    {
        const char* text = "song=Dummy Song";

        const auto ini_values = parse_ini(text);

        REQUIRE(ini_values.name == "Dummy Song");
    }

    SECTION("song value is read with spaces around the =")
    {
        const char* text = "song = Dummy Song 2";

        const auto ini_values = parse_ini(text);

        REQUIRE(ini_values.name == "Dummy Song 2");
    }

    SECTION("song value is read in a multi-line file")
    {
        const char* text = "[song]\nsong=Dummy Song 3\ndiff=6";

        const auto ini_values = parse_ini(text);

        REQUIRE(ini_values.name == "Dummy Song 3");
    }
}
