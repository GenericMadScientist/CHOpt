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

#include <array>

#include <boost/test/unit_test.hpp>

#include "test_helpers.hpp"
#include "timeconverter.hpp"

BOOST_AUTO_TEST_CASE(beats_to_seconds_conversion_works_correctly)
{
    TempoMap tempo_map {{{0, 4, 4}}, {{0, 150000}, {800, 200000}}, 192};
    TimeConverter converter {tempo_map, 200, ChGuitarEngine(), {}};
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.0};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 1.9};

    for (auto i = 0U; i < beats.size(); ++i) {
        BOOST_CHECK_CLOSE(converter.beats_to_seconds(Beat(beats.at(i))).value(),
                          seconds.at(i), 0.0001);
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        BOOST_CHECK_CLOSE(
            converter.seconds_to_beats(Second(seconds.at(i))).value(),
            beats.at(i), 0.0001);
    }
}

BOOST_AUTO_TEST_CASE(beats_to_measures_conversion_works_correctly)
{
    TempoMap tempo_map {{{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}}, {}, 192};
    TimeConverter converter {tempo_map, 200, ChGuitarEngine(), {}};
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.5, 6.5};
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};

    for (auto i = 0U; i < beats.size(); ++i) {
        BOOST_CHECK_CLOSE(
            converter.beats_to_measures(Beat(beats.at(i))).value(),
            measures.at(i), 0.0001);
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        BOOST_CHECK_CLOSE(
            converter.measures_to_beats(Measure(measures.at(i))).value(),
            beats.at(i), 0.0001);
    }
}

BOOST_AUTO_TEST_CASE(measures_to_seconds_conversion_works_correctly)
{
    TempoMap tempo_map {{{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}},
                        {{0, 150000}, {800, 200000}},
                        192};
    TimeConverter converter {tempo_map, 200, ChGuitarEngine(), {}};
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 2.05, 2.35};

    for (auto i = 0U; i < measures.size(); ++i) {
        BOOST_CHECK_CLOSE(
            converter.measures_to_seconds(Measure(measures.at(i))).value(),
            seconds.at(i), 0.0001);
    }

    for (auto i = 0U; i < measures.size(); ++i) {
        BOOST_CHECK_CLOSE(
            converter.seconds_to_measures(Second(seconds.at(i))).value(),
            measures.at(i), 0.0001);
    }
}
