/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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

#include "tempomap.hpp"
#include "test_helpers.hpp"

BOOST_AUTO_TEST_SUITE(sync_track_ctor_maintains_invariants)

BOOST_AUTO_TEST_CASE(bpms_are_sorted_by_position)
{
    TempoMap tempo_map {{}, {{0, 150000}, {2000, 200000}, {1000, 225000}}};
    std::vector<BPM> expected_bpms {
        {0, 150000}, {1000, 225000}, {2000, 200000}};

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.bpms().cbegin(),
                                  tempo_map.bpms().cend(),
                                  expected_bpms.cbegin(), expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(no_two_bpms_have_the_same_position)
{
    TempoMap tempo_map {{}, {{0, 150000}, {0, 200000}}};
    std::vector<BPM> expected_bpms {{0, 200000}};

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.bpms().cbegin(),
                                  tempo_map.bpms().cend(),
                                  expected_bpms.cbegin(), expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(bpms_is_never_empty)
{
    TempoMap tempo_map;
    std::vector<BPM> expected_bpms {{0, 120000}};

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.bpms().cbegin(),
                                  tempo_map.bpms().cend(),
                                  expected_bpms.cbegin(), expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(time_signatures_are_sorted_by_position)
{
    TempoMap tempo_map {{{0, 4, 4}, {2000, 3, 3}, {1000, 2, 2}}, {}};
    std::vector<TimeSignature> expected_tses {
        {0, 4, 4}, {1000, 2, 2}, {2000, 3, 3}};

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.time_sigs().cbegin(),
                                  tempo_map.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(no_two_time_signatures_have_the_same_position)
{
    TempoMap tempo_map {{{0, 4, 4}, {0, 3, 4}}, {}};
    std::vector<TimeSignature> expected_tses {{0, 3, 4}};

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.time_sigs().cbegin(),
                                  tempo_map.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(time_sigs_is_never_empty)
{
    TempoMap tempo_map;
    std::vector<TimeSignature> expected_tses {{0, 4, 4}};

    BOOST_CHECK_EQUAL_COLLECTIONS(tempo_map.time_sigs().cbegin(),
                                  tempo_map.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(bpms_must_not_be_zero_or_negative)
{
    BOOST_CHECK_THROW(([&] {
                          return TempoMap {{}, {{192, 0}}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return TempoMap {{}, {{192, -1}}};
                      })(),
                      ParseError);
}

BOOST_AUTO_TEST_CASE(time_signatures_must_be_positive_positive)
{
    BOOST_CHECK_THROW(([&] {
                          return TempoMap {{{0, 0, 4}}, {}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return TempoMap {{{0, -1, 4}}, {}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return TempoMap {{{0, 4, 0}}, {}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return TempoMap {{{0, 4, -1}}, {}};
                      })(),
                      ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(speedup_returns_correct_tempo_map)
{
    const TempoMap tempo_map {{{0, 4, 4}}, {{0, 120000}, {192, 240000}}};
    const std::vector<BPM> expected_bpms {{0, 180000}, {192, 360000}};
    const std::vector<TimeSignature> expected_tses {{0, 4, 4}};

    const auto speedup = tempo_map.speedup(150);

    BOOST_CHECK_EQUAL_COLLECTIONS(speedup.bpms().cbegin(),
                                  speedup.bpms().cend(), expected_bpms.cbegin(),
                                  expected_bpms.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(speedup.time_sigs().cbegin(),
                                  speedup.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(speedup_doesnt_overflow)
{
    const TempoMap tempo_map {{}, {{0, 200000000}}};
    const std::vector<BPM> expected_bpms {{0, 400000000}};

    const auto speedup = tempo_map.speedup(200);

    BOOST_CHECK_EQUAL_COLLECTIONS(speedup.bpms().cbegin(),
                                  speedup.bpms().cend(), expected_bpms.cbegin(),
                                  expected_bpms.cend());
}
