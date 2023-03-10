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

#include <algorithm>
#include <cstdlib>
#include <numeric>

#include <boost/test/unit_test.hpp>

#include "test_helpers.hpp"

namespace {
std::vector<int> set_values(const PointSet& points)
{
    std::vector<int> values;
    values.reserve(static_cast<std::size_t>(
        std::distance(points.cbegin(), points.cend())));
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        values.push_back(p->value);
    }
    return values;
}

std::vector<int> set_base_values(const PointSet& points)
{
    std::vector<int> base_values;
    base_values.reserve(static_cast<std::size_t>(
        std::distance(points.cbegin(), points.cend())));
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        base_values.push_back(p->base_value);
    }
    return base_values;
}

std::vector<Beat> set_position_beats(const PointSet& points)
{
    std::vector<Beat> values;
    values.reserve(static_cast<std::size_t>(
        std::distance(points.cbegin(), points.cend())));
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        values.push_back(p->position.beat);
    }
    return values;
}
}

BOOST_AUTO_TEST_SUITE(non_sustain_notes)

BOOST_AUTO_TEST_CASE(single_notes_give_fifty_points)
{
    NoteTrack<NoteColour> track {{{768}, {960}}, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    std::vector<int> expected_values {50, 50};
    std::vector<int> values = set_values(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
}

BOOST_AUTO_TEST_CASE(dhords_give_multiples_of_fifty_points)
{
    NoteTrack<NoteColour> track {
        {{768, 0, NoteColour::Green}, {768, 0, NoteColour::Red}},
        {},
        {},
        {},
        {},
        {},
        192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    std::vector<int> expected_values {100};
    std::vector<int> values = set_values(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
}

BOOST_AUTO_TEST_CASE(ghl_notes_behave_the_same_as_five_fret_notes)
{
    NoteTrack<GHLNoteColour> track {{{768}, {960}}, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    std::vector<int> expected_values {50, 50};
    std::vector<int> values = set_values(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(sustain_notes)

BOOST_AUTO_TEST_CASE(sustain_points_depend_on_resolution)
{
    NoteTrack<NoteColour> track {{{768, 15}}, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet first_points {track,
                           converter,
                           {},
                           SqueezeSettings::default_settings(),
                           DrumSettings::default_settings(),
                           ChGuitarEngine()};
    std::vector<int> first_expected_values {50, 3};
    std::vector<Beat> first_expected_beats {Beat(4.0), Beat(4.0026)};
    NoteTrack<NoteColour> second_track {{{768, 15}}, {}, {}, {}, {}, {}, 200};
    TimeConverter second_converter {{}, 200, ChGuitarEngine(), {}};
    PointSet second_points {second_track,
                            second_converter,
                            {},
                            SqueezeSettings::default_settings(),
                            DrumSettings::default_settings(),
                            ChGuitarEngine()};
    std::vector<int> second_expected_values {50, 2};
    std::vector<Beat> second_expected_beats {Beat(3.84), Beat(3.8425)};

    std::vector<int> first_values = set_values(first_points);
    std::vector<Beat> first_beats = set_position_beats(first_points);
    std::vector<int> second_values = set_values(second_points);
    std::vector<Beat> second_beats = set_position_beats(second_points);

    BOOST_CHECK_EQUAL_COLLECTIONS(first_values.cbegin(), first_values.cend(),
                                  first_expected_values.cbegin(),
                                  first_expected_values.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(first_beats.cbegin(), first_beats.cend(),
                                  first_expected_beats.cbegin(),
                                  first_expected_beats.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(second_values.cbegin(), second_values.cend(),
                                  second_expected_values.cbegin(),
                                  second_expected_values.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(second_beats.cbegin(), second_beats.cend(),
                                  second_expected_beats.cbegin(),
                                  second_expected_beats.cend());
}

BOOST_AUTO_TEST_CASE(sustain_points_and_chords)
{
    NoteTrack<NoteColour> track {
        {{768, 8, NoteColour::Green}, {768, 8, NoteColour::Red}},
        {},
        {},
        {},
        {},
        {},
        192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    std::vector<int> expected_values {100, 2};
    std::vector<Beat> expected_beats {Beat(4.0), Beat(4.0026)};
    std::vector<int> values = set_values(points);
    std::vector<Beat> beats = set_position_beats(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(beats.cbegin(), beats.cend(),
                                  expected_beats.cbegin(),
                                  expected_beats.cend());
}

BOOST_AUTO_TEST_CASE(resolutions_below_25_do_not_enter_an_infinite_loop)
{
    NoteTrack<NoteColour> track {{{768, 2}}, {}, {}, {}, {}, {}, 1};
    TimeConverter converter {{}, 1, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(std::distance(points.cbegin(), points.cend()), 3);
}

BOOST_AUTO_TEST_CASE(sustains_of_uneven_length_are_handled_correctly)
{
    NoteTrack<NoteColour> track {{{0, 1504, NoteColour::Green},
                                  {0, 1504, NoteColour::Red},
                                  {0, 736, NoteColour::Yellow}},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {},
                                 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 686);
}

// RB1 Here It Goes Again m20 GRB sustain
BOOST_AUTO_TEST_CASE(chord_sustains_in_rb_are_handled_correctly)
{
    NoteTrack<NoteColour> track {{{0, 1800, NoteColour::Green},
                                  {0, 1800, NoteColour::Red},
                                  {0, 1800, NoteColour::Blue}},
                                 {},
                                 {},
                                 {},
                                 {},
                                 {},
                                 480};
    TimeConverter converter {{}, 480, RbEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     RbEngine()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 210);
}

// RB1 Cherub Rock m24 Y sustain
BOOST_AUTO_TEST_CASE(
    rounding_from_length_in_rb_for_single_notes_is_handled_correctly)
{
    NoteTrack<NoteColour> track {{{0, 419}}, {}, {}, {}, {}, {}, 480};
    TimeConverter converter {{}, 480, RbEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     RbEngine()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 35);
}

// RB1 Cherub Rock m65 RO sustain
BOOST_AUTO_TEST_CASE(rounding_from_length_in_rb_for_chords_is_handled_correctly)
{
    NoteTrack<NoteColour> track {
        {{0, 419, NoteColour::Red}, {0, 419, NoteColour::Orange}},
        {},
        {},
        {},
        {},
        {},
        480};
    TimeConverter converter {{}, 480, RbEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     RbEngine()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 70);
}

// GH1 Ace of Spades m6 Y sustain
BOOST_AUTO_TEST_CASE(gh1_one_beat_sustain_is_handled_correctly)
{
    NoteTrack<NoteColour> track {
        {{0, 1917, NoteColour::Yellow}}, {}, {}, {}, {}, {}, 480};
    TimeConverter converter {{}, 480, Gh1Engine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 150);
}

// GH1 Ace of Spades m96 O sustain
BOOST_AUTO_TEST_CASE(
    rounding_from_fractional_length_in_gh1_is_handled_correctly)
{
    NoteTrack<NoteColour> track {
        {{0, 1560, NoteColour::Orange}}, {}, {}, {}, {}, {}, 480};
    TimeConverter converter {{}, 480, Gh1Engine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 131);
}

// GH1 Ace of Spades m15 BO sustain
BOOST_AUTO_TEST_CASE(
    rounding_from_length_in_gh1_for_chords_is_handled_correctly)
{
    NoteTrack<NoteColour> track {
        {{0, 360, NoteColour::Blue}, {0, 360, NoteColour::Orange}},
        {},
        {},
        {},
        {},
        {},
        480};
    TimeConverter converter {{}, 480, Gh1Engine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 138);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(points_are_sorted)
{
    NoteTrack<NoteColour> track {
        {{768, 15}, {770, 0}}, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    const auto beats = set_position_beats(points);

    BOOST_TEST(std::is_sorted(beats.cbegin(), beats.cend()));
}

BOOST_AUTO_TEST_CASE(end_of_sp_phrase_points)
{
    NoteTrack<NoteColour> track {{{768}, {960}, {1152}},
                                 {{768, 1}, {900, 50}, {1100, 53}},
                                 {},
                                 {},
                                 {},
                                 {},
                                 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    PointSet unison_points {track,
                            converter,
                            {{1100, 53}},
                            SqueezeSettings::default_settings(),
                            DrumSettings::default_settings(),
                            Rb3Engine()};

    BOOST_TEST(points.cbegin()->is_sp_granting_note);
    BOOST_TEST(!std::next(points.cbegin())->is_sp_granting_note);
    BOOST_TEST(std::next(points.cbegin(), 2)->is_sp_granting_note);
    BOOST_TEST(!std::next(points.cbegin(), 2)->is_unison_sp_granting_note);

    BOOST_TEST(!unison_points.cbegin()->is_unison_sp_granting_note);
    BOOST_TEST(
        std::next(unison_points.cbegin(), 2)->is_unison_sp_granting_note);
}

BOOST_AUTO_TEST_SUITE(combo_multiplier_is_taken_into_account)

BOOST_AUTO_TEST_CASE(multiplier_applies_to_non_sustains)
{
    std::vector<Note<NoteColour>> notes;
    notes.reserve(50);
    for (int i = 0; i < 50; ++i) {
        notes.push_back({192 * i});
    }
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    std::vector<int> expected_values;
    std::vector<int> expected_base_values;
    expected_values.reserve(50);
    expected_base_values.reserve(50);
    for (int i = 0; i < 50; ++i) {
        const auto mult = 1 + std::min((i + 1) / 10, 3);
        expected_values.push_back(50 * mult);
        expected_base_values.push_back(50);
    }
    std::vector<int> values = set_values(points);
    std::vector<int> base_values = set_base_values(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(base_values.cbegin(), base_values.cend(),
                                  expected_base_values.cbegin(),
                                  expected_base_values.cend());
}

BOOST_AUTO_TEST_CASE(sustain_points_are_multiplied)
{
    std::vector<Note<NoteColour>> notes;
    notes.reserve(50);
    for (int i = 0; i < 50; ++i) {
        notes.push_back({192 * i});
    }
    notes.push_back({9600, 192});

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 4);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->base_value, 1);
}

BOOST_AUTO_TEST_CASE(later_sustain_points_in_extended_sustains_are_multiplied)
{
    std::vector<Note<NoteColour>> notes;
    notes.reserve(10);
    for (int i = 0; i < 10; ++i) {
        notes.push_back({192 * i});
    }
    notes[0].length = 2000;

    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 2);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->base_value, 1);
}

BOOST_AUTO_TEST_CASE(drum_notes_have_the_multiplier_handled_correctly)
{
    std::vector<Note<DrumNoteColour>> notes;
    notes.reserve(10);
    for (int i = 0; i < 9; ++i) {
        notes.push_back({192 * i, 0, DrumNoteColour::Red});
    }
    notes.push_back({192 * 7, 0, DrumNoteColour::Yellow});

    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChDrumEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 1)->value, 100);
}

BOOST_AUTO_TEST_CASE(gh1_multiplier_delay_accounted_for)
{
    std::vector<Note<NoteColour>> notes {{0},   {100},     {200}, {300},
                                         {400}, {500},     {600}, {700},
                                         {800}, {900, 100}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, Gh1Engine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};

    BOOST_CHECK_EQUAL((points.cbegin() + 9)->value, 50);
    BOOST_CHECK_EQUAL((points.cbegin() + 10)->value, 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(hit_window_start_and_hit_window_end_are_set_correctly)

BOOST_AUTO_TEST_CASE(hit_window_starts_for_notes_are_correct)
{
    TimeConverter converter {
        {{}, {{0, 150000}, {768, 200000}}}, 192, ChGuitarEngine(), {}};
    std::vector<Note<NoteColour>> notes {{192}, {787}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_start.beat.value(), 0.825,
                      0.0001);
    BOOST_CHECK_CLOSE(std::next(points.cbegin())->hit_window_start.beat.value(),
                      3.89922, 0.0001);
}

BOOST_AUTO_TEST_CASE(hit_window_ends_for_notes_are_correct)
{
    TimeConverter converter {
        {{}, {{0, 150000}, {768, 200000}}}, 192, ChGuitarEngine(), {}};
    std::vector<Note<NoteColour>> notes {{192}, {749}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.175,
                      0.0001);
    BOOST_CHECK_CLOSE(std::next(points.cbegin())->hit_window_end.beat.value(),
                      4.10139, 0.0001);
}

BOOST_AUTO_TEST_CASE(hit_window_starts_and_ends_for_hold_points_are_correct)
{
    TimeConverter converter {
        {{}, {{0, 150000}, {768, 200000}}}, 192, ChGuitarEngine(), {}};
    std::vector<Note<NoteColour>> notes {{672, 192}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    for (auto p = std::next(points.cbegin()); p < points.cend(); ++p) {
        BOOST_CHECK_EQUAL(p->position.beat, p->hit_window_start.beat);
        BOOST_CHECK_EQUAL(p->position.beat, p->hit_window_end.beat);
    }
}

BOOST_AUTO_TEST_CASE(squeeze_setting_is_accounted_for)
{
    TimeConverter converter {
        {{}, {{0, 150000}, {768, 200000}}}, 192, ChGuitarEngine(), {}};
    std::vector<Note<NoteColour>> notes {{192}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     converter,
                     {},
                     {0.5, 1.0, Second {0.0}, Second {0.0}, Second {0.0}},
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_start.beat.value(), 0.9125,
                      0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.0875,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(restricted_back_end_is_taken_account_of)
{
    TimeConverter converter {
        {{}, {{0, 150000}, {768, 200000}}}, 192, ChGuitarEngine(), {}};
    std::vector<Note<NoteColour>> notes {{192}, {240}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     RbEngine()};
    PointSet fifty_sqz_points {
        track,
        converter,
        {},
        {0.5, 1.0, Second {0.0}, Second {0.0}, Second {0.0}},
        DrumSettings::default_settings(),
        RbEngine()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.125,
                      0.0001);
    BOOST_CHECK_CLOSE(fifty_sqz_points.cbegin()->hit_window_end.beat.value(),
                      1.0625, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(rb_bass_multiplier_is_taken_into_account)
{
    std::vector<Note<NoteColour>> notes;
    notes.reserve(60);
    for (auto i = 0; i < 60; ++i) {
        notes.push_back({192 * i});
    }
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    const TimeConverter converter {{}, 192, RbBassEngine(), {}};
    const PointSet points {track,
                           converter,
                           {},
                           SqueezeSettings::default_settings(),
                           DrumSettings::default_settings(),
                           RbBassEngine()};

    const auto total_points
        = std::accumulate(points.cbegin(), points.cend(), 0,
                          [&](auto x, auto y) { return x + y.value; });

    BOOST_CHECK_EQUAL(total_points, 5375);
}

BOOST_AUTO_TEST_SUITE(video_lag_is_taken_into_account)

BOOST_AUTO_TEST_CASE(negative_video_lag_is_handled_correctly)
{
    const std::vector<Note<NoteColour>> notes {{192, 0}, {384, 192}};
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    const TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     {1.0, 1.0, Second {0.0}, Second {-0.20}, Second {0.0}},
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_CLOSE(points.cbegin()->position.beat.value(), 0.6, 0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_start.beat.value(), 0.46,
                      0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 0.74,
                      0.0001);
    BOOST_CHECK_CLOSE(std::next(points.cbegin(), 2)->position.beat.value(),
                      2.033854, 0.0001);
}

BOOST_AUTO_TEST_CASE(positive_video_lag_is_handled_correctly)
{
    const std::vector<Note<NoteColour>> notes {{192, 0}, {384, 192}};
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    const TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     {1.0, 1.0, Second {0.0}, Second {0.20}, Second {0.0}},
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_CLOSE(points.cbegin()->position.beat.value(), 1.4, 0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_start.beat.value(), 1.26,
                      0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.54,
                      0.0001);
    BOOST_CHECK_CLOSE(std::next(points.cbegin(), 2)->position.beat.value(),
                      2.033854, 0.0001);
}

BOOST_AUTO_TEST_CASE(tick_points_are_not_multiplied_prematurely)
{
    std::vector<Note<NoteColour>> notes {{192},      {193}, {194}, {195},
                                         {196},      {197}, {198}, {199},
                                         {200, 200}, {400}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    const TimeConverter converter {{}, 192, ChGuitarEngine(), {}};
    PointSet points {track,
                     converter,
                     {},
                     {1.0, 1.0, Second {0.0}, Second {-0.40}, Second {0.0}},
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(std::prev(points.cend())->value, 100);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 7);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(next_non_hold_point_is_correct)
{
    std::vector<Note<NoteColour>> notes {{0}, {192, 192}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

    PointSet points {track,
                     {{}, 192, ChGuitarEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(points.next_non_hold_point(points.cbegin()),
                      points.cbegin());
    BOOST_CHECK_EQUAL(points.next_non_hold_point(std::next(points.cbegin(), 2)),
                      points.cend());
}

BOOST_AUTO_TEST_CASE(next_sp_granting_note_is_correct)
{
    std::vector<Note<NoteColour>> notes {{100, 0}, {200, 100}, {400, 0}};
    std::vector<StarPower> phrases {{200, 1}, {400, 1}};
    NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    TimeConverter converter {{}, 192, ChGuitarEngine(), {}};

    PointSet points {track,
                     converter,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(points.next_sp_granting_note(points.cbegin()),
                      std::next(points.cbegin()));
    BOOST_CHECK_EQUAL(points.next_sp_granting_note(std::next(points.cbegin())),
                      std::next(points.cbegin()));
    BOOST_CHECK_EQUAL(
        points.next_sp_granting_note(std::next(points.cbegin(), 2)),
        std::prev(points.cend()));
}

BOOST_AUTO_TEST_CASE(solo_sections_are_added)
{
    std::vector<Solo> solos {{0, 576, 100}, {768, 1152, 200}};
    NoteTrack<NoteColour> track {{}, {}, solos, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChGuitarEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    std::vector<std::tuple<Position, int>> expected_solo_boosts {
        {{Beat(3.0), Measure(0.75)}, 100}, {{Beat(6.0), Measure(1.5)}, 200}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        points.solo_boosts().cbegin(), points.solo_boosts().cend(),
        expected_solo_boosts.cbegin(), expected_solo_boosts.cend());
}

BOOST_AUTO_TEST_CASE(range_score_is_correct)
{
    NoteTrack<NoteColour> track {{{0, 192}, {386}}, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChGuitarEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.range_score(begin, begin), 0);
    BOOST_CHECK_EQUAL(points.range_score(begin, end), 128);
    BOOST_CHECK_EQUAL(points.range_score(begin + 1, end - 1), 28);
}

BOOST_AUTO_TEST_CASE(colour_set_is_correct_for_five_fret)
{
    std::vector<Note<NoteColour>> notes {{0},
                                         {0, 0, NoteColour::Red},
                                         {176, 100, NoteColour::Yellow},
                                         {500, 0, NoteColour::Blue}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChGuitarEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.colour_set(begin), "GR");
    BOOST_CHECK_EQUAL(points.colour_set(begin + 1), "Y");
    BOOST_CHECK_EQUAL(points.colour_set(end - 1), "B");
}

BOOST_AUTO_TEST_CASE(colour_set_is_correct_for_six_fret)
{
    std::vector<Note<GHLNoteColour>> notes {
        {0},
        {0, 0, GHLNoteColour::WhiteMid},
        {176, 100, GHLNoteColour::BlackHigh},
        {500, 0, GHLNoteColour::Open}};
    NoteTrack<GHLNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChGuitarEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.colour_set(begin), "W1W2");
    BOOST_CHECK_EQUAL(points.colour_set(begin + 1), "B3");
    BOOST_CHECK_EQUAL(points.colour_set(end - 1), "open");
}

BOOST_AUTO_TEST_CASE(colour_set_is_correct_for_drums)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0},
        {0, 0, DrumNoteColour::YellowCymbal},
        {176, 0, DrumNoteColour::BlueCymbal},
        {500, 0, DrumNoteColour::Kick}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.colour_set(begin), "R");
    BOOST_CHECK_EQUAL(points.colour_set(begin + 1), "Y cymbal");
    BOOST_CHECK_EQUAL(points.colour_set(end - 1), "kick");
}

BOOST_AUTO_TEST_CASE(double_kicks_only_appear_with_enable_double_kick)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Kick}, {192, 0, DrumNoteColour::DoubleKick}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet single_points {track,
                            {{}, 192, ChDrumEngine(), {}},
                            {},
                            SqueezeSettings::default_settings(),
                            {false, false, true, false},
                            ChDrumEngine()};
    PointSet double_points {track,
                            {{}, 192, ChDrumEngine(), {}},
                            {},
                            SqueezeSettings::default_settings(),
                            DrumSettings::default_settings(),
                            ChDrumEngine()};

    BOOST_CHECK_EQUAL(single_points.cend() - single_points.cbegin(), 1);
    BOOST_CHECK_EQUAL(double_points.cend() - double_points.cbegin(), 2);
}

BOOST_AUTO_TEST_CASE(single_kicks_are_removed_with_disable_kick)
{
    std::vector<Note<DrumNoteColour>> notes {{0, 0, DrumNoteColour::DoubleKick},
                                             {192, 0, DrumNoteColour::Kick}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     {true, true, false, false},
                     ChDrumEngine()};

    BOOST_CHECK_EQUAL(points.cend() - points.cbegin(), 1);
    BOOST_CHECK_EQUAL(points.cbegin()->value, 50);
}

BOOST_AUTO_TEST_CASE(disable_kick_doesnt_kill_sp_phrases)
{
    std::vector<Note<DrumNoteColour>> notes {{0, 0, DrumNoteColour::Red},
                                             {192, 0, DrumNoteColour::Kick}};
    std::vector<StarPower> phrases {{0, 200}};
    NoteTrack<DrumNoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     {false, true, false, false},
                     ChDrumEngine()};

    BOOST_TEST(points.cbegin()->is_sp_granting_note);
}

BOOST_AUTO_TEST_CASE(double_kicks_dont_kill_phrases)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red}, {192, 0, DrumNoteColour::DoubleKick}};
    std::vector<StarPower> phrases {{0, 200}};
    NoteTrack<DrumNoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     {false, false, false, false},
                     ChDrumEngine()};

    BOOST_TEST(points.cbegin()->is_sp_granting_note);
}

BOOST_AUTO_TEST_CASE(activation_notes_are_marked_with_drum_fills)
{
    std::vector<Note<DrumNoteColour>> notes {{0}, {192}, {385}, {576}};
    std::vector<DrumFill> fills {{384, 5}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, fills, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST(!(begin + 1)->fill_start.has_value());
    BOOST_CHECK_CLOSE((begin + 2)->fill_start->value(), 1.0, 0.0001);
    BOOST_TEST(!(begin + 3)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_ending_only_in_a_kick_are_killed)
{
    std::vector<Note<DrumNoteColour>> notes {{0}, {1, 0, DrumNoteColour::Kick}};
    std::vector<DrumFill> fills {{0, 2}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, fills, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     {false, false, false, false},
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST(!(begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_ending_only_in_a_double_kick_are_killed)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0}, {1, 0, DrumNoteColour::DoubleKick}};
    std::vector<DrumFill> fills {{0, 2}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, fills, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST(!(begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(
    fills_ending_in_a_multi_note_have_the_activation_attached_to_the_first_note)
{
    std::vector<Note<DrumNoteColour>> notes {{0}, {0, 0, DrumNoteColour::Kick}};
    std::vector<DrumFill> fills {{0, 2}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, fills, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_CLOSE(begin->fill_start->value(), 0.0, 0.0001);
    BOOST_TEST(!(begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_are_attached_to_the_nearest_ending_point)
{
    std::vector<Note<DrumNoteColour>> notes {{0}, {192}, {370}, {384}};
    std::vector<DrumFill> fills {{0, 2}, {193, 5}, {377, 4}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, fills, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_TEST(begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
    BOOST_TEST(!(begin + 2)->fill_start.has_value());
    BOOST_TEST((begin + 3)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_attach_to_later_point_in_case_of_a_tie)
{
    std::vector<Note<DrumNoteColour>> notes {{0}, {192}};
    std::vector<DrumFill> fills {{0, 96}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, fills, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(cymbals_get_extra_points)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::YellowCymbal}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(begin->value, 65);
}

BOOST_AUTO_TEST_CASE(dynamics_get_double_points)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::RedGhost},
        {192, 0, DrumNoteColour::RedAccent},
        {384, 0, DrumNoteColour::Red},
        {576, 0, DrumNoteColour::YellowCymbalGhost},
        {768, 0, DrumNoteColour::YellowCymbalAccent},
        {960, 0, DrumNoteColour::YellowCymbal}};
    NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChDrumEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(begin->value, 100);
    BOOST_CHECK_EQUAL((begin + 1)->value, 100);
    BOOST_CHECK_EQUAL((begin + 2)->value, 50);
    BOOST_CHECK_EQUAL((begin + 3)->value, 130);
    BOOST_CHECK_EQUAL((begin + 4)->value, 130);
    BOOST_CHECK_EQUAL((begin + 5)->value, 65);
}

BOOST_AUTO_TEST_SUITE(first_after_current_phrase_works_correctly)

BOOST_AUTO_TEST_CASE(returns_next_point_outside_of_sp)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, Gh1Engine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin),
                      std::next(begin));
}

BOOST_AUTO_TEST_CASE(returns_next_point_outside_current_sp_for_overlap_engine)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}};
    std::vector<StarPower> phrases {{0, 200}};
    NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, Gh1Engine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin), begin + 2);
}

BOOST_AUTO_TEST_CASE(returns_next_point_always_next_for_non_overlap_engine)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}};
    std::vector<StarPower> phrases {{0, 200}};
    NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    PointSet points {track,
                     {{}, 192, ChGuitarEngine(), {}},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin), begin + 1);
}

BOOST_AUTO_TEST_SUITE_END()
