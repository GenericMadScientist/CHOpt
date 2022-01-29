/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022 Raymond Wright
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

#define BOOST_TEST_MODULE Ini Parser Tests
#include <boost/test/unit_test.hpp>

#include "ini.hpp"

BOOST_AUTO_TEST_CASE(default_ini_values_are_correct)
{
    const auto ini_values = parse_ini("");

    BOOST_CHECK_EQUAL(ini_values.name, "Unknown Song");
    BOOST_CHECK_EQUAL(ini_values.artist, "Unknown Artist");
    BOOST_CHECK_EQUAL(ini_values.charter, "Unknown Charter");
}

BOOST_AUTO_TEST_CASE(values_with_no_spaces_around_equals_are_read)
{
    const char* text
        = "name=Dummy Song\nartist=Dummy Artist\ncharter=Dummy Charter";

    const auto ini_values = parse_ini(text);

    BOOST_CHECK_EQUAL(ini_values.name, "Dummy Song");
    BOOST_CHECK_EQUAL(ini_values.artist, "Dummy Artist");
    BOOST_CHECK_EQUAL(ini_values.charter, "Dummy Charter");
}

BOOST_AUTO_TEST_CASE(values_with_spaces_around_equals_are_read)
{
    const char* text = "name = Dummy Song 2\nartist = Dummy Artist 2\ncharter "
                       "= Dummy Charter 2";

    const auto ini_values = parse_ini(text);

    BOOST_CHECK_EQUAL(ini_values.name, "Dummy Song 2");
    BOOST_CHECK_EQUAL(ini_values.artist, "Dummy Artist 2");
    BOOST_CHECK_EQUAL(ini_values.charter, "Dummy Charter 2");
}

BOOST_AUTO_TEST_CASE(equals_must_be_the_character_after_the_key)
{
    const char* text = "name_length=0\nartist_length=0\ncharter_length=0";

    const auto ini_values = parse_ini(text);

    BOOST_CHECK_EQUAL(ini_values.name, "Unknown Song");
    BOOST_CHECK_EQUAL(ini_values.artist, "Unknown Artist");
    BOOST_CHECK_EQUAL(ini_values.charter, "Unknown Charter");
}

BOOST_AUTO_TEST_CASE(frets_is_a_synonym_for_charter)
{
    const char* text = "frets=GMS";

    const auto ini_values = parse_ini(text);

    BOOST_CHECK_EQUAL(ini_values.charter, "GMS");
}

BOOST_AUTO_TEST_CASE(blank_frets_after_charter_is_ignored)
{
    const char* text = "charter = Haggis\nfrets = ";

    const auto ini_values = parse_ini(text);

    BOOST_CHECK_EQUAL(ini_values.charter, "Haggis");
}

BOOST_AUTO_TEST_CASE(utf16le_inis_are_read_correctly)
{
    const std::string text {
        "\xFF\xFE\x6E\x00\x61\x00\x6D\x00\x65\x00\x3D\x00\x54\x00\x65\x00\x73"
        "\x00\x74\x00",
        20};

    const auto ini_values = parse_ini(text);

    BOOST_CHECK_EQUAL(ini_values.name, "Test");
}
