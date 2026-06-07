/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2026 Raymond Wright
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

#include <boost/test/unit_test.hpp>

#include "activationendset.hpp"

using IntSet = ActivationEndSet<int>;

BOOST_AUTO_TEST_SUITE(no_elements_added_set)

BOOST_AUTO_TEST_CASE(contains_returns_false_within_range)
{
    const IntSet set {0, 3};

    BOOST_CHECK(!set.contains(1));
}

BOOST_AUTO_TEST_CASE(contains_returns_false_before_range)
{
    const IntSet set {0, 3};

    BOOST_CHECK(!set.contains(-1));
}

BOOST_AUTO_TEST_CASE(contains_returns_false_after_range)
{
    const IntSet set {0, 3};

    BOOST_CHECK(!set.contains(3));
}

BOOST_AUTO_TEST_CASE(lowest_absent_element_is_start_of_range)
{
    const IntSet set {0, 3};

    BOOST_CHECK_EQUAL(set.lowest_absent_element(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(set_with_element_in_middle_of_range)

BOOST_AUTO_TEST_CASE(contains_returns_true_for_added_element)
{
    IntSet set {0, 3};
    set.add(1);

    BOOST_CHECK(set.contains(1));
}

BOOST_AUTO_TEST_CASE(
    contains_returns_false_for_element_in_range_that_wasnt_added)
{
    IntSet set {0, 3};
    set.add(1);

    BOOST_CHECK(!set.contains(2));
}

BOOST_AUTO_TEST_CASE(contains_returns_false_before_range)
{
    IntSet set {0, 3};
    set.add(1);

    BOOST_CHECK(!set.contains(-1));
}

BOOST_AUTO_TEST_CASE(contains_returns_false_after_range)
{
    IntSet set {0, 3};
    set.add(1);

    BOOST_CHECK(!set.contains(3));
}

BOOST_AUTO_TEST_CASE(lowest_absent_element_is_start_of_range)
{
    IntSet set {0, 3};
    set.add(1);

    BOOST_CHECK_EQUAL(set.lowest_absent_element(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(set_with_element_added_at_start_of_range)

BOOST_AUTO_TEST_CASE(contains_returns_true_for_added_element)
{
    IntSet set {0, 3};
    set.add(0);

    BOOST_CHECK(set.contains(0));
}

BOOST_AUTO_TEST_CASE(
    contains_returns_false_for_element_in_range_that_wasnt_added)
{
    IntSet set {0, 3};
    set.add(0);

    BOOST_CHECK(!set.contains(2));
}

BOOST_AUTO_TEST_CASE(contains_returns_false_before_range)
{
    IntSet set {0, 3};
    set.add(0);

    BOOST_CHECK(!set.contains(-1));
}

BOOST_AUTO_TEST_CASE(contains_returns_false_after_range)
{
    IntSet set {0, 3};
    set.add(0);

    BOOST_CHECK(!set.contains(3));
}

BOOST_AUTO_TEST_CASE(lowest_absent_element_is_first_element_after_start)
{
    IntSet set {0, 3};
    set.add(0);

    BOOST_CHECK_EQUAL(set.lowest_absent_element(), 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(next_absent_element)

BOOST_AUTO_TEST_CASE(returns_first_next_element_out_of_set)
{
    IntSet set {0, 3};
    set.add(1);

    BOOST_CHECK_EQUAL(set.next_absent_element(0), 2);
}

BOOST_AUTO_TEST_CASE(returns_max_of_range_if_all_subsequent_elements_in_set)
{
    const IntSet set {0, 3};

    BOOST_CHECK_EQUAL(set.next_absent_element(2), 3);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(temporary_elements)

BOOST_AUTO_TEST_CASE(contains_includes_temporary_elements)
{
    IntSet set {0, 3};
    set.add_temporary_element(1);

    BOOST_CHECK(set.contains(1));
}

BOOST_AUTO_TEST_CASE(contains_ignores_temporary_elements_after_clear)
{
    IntSet set {0, 3};
    set.add_temporary_element(1);
    set.clear_temporary_elements();

    BOOST_CHECK(!set.contains(1));
}

BOOST_AUTO_TEST_CASE(contains_retains_normal_elements_after_clear)
{
    IntSet set {0, 3};
    set.add(1);
    set.clear_temporary_elements();

    BOOST_CHECK(set.contains(1));
}

BOOST_AUTO_TEST_CASE(lowest_absent_element_respects_temporary_elements)
{
    IntSet set {0, 3};
    set.add_temporary_element(0);

    BOOST_CHECK_EQUAL(set.lowest_absent_element(), 1);
}

BOOST_AUTO_TEST_CASE(next_absent_element_respects_temporary_elements)
{
    IntSet set {0, 3};
    set.add_temporary_element(1);

    BOOST_CHECK_EQUAL(set.next_absent_element(0), 2);
}

BOOST_AUTO_TEST_SUITE_END()
