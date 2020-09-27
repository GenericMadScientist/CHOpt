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

TEST_CASE("Default ini values are correct")
{
    const char* text = "";

    const auto ini_values = parse_ini(text);

    REQUIRE(ini_values.name == "Unknown Song");
    REQUIRE(ini_values.artist == "Unknown Artist");
    REQUIRE(ini_values.charter == "Unknown Charter");
}

TEST_CASE("Values are read with no spaces around the =")
{
    const char* text
        = "song=Dummy Song\nartist=Dummy Artist\ncharter=Dummy Charter";

    const auto ini_values = parse_ini(text);

    REQUIRE(ini_values.name == "Dummy Song");
    REQUIRE(ini_values.artist == "Dummy Artist");
    REQUIRE(ini_values.charter == "Dummy Charter");
}

TEST_CASE("Values are read with spaces around the =")
{
    const char* text = "song = Dummy Song 2\nartist = Dummy Artist 2\ncharter "
                       "= Dummy Charter 2";

    const auto ini_values = parse_ini(text);

    REQUIRE(ini_values.name == "Dummy Song 2");
    REQUIRE(ini_values.artist == "Dummy Artist 2");
    REQUIRE(ini_values.charter == "Dummy Charter 2");
}
