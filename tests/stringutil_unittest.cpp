/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024, 2026 Raymond Wright
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
