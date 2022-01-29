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

#include <stdexcept>

#define BOOST_TEST_MODULE Stringutils Tests
#include <boost/test/unit_test.hpp>

#include "stringutil.hpp"

BOOST_AUTO_TEST_CASE(break_off_newline_works_correctly)
{
    std::string_view data = "Hello\nThere\r\nWorld!";

    BOOST_TEST(break_off_newline(data) == "Hello");
    BOOST_TEST(data == "There\r\nWorld!");
    BOOST_TEST(break_off_newline(data) == "There");
    BOOST_TEST(data == "World!");
}

BOOST_AUTO_TEST_CASE(skip_whitespace_works_correctly)
{
    BOOST_TEST(skip_whitespace("Hello") == "Hello");
    BOOST_TEST(skip_whitespace("  Hello") == "Hello");
    BOOST_TEST(skip_whitespace("H  ello") == "H  ello");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_strips_utf8_bom)
{
    const std::string text {"\xEF\xBB\xBF\x6E"};

    BOOST_TEST(to_utf8_string(text) == "n");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_treats_no_bom_string_as_utf8)
{
    const std::string text {"Hello"};

    BOOST_TEST(to_utf8_string(text) == "Hello");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_correctly_converts_from_utf16le_to_utf8)
{
    const std::string text {
        "\xFF\xFE\x6E\x00\x61\x00\x6D\x00\x65\x00\x3D\x00\x54\x00\x65\x00\x73"
        "\x00\x74\x00",
        20};

    BOOST_TEST(to_utf8_string(text) == "name=Test");
}

BOOST_AUTO_TEST_CASE(to_utf8_string_throws_on_a_string_with_odd_length)
{
    const std::string text {"\xFF\xFE\x6E"};

    BOOST_CHECK_THROW([&] { return to_utf8_string(text); }(), std::exception);
}

BOOST_AUTO_TEST_CASE(to_ordinal_works_correctly)
{
    BOOST_TEST(to_ordinal(0) == "0th");
    BOOST_TEST(to_ordinal(1) == "1st");
    BOOST_TEST(to_ordinal(2) == "2nd");
    BOOST_TEST(to_ordinal(3) == "3rd");
    BOOST_TEST(to_ordinal(4) == "4th");
    BOOST_TEST(to_ordinal(11) == "11th");
    BOOST_TEST(to_ordinal(12) == "12th");
    BOOST_TEST(to_ordinal(13) == "13th");
    BOOST_CHECK_THROW([&] { return to_ordinal(-1); }(), std::exception);
}
