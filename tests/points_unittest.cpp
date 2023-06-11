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
    NoteTrack track {{make_note(768), make_note(960)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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

BOOST_AUTO_TEST_CASE(chords_give_multiples_of_fifty_points)
{
    NoteTrack track {
        {make_chord(768, {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}})},
        {},
        {},
        {},
        {},
        {},
        TrackType::FiveFret,
        std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {{make_ghl_note(768), make_ghl_note(960)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::SixFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {{make_note(768, 15)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet first_points {track,
                           {{}, SpMode::Measure},
                           {},
                           SqueezeSettings::default_settings(),
                           DrumSettings::default_settings(),
                           ChGuitarEngine()};
    std::vector<int> first_expected_values {50, 3};
    std::vector<Beat> first_expected_beats {Beat(4.0), Beat(4.0026)};
    NoteTrack second_track {
        {make_note(768, 15)}, {}, {}, {}, {}, {}, TrackType::FiveFret,
        make_resolution(200)};
    PointSet second_points {
        second_track,
        {make_resolution(200)->tempo_map(), SpMode::Measure},
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
    NoteTrack track {
        {make_chord(768, {{FIVE_FRET_GREEN, 8}, {FIVE_FRET_RED, 8}})},
        {},
        {},
        {},
        {},
        {},
        TrackType::FiveFret,
        std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {
        {make_note(768, 2)}, {}, {}, {}, {}, {}, TrackType::FiveFret,
        make_resolution(1)};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(std::distance(points.cbegin(), points.cend()), 3);
}

BOOST_AUTO_TEST_CASE(sustains_of_uneven_length_are_handled_correctly)
{
    NoteTrack track {{make_chord(0,
                                 {{FIVE_FRET_GREEN, 1504},
                                  {FIVE_FRET_RED, 1504},
                                  {FIVE_FRET_YELLOW, 736}})},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {{make_chord(0,
                                 {{FIVE_FRET_GREEN, 1800},
                                  {FIVE_FRET_RED, 1800},
                                  {FIVE_FRET_BLUE, 1800}})},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     make_resolution(480)};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {
        {make_note(0, 419)}, {}, {}, {}, {}, {}, TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {
        {make_chord(0, {{FIVE_FRET_RED, 419}, {FIVE_FRET_ORANGE, 419}})},
        {},
        {},
        {},
        {},
        {},
        TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {track,
                     {{}, SpMode::OdBeat},
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
    NoteTrack track {{make_note(0, 1917, FIVE_FRET_YELLOW)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     make_resolution(480)};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {{make_note(0, 1560, FIVE_FRET_ORANGE)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     make_resolution(480)};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {
        {make_chord(0, {{FIVE_FRET_BLUE, 360}, {FIVE_FRET_ORANGE, 360}})},
        {},
        {},
        {},
        {},
        {},
        TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    NoteTrack track {{make_note(768, 15), make_note(770, 0)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    const auto beats = set_position_beats(points);

    BOOST_TEST(std::is_sorted(beats.cbegin(), beats.cend()));
}

BOOST_AUTO_TEST_CASE(end_of_sp_phrase_points)
{
    NoteTrack track {{make_note(768), make_note(960), make_note(1152)},
                     {{Tick {768}, Tick {1}},
                      {Tick {900}, Tick {50}},
                      {Tick {1100}, Tick {53}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    PointSet unison_points {track,
                            {{}, SpMode::Measure},
                            {{Tick {1100}, Tick {53}}},
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
    std::vector<Note> notes;
    notes.reserve(50);
    for (int i = 0; i < 50; ++i) {
        notes.push_back(make_note(192 * i));
    }
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes;
    notes.reserve(50);
    for (int i = 0; i < 50; ++i) {
        notes.push_back(make_note(192 * i));
    }
    notes.push_back(make_note(9600, 192));

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 4);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->base_value, 1);
}

BOOST_AUTO_TEST_CASE(later_sustain_points_in_extended_sustains_are_multiplied)
{
    std::vector<Note> notes;
    notes.reserve(10);
    for (int i = 0; i < 10; ++i) {
        notes.push_back(make_note(192 * i));
    }
    notes[0].lengths[0] = Tick {2000};

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 2);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->base_value, 1);
}

BOOST_AUTO_TEST_CASE(drum_notes_have_the_multiplier_handled_correctly)
{
    std::vector<Note> notes;
    notes.reserve(10);
    for (int i = 0; i < 9; ++i) {
        notes.push_back(make_drum_note(192 * i, DRUM_RED));
    }
    notes.push_back(make_drum_note(192 * 7, DRUM_YELLOW));

    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 1)->value, 100);
}

BOOST_AUTO_TEST_CASE(gh1_multiplier_delay_accounted_for)
{
    std::vector<Note> notes {make_note(0),       make_note(100), make_note(200),
                             make_note(300),     make_note(400), make_note(500),
                             make_note(600),     make_note(700), make_note(800),
                             make_note(900, 100)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    TempoMap tempo_map {
        {}, {{Tick {0}, 150000}, {Tick {768}, 200000}}, {}, 192};
    auto global_data = std::make_shared<SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<Note> notes {make_note(192), make_note(787)};
    NoteTrack track {notes,      {}, {}, {}, {}, {}, TrackType::FiveFret,
                     global_data};
    PointSet points {track,
                     {tempo_map, SpMode::Measure},
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
    TempoMap tempo_map {
        {}, {{Tick {0}, 150000}, {Tick {768}, 200000}}, {}, 192};
    auto global_data = std::make_shared<SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<Note> notes {make_note(192), make_note(749)};
    NoteTrack track {notes,      {}, {}, {}, {}, {}, TrackType::FiveFret,
                     global_data};
    PointSet points {track,
                     {tempo_map, SpMode::Measure},
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
    TempoMap tempo_map {
        {}, {{Tick {0}, 150000}, {Tick {768}, 200000}}, {}, 192};
    auto global_data = std::make_shared<SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<Note> notes {make_note(672, 192)};
    NoteTrack track {notes,      {}, {}, {}, {}, {}, TrackType::FiveFret,
                     global_data};
    PointSet points {track,
                     {tempo_map, SpMode::Measure},
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
    TempoMap tempo_map {
        {}, {{Tick {0}, 150000}, {Tick {768}, 200000}}, {}, 192};
    auto global_data = std::make_shared<SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<Note> notes {make_note(192)};
    NoteTrack track {notes,      {}, {}, {}, {}, {}, TrackType::FiveFret,
                     global_data};
    PointSet points {track,
                     {tempo_map, SpMode::Measure},
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
    TempoMap tempo_map {
        {}, {{Tick {0}, 150000}, {Tick {768}, 200000}}, {}, 192};
    auto global_data = std::make_shared<SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<Note> notes {make_note(192), make_note(240)};
    NoteTrack track {notes,      {}, {}, {}, {}, {}, TrackType::FiveFret,
                     global_data};
    PointSet points {track,
                     {tempo_map, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     RbEngine()};
    PointSet fifty_sqz_points {
        track,
        {{}, SpMode::Measure},
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
    std::vector<Note> notes;
    notes.reserve(60);
    for (auto i = 0; i < 60; ++i) {
        notes.push_back(make_note(192 * i));
    }
    const NoteTrack track {notes,
                           {},
                           {},
                           {},
                           {},
                           {},
                           TrackType::FiveFret,
                           std::make_unique<SongGlobalData>()};
    const PointSet points {track,
                           {{}, SpMode::Measure},
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
    const std::vector<Note> notes {make_note(192, 0), make_note(384, 192)};
    const NoteTrack track {notes,
                           {},
                           {},
                           {},
                           {},
                           {},
                           TrackType::FiveFret,
                           std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    const std::vector<Note> notes {make_note(192, 0), make_note(384, 192)};
    const NoteTrack track {notes,
                           {},
                           {},
                           {},
                           {},
                           {},
                           TrackType::FiveFret,
                           std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {
        make_note(192),      make_note(193), make_note(194), make_note(195),
        make_note(196),      make_note(197), make_note(198), make_note(199),
        make_note(200, 200), make_note(400)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_note(0), make_note(192, 192)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};

    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_note(100, 0), make_note(200, 100),
                             make_note(400, 0)};
    std::vector<StarPower> phrases {{Tick {200}, Tick {1}},
                                    {Tick {400}, Tick {1}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};

    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Solo> solos {{Tick {0}, Tick {576}, 100},
                             {Tick {768}, Tick {1152}, 200}};
    NoteTrack track {{},
                     {},
                     solos,
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    std::vector<std::tuple<SpPosition, int>> expected_solo_boosts {
        {{Beat(3.0), SpMeasure(0.75)}, 100},
        {{Beat(6.0), SpMeasure(1.5)}, 200}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        points.solo_boosts().cbegin(), points.solo_boosts().cend(),
        expected_solo_boosts.cbegin(), expected_solo_boosts.cend());
}

BOOST_AUTO_TEST_CASE(range_score_is_correct)
{
    NoteTrack track {{make_note(0, 192), make_note(386)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {
        make_chord(0, {{FIVE_FRET_GREEN, 0}, {FIVE_FRET_RED, 0}}),
        make_note(176, 100, FIVE_FRET_YELLOW),
        make_note(500, 0, FIVE_FRET_BLUE)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {
        make_ghl_chord(0, {{SIX_FRET_WHITE_LOW, 0}, {SIX_FRET_WHITE_MID, 0}}),
        make_ghl_note(176, 100, SIX_FRET_BLACK_HIGH),
        make_ghl_note(500, 0, SIX_FRET_OPEN)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::SixFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(0, DRUM_YELLOW, FLAGS_CYMBAL),
                             make_drum_note(176, DRUM_BLUE, FLAGS_CYMBAL),
                             make_drum_note(500, DRUM_KICK)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_drum_note(0, DRUM_KICK),
                             make_drum_note(192, DRUM_DOUBLE_KICK)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet single_points {track,
                            {{}, SpMode::Measure},
                            {},
                            SqueezeSettings::default_settings(),
                            {false, false, true, false},
                            ChDrumEngine()};
    PointSet double_points {track,
                            {{}, SpMode::Measure},
                            {},
                            SqueezeSettings::default_settings(),
                            DrumSettings::default_settings(),
                            ChDrumEngine()};

    BOOST_CHECK_EQUAL(single_points.cend() - single_points.cbegin(), 1);
    BOOST_CHECK_EQUAL(double_points.cend() - double_points.cbegin(), 2);
}

BOOST_AUTO_TEST_CASE(single_kicks_are_removed_with_disable_kick)
{
    std::vector<Note> notes {make_drum_note(0, DRUM_DOUBLE_KICK),
                             make_drum_note(192, DRUM_KICK)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     {true, true, false, false},
                     ChDrumEngine()};

    BOOST_CHECK_EQUAL(points.cend() - points.cbegin(), 1);
    BOOST_CHECK_EQUAL(points.cbegin()->value, 50);
}

BOOST_AUTO_TEST_CASE(disable_kick_doesnt_kill_sp_phrases)
{
    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(192, DRUM_KICK)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {200}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     {false, true, false, false},
                     ChDrumEngine()};

    BOOST_TEST(points.cbegin()->is_sp_granting_note);
}

BOOST_AUTO_TEST_CASE(double_kicks_dont_kill_phrases)
{
    std::vector<Note> notes {make_drum_note(0, DRUM_RED),
                             make_drum_note(192, DRUM_DOUBLE_KICK)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {200}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     {false, false, false, false},
                     ChDrumEngine()};

    BOOST_TEST(points.cbegin()->is_sp_granting_note);
}

BOOST_AUTO_TEST_CASE(activation_notes_are_marked_with_drum_fills)
{
    std::vector<Note> notes {make_drum_note(0), make_drum_note(192),
                             make_drum_note(385), make_drum_note(576)};
    std::vector<DrumFill> fills {{Tick {384}, Tick {5}}};
    NoteTrack track {notes,
                     {},
                     {},
                     fills,
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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

BOOST_AUTO_TEST_CASE(fills_ending_only_in_a_kick_are_not_killed)
{
    std::vector<Note> notes {make_drum_note(0), make_drum_note(1, DRUM_KICK)};
    std::vector<DrumFill> fills {{Tick {0}, Tick {2}}};
    NoteTrack track {notes,
                     {},
                     {},
                     fills,
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     {false, false, false, false},
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_ending_only_in_a_double_kick_are_not_killed)
{
    std::vector<Note> notes {make_drum_note(0),
                             make_drum_note(1, DRUM_DOUBLE_KICK)};
    std::vector<DrumFill> fills {{Tick {0}, Tick {2}}};
    NoteTrack track {notes,
                     {},
                     {},
                     fills,
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(
    fills_ending_in_a_multi_note_have_the_activation_attached_to_the_first_note)
{
    std::vector<Note> notes {make_drum_note(0), make_drum_note(0, DRUM_KICK)};
    std::vector<DrumFill> fills {{Tick {0}, Tick {2}}};
    NoteTrack track {notes,
                     {},
                     {},
                     fills,
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_drum_note(0), make_drum_note(192),
                             make_drum_note(370), make_drum_note(384)};
    std::vector<DrumFill> fills {
        {Tick {0}, Tick {2}}, {Tick {193}, Tick {5}}, {Tick {377}, Tick {4}}};
    NoteTrack track {notes,
                     {},
                     {},
                     fills,
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_drum_note(0), make_drum_note(192)};
    std::vector<DrumFill> fills {{Tick {0}, Tick {96}}};
    NoteTrack track {notes,
                     {},
                     {},
                     fills,
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_drum_note(0, DRUM_YELLOW, FLAGS_CYMBAL)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChDrumEngine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(begin->value, 65);
}

BOOST_AUTO_TEST_CASE(dynamics_get_double_points)
{
    std::vector<Note> notes {
        make_drum_note(0, DRUM_RED, FLAGS_GHOST),
        make_drum_note(192, DRUM_RED, FLAGS_ACCENT),
        make_drum_note(384, DRUM_RED),
        make_drum_note(576, DRUM_YELLOW,
                       static_cast<NoteFlags>(FLAGS_CYMBAL | FLAGS_GHOST)),
        make_drum_note(768, DRUM_YELLOW,
                       static_cast<NoteFlags>(FLAGS_CYMBAL | FLAGS_ACCENT)),
        make_drum_note(960, DRUM_YELLOW, FLAGS_CYMBAL)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_note(0), make_note(192), make_note(384)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
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
    std::vector<Note> notes {make_note(0), make_note(192), make_note(384)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {200}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin), begin + 2);
}

BOOST_AUTO_TEST_CASE(returns_next_point_always_next_for_non_overlap_engine)
{
    std::vector<Note> notes {make_note(0), make_note(192), make_note(384)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {200}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_unique<SongGlobalData>()};
    PointSet points {track,
                     {{}, SpMode::Measure},
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin), begin + 1);
}

BOOST_AUTO_TEST_SUITE_END()
