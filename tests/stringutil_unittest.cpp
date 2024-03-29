/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024 Raymond Wright
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

#include <stdexcept>

#include <boost/test/unit_test.hpp>

#include "stringutil.hpp"

BOOST_AUTO_TEST_CASE(break_off_newline_works_correctly)
{
    std::string_view data = "Hello\nThe\rre\r\nWorld!";

    BOOST_CHECK_EQUAL(break_off_newline(data), "Hello");
    BOOST_CHECK_EQUAL(data, "The\rre\r\nWorld!");
    BOOST_CHECK_EQUAL(break_off_newline(data), "The\rre");
    BOOST_CHECK_EQUAL(data, "World!");
}

BOOST_AUTO_TEST_CASE(skip_whitespace_works_correctly)
{
    BOOST_CHECK_EQUAL(skip_whitespace("Hello"), "Hello");
    BOOST_CHECK_EQUAL(skip_whitespace("  Hello"), "Hello");
    BOOST_CHECK_EQUAL(skip_whitespace("H  ello"), "H  ello");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_strips_utf8_bom)
{
    const std::string text {"\xEF\xBB\xBF\x6E"};

    BOOST_CHECK_EQUAL(to_utf8_string(text), "n");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_treats_no_bom_string_as_utf8)
{
    const std::string text {"Hello"};

    BOOST_CHECK_EQUAL(to_utf8_string(text), "Hello");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_correctly_converts_from_utf16le_to_utf8)
{
    const std::string text {
        "\xFF\xFE\x6E\x00\x61\x00\x6D\x00\x65\x00\x3D\x00\x54\x00\x65\x00\x73"
        "\x00\x74\x00",
        20};

    BOOST_CHECK_EQUAL(to_utf8_string(text), "name=Test");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_correctly_converts_from_latin_1_strings)
{
    const std::string text {"\xE9\x30"};

    BOOST_CHECK_EQUAL(to_utf8_string(text), "é0");
}

BOOST_AUTO_TEST_CASE(to_ordinal_works_correctly)
{
    BOOST_CHECK_EQUAL(to_ordinal(0), "0th");
    BOOST_CHECK_EQUAL(to_ordinal(1), "1st");
    BOOST_CHECK_EQUAL(to_ordinal(2), "2nd");
    BOOST_CHECK_EQUAL(to_ordinal(3), "3rd");
    BOOST_CHECK_EQUAL(to_ordinal(4), "4th");
    BOOST_CHECK_EQUAL(to_ordinal(11), "11th");
    BOOST_CHECK_EQUAL(to_ordinal(12), "12th");
    BOOST_CHECK_EQUAL(to_ordinal(13), "13th");
    BOOST_CHECK_THROW([&] { return to_ordinal(-1); }(), std::exception);
}
