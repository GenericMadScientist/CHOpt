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

#include "chartparser.hpp"
#include "song.hpp"
#include "test_helpers.hpp"

BOOST_AUTO_TEST_CASE(unison_phrase_positions_is_correct)
{
    ChartSection guitar {"ExpertSingle",
                         {},
                         {},
                         {},
                         {{768, 0, 0}, {1024, 0, 0}},
                         {{768, 2, 100}, {1024, 2, 100}},
                         {}};
    // Note the first phrase has a different length than the other instruments.
    // It should still be a unison phrase: this happens in Roundabout, with the
    // key phrases being a slightly different length.
    ChartSection bass {"ExpertDoubleBass",
                       {},
                       {},
                       {},
                       {{768, 0, 0}, {2048, 0, 0}},
                       {{768, 2, 99}, {2048, 2, 100}},
                       {}};
    // The 768 phrase is absent for drums: this is to test that unison bonuses
    // can apply when at least 2 instruments have the phrase. This happens with
    // the first phrase on RB3 Last Dance guitar, the phrase is missing on bass.
    ChartSection drums {
        "ExpertDrums",    {}, {}, {}, {{768, 0, 0}, {4096, 0, 0}},
        {{4096, 2, 100}}, {}};
    std::vector<ChartSection> sections {guitar, bass, drums};
    const Chart chart {sections};
    const auto song = ChartParser({}).parse(chart);

    const std::vector<Tick> expected_unison_phrases {Tick {768}};
    const std::vector<Tick> unison_phrases = song.unison_phrase_positions();

    BOOST_CHECK_EQUAL_COLLECTIONS(
        unison_phrases.cbegin(), unison_phrases.cend(),
        expected_unison_phrases.cbegin(), expected_unison_phrases.cend());
}

BOOST_AUTO_TEST_SUITE(speedup)

BOOST_AUTO_TEST_CASE(song_name_is_updated)
{
    Song song;
    song.global_data().name("TestName");

    song.speedup(200);

    BOOST_CHECK_EQUAL(song.global_data().name(), "TestName (200%)");
}

BOOST_AUTO_TEST_CASE(song_name_is_unaffected_by_normal_speed)
{
    Song song;
    song.global_data().name("TestName");

    song.speedup(100);

    BOOST_CHECK_EQUAL(song.global_data().name(), "TestName");
}

BOOST_AUTO_TEST_CASE(tempo_map_affected_by_speedup)
{
    Song song;

    song.speedup(200);
    const auto& tempo_map = song.global_data().tempo_map();

    BOOST_CHECK_EQUAL(tempo_map.bpms().front().bpm, 240000);
}

BOOST_AUTO_TEST_CASE(throws_on_negative_speeds)
{
    Song song;

    BOOST_CHECK_THROW([&] { song.speedup(-100); }(), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(throws_on_zero_speed)
{
    Song song;

    BOOST_CHECK_THROW([&] { song.speedup(0); }(), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
