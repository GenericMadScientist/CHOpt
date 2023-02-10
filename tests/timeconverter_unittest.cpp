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

BOOST_AUTO_TEST_SUITE(sync_track_ctor_maintains_invariants)

BOOST_AUTO_TEST_CASE(bpms_are_sorted_by_position)
{
    SyncTrack track {{}, {{0, 150000}, {2000, 200000}, {1000, 225000}}};
    std::vector<BPM> expected_bpms {
        {0, 150000}, {1000, 225000}, {2000, 200000}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.bpms().cbegin(), track.bpms().cend(),
                                  expected_bpms.cbegin(), expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(no_two_bpms_have_the_same_position)
{
    SyncTrack track {{}, {{0, 150000}, {0, 200000}}};
    std::vector<BPM> expected_bpms {{0, 200000}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.bpms().cbegin(), track.bpms().cend(),
                                  expected_bpms.cbegin(), expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(bpms_is_never_empty)
{
    SyncTrack track;
    std::vector<BPM> expected_bpms {{0, 120000}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.bpms().cbegin(), track.bpms().cend(),
                                  expected_bpms.cbegin(), expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(time_signatures_are_sorted_by_position)
{
    SyncTrack track {{{0, 4, 4}, {2000, 3, 3}, {1000, 2, 2}}, {}};
    std::vector<TimeSignature> expected_tses {
        {0, 4, 4}, {1000, 2, 2}, {2000, 3, 3}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.time_sigs().cbegin(),
                                  track.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(no_two_time_signatures_have_the_same_position)
{
    SyncTrack track {{{0, 4, 4}, {0, 3, 4}}, {}};
    std::vector<TimeSignature> expected_tses {{0, 3, 4}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.time_sigs().cbegin(),
                                  track.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(time_sigs_is_never_empty)
{
    SyncTrack track;
    std::vector<TimeSignature> expected_tses {{0, 4, 4}};

    BOOST_CHECK_EQUAL_COLLECTIONS(track.time_sigs().cbegin(),
                                  track.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(bpms_must_not_be_zero_or_negative)
{
    BOOST_CHECK_THROW(([&] {
                          return SyncTrack {{}, {{192, 0}}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return SyncTrack {{}, {{192, -1}}};
                      })(),
                      ParseError);
}

BOOST_AUTO_TEST_CASE(time_signatures_must_be_positive_positive)
{
    BOOST_CHECK_THROW(([&] {
                          return SyncTrack {{{0, 0, 4}}, {}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return SyncTrack {{{0, -1, 4}}, {}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return SyncTrack {{{0, 4, 0}}, {}};
                      })(),
                      ParseError);
    BOOST_CHECK_THROW(([&] {
                          return SyncTrack {{{0, 4, -1}}, {}};
                      })(),
                      ParseError);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(speedup_returns_correct_synctrack)
{
    const SyncTrack sync_track {{{0, 4, 4}}, {{0, 120000}, {192, 240000}}};
    const std::vector<BPM> expected_bpms {{0, 180000}, {192, 360000}};
    const std::vector<TimeSignature> expected_tses {{0, 4, 4}};

    const auto speedup = sync_track.speedup(150);

    BOOST_CHECK_EQUAL_COLLECTIONS(speedup.bpms().cbegin(),
                                  speedup.bpms().cend(), expected_bpms.cbegin(),
                                  expected_bpms.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(speedup.time_sigs().cbegin(),
                                  speedup.time_sigs().cend(),
                                  expected_tses.cbegin(), expected_tses.cend());
}

BOOST_AUTO_TEST_CASE(speedup_doesnt_overflow)
{
    const SyncTrack sync_track {{}, {{0, 200000000}}};
    const std::vector<BPM> expected_bpms {{0, 400000000}};

    const auto speedup = sync_track.speedup(200);

    BOOST_CHECK_EQUAL_COLLECTIONS(speedup.bpms().cbegin(),
                                  speedup.bpms().cend(), expected_bpms.cbegin(),
                                  expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(beats_to_seconds_conversion_works_correctly)
{
    SyncTrack track {{{0, 4, 4}}, {{0, 150000}, {800, 200000}}};
    TimeConverter converter {track, 200, ChGuitarEngine(), {}};
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
    SyncTrack track {{{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}}, {}};
    TimeConverter converter {track, 200, ChGuitarEngine(), {}};
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
    SyncTrack track {{{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}},
                     {{0, 150000}, {800, 200000}}};
    TimeConverter converter {track, 200, ChGuitarEngine(), {}};
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
