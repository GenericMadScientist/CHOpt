/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include "time.hpp"

namespace {
std::ostream& operator<<(std::ostream& stream, Beat beat)
{
    stream << beat.value() << 'b';
    return stream;
}

std::ostream& operator<<(std::ostream& stream, Measure measure)
{
    stream << measure.value() << 'm';
    return stream;
}

std::ostream& operator<<(std::ostream& stream, Second second)
{
    stream << second.value() << 's';
    return stream;
}
}

BOOST_AUTO_TEST_SUITE(beat_operations)

BOOST_AUTO_TEST_CASE(value_works_correctly)
{
    BOOST_CHECK_CLOSE(Beat(1.0).value(), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(Beat(-1.0).value(), -1.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(to_second_works_correctly)
{
    BOOST_CHECK_CLOSE(Beat(2.0).to_second(120000).value(), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(Beat(8.0).to_second(240000).value(), 2.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(to_measure_works_correctly)
{
    BOOST_CHECK_CLOSE(Beat(2.0).to_measure(4.0).value(), 0.5, 0.0001);
    BOOST_CHECK_CLOSE(Beat(8.0).to_measure(2.0).value(), 4.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(order_operations_work_correctly)
{
    BOOST_CHECK_LT(Beat(1.0), Beat(2.0));
    BOOST_CHECK_GT(Beat(1.0), Beat(0.5));
    BOOST_CHECK_LE(Beat(1.0), Beat(2.0));
    BOOST_CHECK_GE(Beat(1.0), Beat(0.5));
    BOOST_CHECK_LE(Beat(1.0), Beat(1.0));
    BOOST_CHECK_GE(Beat(1.0), Beat(1.0));
}

BOOST_AUTO_TEST_CASE(addition_works_correctly)
{
    Beat lhs {1.0};
    Beat rhs {0.5};
    lhs += rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 1.5, 0.0001);
    BOOST_CHECK_CLOSE((Beat(1.0) + Beat(0.5)).value(), 1.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(subtraction_works_correctly)
{
    Beat lhs {1.0};
    Beat rhs {0.5};
    lhs -= rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 0.5, 0.0001);
    BOOST_CHECK_CLOSE((Beat(1.0) - Beat(0.5)).value(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(multiplication_works_correctly)
{
    Beat lhs {1.0};
    double rhs {0.5};
    lhs *= rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 0.5, 0.0001);
    BOOST_CHECK_CLOSE((Beat(1.0) * 0.5).value(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(division_works_correctly)
{
    BOOST_CHECK_CLOSE(Beat(1.0) / Beat(2.0), 0.5, 0.0001);
    BOOST_CHECK_CLOSE(Beat(2.0) / Beat(1.0), 2.0, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(measure_operations)

BOOST_AUTO_TEST_CASE(value_works_correctly)
{
    BOOST_CHECK_CLOSE(Measure(1.0).value(), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(Measure(-1.0).value(), -1.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(to_beat_works_correctly)
{
    BOOST_CHECK_CLOSE(Measure(1.0).to_beat(4.0).value(), 4.0, 0.0001);
    BOOST_CHECK_CLOSE(Measure(2.0).to_beat(3.0).value(), 6.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(order_operations_work_correctly)
{
    BOOST_CHECK_LT(Measure(1.0), Measure(2.0));
    BOOST_CHECK_GT(Measure(1.0), Measure(0.5));
    BOOST_CHECK_LE(Measure(1.0), Measure(2.0));
    BOOST_CHECK_GE(Measure(1.0), Measure(0.5));
    BOOST_CHECK_LE(Measure(1.0), Measure(1.0));
    BOOST_CHECK_GE(Measure(1.0), Measure(1.0));
}

BOOST_AUTO_TEST_CASE(addition_works_correctly)
{
    Measure lhs {1.0};
    Measure rhs {0.5};
    lhs += rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 1.5, 0.0001);
    BOOST_CHECK_CLOSE((Measure(1.0) + Measure(0.5)).value(), 1.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(subtraction_works_correctly)
{
    Measure lhs {1.0};
    Measure rhs {0.5};
    lhs -= rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 0.5, 0.0001);
    BOOST_CHECK_CLOSE((Measure(1.0) - Measure(0.5)).value(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(multiplication_works_correctly)
{
    Measure lhs {1.0};
    double rhs {0.5};
    lhs *= rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 0.5, 0.0001);
    BOOST_CHECK_CLOSE((Measure(1.0) * 0.5).value(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(division_works_correctly)
{
    BOOST_CHECK_CLOSE(Measure(1.0) / Measure(2.0), 0.5, 0.0001);
    BOOST_CHECK_CLOSE(Measure(2.0) / Measure(1.0), 2.0, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(second_operations)

BOOST_AUTO_TEST_CASE(value_works_correctly)
{
    BOOST_CHECK_CLOSE(Second(1.0).value(), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(Second(-1.0).value(), -1.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(to_beat_works_correctly)
{
    BOOST_CHECK_CLOSE(Second(1.0).to_beat(120000).value(), 2.0, 0.0001);
    BOOST_CHECK_CLOSE(Second(2.0).to_beat(240000).value(), 8.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(order_operations_work_correctly)
{
    BOOST_CHECK_LT(Second(1.0), Second(2.0));
    BOOST_CHECK_GT(Second(1.0), Second(0.5));
    BOOST_CHECK_LE(Second(1.0), Second(2.0));
    BOOST_CHECK_GE(Second(1.0), Second(0.5));
    BOOST_CHECK_LE(Second(1.0), Second(1.0));
    BOOST_CHECK_GE(Second(1.0), Second(1.0));
}

BOOST_AUTO_TEST_CASE(addition_works_correctly)
{
    Second lhs {1.0};
    Second rhs {0.5};
    lhs += rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 1.5, 0.0001);
    BOOST_CHECK_CLOSE((Second(1.0) + Second(0.5)).value(), 1.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(subtraction_works_correctly)
{
    Second lhs {1.0};
    Second rhs {0.5};
    lhs -= rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 0.5, 0.0001);
    BOOST_CHECK_CLOSE((Second(1.0) - Second(0.5)).value(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(multiplication_works_correctly)
{
    Second lhs {1.0};
    double rhs {0.5};
    lhs *= rhs;

    BOOST_CHECK_CLOSE(lhs.value(), 0.5, 0.0001);
    BOOST_CHECK_CLOSE((Second(1.0) * 0.5).value(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_CASE(division_works_correctly)
{
    BOOST_CHECK_CLOSE(Second(1.0) / Second(2.0), 0.5, 0.0001);
    BOOST_CHECK_CLOSE(Second(2.0) / Second(1.0), 2.0, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()
