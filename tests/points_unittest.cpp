/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024, 2025 Raymond Wright
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

std::vector<SightRead::Beat> set_position_beats(const PointSet& points)
{
    std::vector<SightRead::Beat> values;
    values.reserve(static_cast<std::size_t>(
        std::distance(points.cbegin(), points.cend())));
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        values.push_back(p->position.beat);
    }
    return values;
}

SightRead::Note make_ghl_chord(
    int position,
    const std::vector<std::tuple<SightRead::SixFretNotes, int>>& lengths)
{
    SightRead::Note note;
    note.position = SightRead::Tick {position};
    note.flags = SightRead::FLAGS_SIX_FRET_GUITAR;
    for (const auto& [lane, length] : lengths) {
        note.lengths.at(lane) = SightRead::Tick {length};
    }

    return note;
}

std::shared_ptr<SightRead::SongGlobalData> make_resolution(int resolution)
{
    auto data = std::make_shared<SightRead::SongGlobalData>();
    data->resolution(resolution);
    data->tempo_map({{}, {}, {}, resolution});
    return data;
}

PathingSettings non_pro_drums_pathing_settings()
{
    return {std::make_unique<ChDrumEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            {false, false, false, false}};
}

PathingSettings min_kicks_drums_pathing_settings()
{
    return {std::make_unique<ChDrumEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            {false, true, false, false}};
}

PathingSettings extra_kicks_only_drums_pathing_settings()
{
    return {std::make_unique<ChDrumEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            {true, true, false, false}};
}

PathingSettings default_rb_bass_pathing_settings()
{
    return {std::make_unique<RbBassEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

PathingSettings default_fortnite_vocals_pathing_settings()
{
    return {std::make_unique<FortniteVocalsEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

PathingSettings mid_squeeze_ch_guitar_pathing_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            0.5,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

PathingSettings mid_squeeze_rb_pathing_settings()
{
    return {std::make_unique<RbEngine>(),
            0.5,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

PathingSettings slight_negative_video_lag_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {-0.20},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

PathingSettings negative_video_lag_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {-0.40},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}
}

BOOST_AUTO_TEST_SUITE(non_sustain_notes)

BOOST_AUTO_TEST_CASE(single_notes_give_fifty_points)
{
    SightRead::NoteTrack track {{make_note(768), make_note(960)},
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    std::vector<int> expected_values {50, 50};
    std::vector<int> values = set_values(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
}

BOOST_AUTO_TEST_CASE(chords_give_multiples_of_fifty_points)
{
    SightRead::NoteTrack track {{make_chord(768,
                                            {{SightRead::FIVE_FRET_GREEN, 0},
                                             {SightRead::FIVE_FRET_RED, 0}})},
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    std::vector<int> expected_values {100};
    std::vector<int> values = set_values(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
}

BOOST_AUTO_TEST_CASE(ghl_notes_behave_the_same_as_five_fret_notes)
{
    SightRead::NoteTrack track {{make_ghl_note(768), make_ghl_note(960)},
                                {},
                                SightRead::TrackType::SixFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
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
    SightRead::NoteTrack track {{make_note(768, 15)},
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet first_points {track,
                           {{{}, SpMode::Measure}, {}, {}},
                           default_guitar_pathing_settings()};
    std::vector<int> first_expected_values {50, 3};
    std::vector<SightRead::Beat> first_expected_beats {SightRead::Beat(4.0),
                                                       SightRead::Beat(4.0026)};
    SightRead::NoteTrack second_track {{make_note(768, 15)},
                                       {},
                                       SightRead::TrackType::FiveFret,
                                       make_resolution(200)};
    PointSet second_points {
        second_track,
        {{make_resolution(200)->tempo_map(), SpMode::Measure}, {}, {}},
        default_guitar_pathing_settings()};
    std::vector<int> second_expected_values {50, 2};
    std::vector<SightRead::Beat> second_expected_beats {
        SightRead::Beat(3.84), SightRead::Beat(3.8425)};

    std::vector<int> first_values = set_values(first_points);
    std::vector<SightRead::Beat> first_beats = set_position_beats(first_points);
    std::vector<int> second_values = set_values(second_points);
    std::vector<SightRead::Beat> second_beats
        = set_position_beats(second_points);

    BOOST_CHECK_EQUAL_COLLECTIONS(first_values.cbegin(), first_values.cend(),
                                  first_expected_values.cbegin(),
                                  first_expected_values.cend());
    BOOST_CHECK_EQUAL(first_beats.size(), first_expected_beats.size());
    for (auto i = 0U; i < first_beats.size(); ++i) {
        BOOST_CHECK_CLOSE(first_beats[i].value(),
                          first_expected_beats[i].value(), 0.01);
    }
    BOOST_CHECK_EQUAL_COLLECTIONS(second_values.cbegin(), second_values.cend(),
                                  second_expected_values.cbegin(),
                                  second_expected_values.cend());
    BOOST_CHECK_EQUAL(second_beats.size(), second_expected_beats.size());
    for (auto i = 0U; i < second_beats.size(); ++i) {
        BOOST_CHECK_CLOSE(second_beats[i].value(),
                          second_expected_beats[i].value(), 0.01);
    }
}

BOOST_AUTO_TEST_CASE(sustain_points_and_chords)
{
    SightRead::NoteTrack track {{make_chord(768,
                                            {{SightRead::FIVE_FRET_GREEN, 8},
                                             {SightRead::FIVE_FRET_RED, 8}})},
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    std::vector<int> expected_values {100, 2};
    std::vector<SightRead::Beat> expected_beats {SightRead::Beat(4.0),
                                                 SightRead::Beat(4.0026)};
    std::vector<int> values = set_values(points);
    std::vector<SightRead::Beat> beats = set_position_beats(points);

    BOOST_CHECK_EQUAL_COLLECTIONS(values.cbegin(), values.cend(),
                                  expected_values.cbegin(),
                                  expected_values.cend());
    BOOST_CHECK_EQUAL(beats.size(), expected_beats.size());
    for (auto i = 0U; i < beats.size(); ++i) {
        BOOST_CHECK_CLOSE(beats[i].value(), expected_beats[i].value(), 0.01);
    }
}

BOOST_AUTO_TEST_CASE(resolutions_below_25_do_not_enter_an_infinite_loop)
{
    SightRead::NoteTrack track {{make_note(768, 2)},
                                {},
                                SightRead::TrackType::FiveFret,
                                make_resolution(1)};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

    BOOST_CHECK_EQUAL(std::distance(points.cbegin(), points.cend()), 3);
}

BOOST_AUTO_TEST_CASE(sustains_of_uneven_length_are_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_chord(0,
                    {{SightRead::FIVE_FRET_GREEN, 1504},
                     {SightRead::FIVE_FRET_RED, 1504},
                     {SightRead::FIVE_FRET_YELLOW, 736}})},
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 686);
}

// RB1 Here It Goes Again m20 GRB sustain
BOOST_AUTO_TEST_CASE(chord_sustains_in_rb_are_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_chord(0,
                    {{SightRead::FIVE_FRET_GREEN, 1800},
                     {SightRead::FIVE_FRET_RED, 1800},
                     {SightRead::FIVE_FRET_BLUE, 1800}})},
        {},
        SightRead::TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_rb_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 210);
}

// RB1 Cherub Rock m24 Y sustain
BOOST_AUTO_TEST_CASE(
    rounding_from_length_in_rb_for_single_notes_is_handled_correctly)
{
    SightRead::NoteTrack track {{make_note(0, 419)},
                                {},
                                SightRead::TrackType::FiveFret,
                                make_resolution(480)};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_rb_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 35);
}

// RB1 Cherub Rock m65 RO sustain
BOOST_AUTO_TEST_CASE(rounding_from_length_in_rb_for_chords_is_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_chord(0,
                    {{SightRead::FIVE_FRET_RED, 419},
                     {SightRead::FIVE_FRET_ORANGE, 419}})},
        {},
        SightRead::TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {
        track, {{{}, SpMode::OdBeat}, {}, {}}, default_rb_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 70);
}

// GH1 Ace of Spades m6 Y sustain
BOOST_AUTO_TEST_CASE(gh1_one_beat_sustain_is_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_note(0, 1917, SightRead::FIVE_FRET_YELLOW)},
        {},
        SightRead::TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_gh1_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 150);
}

// GH1 Ace of Spades m96 O sustain
BOOST_AUTO_TEST_CASE(
    rounding_from_fractional_length_in_gh1_is_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_note(0, 1560, SightRead::FIVE_FRET_ORANGE)},
        {},
        SightRead::TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_gh1_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 131);
}

// GH1 Ace of Spades m15 BO sustain
BOOST_AUTO_TEST_CASE(
    rounding_from_length_in_gh1_for_chords_is_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_chord(0,
                    {{SightRead::FIVE_FRET_BLUE, 360},
                     {SightRead::FIVE_FRET_ORANGE, 360}})},
        {},
        SightRead::TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_gh1_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 138);
}

BOOST_AUTO_TEST_CASE(fortnite_festival_half_od_time_is_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_chord(0, {{SightRead::FIVE_FRET_GREEN, 1920}})},
        {},
        SightRead::TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {
        track,
        {{{{},
           {},
           {SightRead::Tick {0}, SightRead::Tick {960}, SightRead::Tick {1920}},
           480},
          SpMode::OdBeat},
         {},
         {}},
        default_fortnite_guitar_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 60);
}

BOOST_AUTO_TEST_CASE(long_fortnite_sustains_are_handled_correctly)
{
    SightRead::NoteTrack track {
        {make_chord(0, {{SightRead::FIVE_FRET_GREEN, 1920}})},
        {},
        SightRead::TrackType::FiveFret,
        make_resolution(480)};
    PointSet points {track,
                     {{{{}, {}, {}, 480}, SpMode::OdBeat}, {}, {}},
                     default_fortnite_vocals_pathing_settings()};
    const auto total_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto& a, const auto& b) { return a + b.value; });

    BOOST_CHECK_EQUAL(total_score, 136);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(points_are_sorted)
{
    SightRead::NoteTrack track {{make_note(768, 15), make_note(770, 0)},
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    const auto beats = set_position_beats(points);

    BOOST_TEST(std::is_sorted(beats.cbegin(), beats.cend()));
}

BOOST_AUTO_TEST_CASE(end_of_sp_phrase_points)
{
    SightRead::NoteTrack track {
        {make_note(768), make_note(960), make_note(1152)},
        {{SightRead::Tick {768}, SightRead::Tick {1}},
         {SightRead::Tick {900}, SightRead::Tick {50}},
         {SightRead::Tick {1100}, SightRead::Tick {53}}},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    PointSet unison_points {track,
                            {{{}, SpMode::Measure},
                             {},
                             {{SightRead::Tick {1100}, SightRead::Tick {53}}}},
                            default_rb3_pathing_settings()};

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
    std::vector<SightRead::Note> notes;
    notes.reserve(50);
    for (int i = 0; i < 50; ++i) {
        notes.push_back(make_note(192 * i));
    }
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
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
    std::vector<SightRead::Note> notes;
    notes.reserve(50);
    for (int i = 0; i < 50; ++i) {
        notes.push_back(make_note(192 * i));
    }
    notes.push_back(make_note(9600, 192));

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 4);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->base_value, 1);
}

BOOST_AUTO_TEST_CASE(later_sustain_points_in_extended_sustains_are_multiplied)
{
    std::vector<SightRead::Note> notes;
    notes.reserve(10);
    for (int i = 0; i < 10; ++i) {
        notes.push_back(make_note(192 * i));
    }
    notes[0].lengths[0] = SightRead::Tick {2000};

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 2);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->base_value, 1);
}

BOOST_AUTO_TEST_CASE(drum_notes_have_the_multiplier_handled_correctly)
{
    std::vector<SightRead::Note> notes;
    notes.reserve(10);
    for (int i = 0; i < 9; ++i) {
        notes.push_back(make_drum_note(192 * i, SightRead::DRUM_RED));
    }
    notes.push_back(make_drum_note(192 * 7, SightRead::DRUM_YELLOW));

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 1)->value, 100);
}

BOOST_AUTO_TEST_CASE(fortnite_notes_have_the_multiplier_handled_correctly)
{
    std::vector<SightRead::Note> notes;
    notes.reserve(11);
    for (int i = 0; i < 10; ++i) {
        notes.push_back(make_note(192 * i, 0, SightRead::FIVE_FRET_GREEN));
    }
    notes.push_back(make_note(192 * 7, 0, SightRead::FIVE_FRET_RED));

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FortniteFestival,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::OdBeat}, {}, {}},
                     default_fortnite_guitar_pathing_settings()};

    BOOST_CHECK_EQUAL(std::prev(points.cend(), 1)->value, 72);
}

BOOST_AUTO_TEST_CASE(gh1_multiplier_delay_accounted_for)
{
    std::vector<SightRead::Note> notes {
        make_note(0),   make_note(100),     make_note(200), make_note(300),
        make_note(400), make_note(500),     make_note(600), make_note(700),
        make_note(800), make_note(900, 100)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_gh1_pathing_settings()};

    BOOST_CHECK_EQUAL((points.cbegin() + 9)->value, 50);
    BOOST_CHECK_EQUAL((points.cbegin() + 10)->value, 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(hit_window_start_and_hit_window_end_are_set_correctly)

BOOST_AUTO_TEST_CASE(hit_window_starts_for_notes_are_correct)
{
    SightRead::TempoMap tempo_map {
        {},
        {{SightRead::Tick {0}, 150000}, {SightRead::Tick {768}, 200000}},
        {},
        192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(192), make_note(787)};
    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    PointSet points {track,
                     {{tempo_map, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_start.beat.value(), 0.825,
                      0.0001);
    BOOST_CHECK_CLOSE(std::next(points.cbegin())->hit_window_start.beat.value(),
                      3.89922, 0.0001);
}

BOOST_AUTO_TEST_CASE(hit_window_ends_for_notes_are_correct)
{
    SightRead::TempoMap tempo_map {
        {},
        {{SightRead::Tick {0}, 150000}, {SightRead::Tick {768}, 200000}},
        {},
        192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(192), make_note(749)};
    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    PointSet points {track,
                     {{tempo_map, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.175,
                      0.0001);
    BOOST_CHECK_CLOSE(std::next(points.cbegin())->hit_window_end.beat.value(),
                      4.10139, 0.0001);
}

BOOST_AUTO_TEST_CASE(hit_window_starts_and_ends_for_hold_points_are_correct)
{
    SightRead::TempoMap tempo_map {
        {},
        {{SightRead::Tick {0}, 150000}, {SightRead::Tick {768}, 200000}},
        {},
        192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(672, 192)};
    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    PointSet points {track,
                     {{tempo_map, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

    for (auto p = std::next(points.cbegin()); p < points.cend(); ++p) {
        BOOST_CHECK_CLOSE(p->position.beat.value(),
                          p->hit_window_start.beat.value(), 0.0001);
        BOOST_CHECK_CLOSE(p->position.beat.value(),
                          p->hit_window_end.beat.value(), 0.0001);
    }
}

BOOST_AUTO_TEST_CASE(squeeze_setting_is_accounted_for)
{
    SightRead::TempoMap tempo_map {
        {},
        {{SightRead::Tick {0}, 150000}, {SightRead::Tick {768}, 200000}},
        {},
        192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(192)};
    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    PointSet points {track,
                     {{tempo_map, SpMode::Measure}, {}, {}},
                     mid_squeeze_ch_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_start.beat.value(), 0.9125,
                      0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.0875,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(restricted_back_end_is_taken_account_of)
{
    SightRead::TempoMap tempo_map {
        {},
        {{SightRead::Tick {0}, 150000}, {SightRead::Tick {768}, 200000}},
        {},
        192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(192), make_note(240)};
    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    PointSet points {track,
                     {{tempo_map, SpMode::Measure}, {}, {}},
                     default_rb_pathing_settings()};
    PointSet fifty_sqz_points {track,
                               {{{}, SpMode::Measure}, {}, {}},
                               mid_squeeze_rb_pathing_settings()};

    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.125,
                      0.0001);
    BOOST_CHECK_CLOSE(fifty_sqz_points.cbegin()->hit_window_end.beat.value(),
                      1.0625, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(rb_bass_multiplier_is_taken_into_account)
{
    std::vector<SightRead::Note> notes;
    notes.reserve(60);
    for (auto i = 0; i < 60; ++i) {
        notes.push_back(make_note(192 * i));
    }
    const SightRead::NoteTrack track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_unique<SightRead::SongGlobalData>()};
    const PointSet points {track,
                           {{{}, SpMode::Measure}, {}, {}},
                           default_rb_bass_pathing_settings()};

    const auto total_points
        = std::accumulate(points.cbegin(), points.cend(), 0,
                          [&](auto x, auto y) { return x + y.value; });

    BOOST_CHECK_EQUAL(total_points, 5375);
}

BOOST_AUTO_TEST_SUITE(video_lag_is_taken_into_account)

BOOST_AUTO_TEST_CASE(negative_video_lag_is_handled_correctly)
{
    const std::vector<SightRead::Note> notes {make_note(192, 0),
                                              make_note(384, 192)};
    const SightRead::NoteTrack track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     slight_negative_video_lag_settings()};

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
    const std::vector<SightRead::Note> notes {make_note(192, 0),
                                              make_note(384, 192)};
    const SightRead::NoteTrack track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, positive_video_lag_settings()};

    BOOST_CHECK_CLOSE(points.cbegin()->position.beat.value(), 1.2, 0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_start.beat.value(), 1.06,
                      0.0001);
    BOOST_CHECK_CLOSE(points.cbegin()->hit_window_end.beat.value(), 1.34,
                      0.0001);
    BOOST_CHECK_CLOSE(std::next(points.cbegin(), 2)->position.beat.value(),
                      2.033854, 0.0001);
}

BOOST_AUTO_TEST_CASE(tick_points_are_not_multiplied_prematurely)
{
    std::vector<SightRead::Note> notes {
        make_note(192),      make_note(193), make_note(194), make_note(195),
        make_note(196),      make_note(197), make_note(198), make_note(199),
        make_note(200, 200), make_note(400)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, negative_video_lag_settings()};

    BOOST_CHECK_EQUAL(std::prev(points.cend())->value, 100);
    BOOST_CHECK_EQUAL(std::prev(points.cend(), 2)->value, 7);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(next_non_hold_point_is_correct)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192, 192)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};

    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

    BOOST_CHECK_EQUAL(points.next_non_hold_point(points.cbegin()),
                      points.cbegin());
    BOOST_CHECK_EQUAL(points.next_non_hold_point(std::next(points.cbegin(), 2)),
                      points.cend());
}

BOOST_AUTO_TEST_CASE(next_sp_granting_note_is_correct)
{
    std::vector<SightRead::Note> notes {make_note(100, 0), make_note(200, 100),
                                        make_note(400, 0)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {200}, SightRead::Tick {1}},
        {SightRead::Tick {400}, SightRead::Tick {1}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};

    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};

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
    std::vector<SightRead::Solo> solos {
        {SightRead::Tick {0}, SightRead::Tick {576}, 100},
        {SightRead::Tick {768}, SightRead::Tick {1152}, 200}};
    SightRead::NoteTrack track {{},
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    track.solos(solos);
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    std::vector<std::tuple<SpPosition, int>> expected_solo_boosts {
        {{SightRead::Beat(3.0), SpMeasure(0.75)}, 100},
        {{SightRead::Beat(6.0), SpMeasure(1.5)}, 200}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        points.solo_boosts().cbegin(), points.solo_boosts().cend(),
        expected_solo_boosts.cbegin(), expected_solo_boosts.cend());
}

BOOST_AUTO_TEST_CASE(range_score_is_correct)
{
    SightRead::NoteTrack track {{make_note(0, 192), make_note(386)},
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.range_score(begin, begin), 0);
    BOOST_CHECK_EQUAL(points.range_score(begin, end), 128);
    BOOST_CHECK_EQUAL(points.range_score(begin + 1, end - 1), 28);
}

BOOST_AUTO_TEST_CASE(colour_set_is_correct_for_five_fret)
{
    std::vector<SightRead::Note> notes {
        make_chord(
            0,
            {{SightRead::FIVE_FRET_GREEN, 0}, {SightRead::FIVE_FRET_RED, 0}}),
        make_note(176, 100, SightRead::FIVE_FRET_YELLOW),
        make_note(500, 0, SightRead::FIVE_FRET_BLUE)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.colour_set(begin), "GR");
    BOOST_CHECK_EQUAL(points.colour_set(begin + 1), "Y");
    BOOST_CHECK_EQUAL(points.colour_set(end - 1), "B");
}

BOOST_AUTO_TEST_CASE(colour_set_is_correct_for_six_fret)
{
    std::vector<SightRead::Note> notes {
        make_ghl_chord(0,
                       {{SightRead::SIX_FRET_WHITE_LOW, 0},
                        {SightRead::SIX_FRET_WHITE_MID, 0}}),
        make_ghl_note(176, 100, SightRead::SIX_FRET_BLACK_HIGH),
        make_ghl_note(500, 0, SightRead::SIX_FRET_OPEN)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::SixFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.colour_set(begin), "W1W2");
    BOOST_CHECK_EQUAL(points.colour_set(begin + 1), "B3");
    BOOST_CHECK_EQUAL(points.colour_set(end - 1), "open");
}

BOOST_AUTO_TEST_CASE(colour_set_is_correct_for_drums)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(0, SightRead::DRUM_YELLOW, SightRead::FLAGS_CYMBAL),
        make_drum_note(176, SightRead::DRUM_BLUE, SightRead::FLAGS_CYMBAL),
        make_drum_note(500, SightRead::DRUM_KICK)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
    const auto begin = points.cbegin();
    const auto end = points.cend();

    BOOST_CHECK_EQUAL(points.colour_set(begin), "R");
    BOOST_CHECK_EQUAL(points.colour_set(begin + 1), "Y cymbal");
    BOOST_CHECK_EQUAL(points.colour_set(end - 1), "kick");
}

BOOST_AUTO_TEST_CASE(double_kicks_only_appear_with_enable_double_kick)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_KICK),
        make_drum_note(192, SightRead::DRUM_DOUBLE_KICK)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet single_points {track,
                            {{{}, SpMode::Measure}, {}, {}},
                            default_pro_drums_pathing_settings()};
    PointSet double_points {track,
                            {{{}, SpMode::Measure}, {}, {}},
                            default_drums_pathing_settings()};

    BOOST_CHECK_EQUAL(single_points.cend() - single_points.cbegin(), 1);
    BOOST_CHECK_EQUAL(double_points.cend() - double_points.cbegin(), 2);
}

BOOST_AUTO_TEST_CASE(single_kicks_are_removed_with_disable_kick)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_DOUBLE_KICK),
        make_drum_note(192, SightRead::DRUM_KICK)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     extra_kicks_only_drums_pathing_settings()};

    BOOST_CHECK_EQUAL(points.cend() - points.cbegin(), 1);
    BOOST_CHECK_EQUAL(points.cbegin()->value, 50);
}

BOOST_AUTO_TEST_CASE(disable_kick_doesnt_kill_sp_phrases)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_KICK)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {200}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     min_kicks_drums_pathing_settings()};

    BOOST_TEST(points.cbegin()->is_sp_granting_note);
}

BOOST_AUTO_TEST_CASE(double_kicks_dont_kill_phrases)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_DOUBLE_KICK)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {200}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     non_pro_drums_pathing_settings()};

    BOOST_TEST(points.cbegin()->is_sp_granting_note);
}

BOOST_AUTO_TEST_CASE(activation_notes_are_marked_with_drum_fills)
{
    std::vector<SightRead::Note> notes {make_drum_note(0), make_drum_note(192),
                                        make_drum_note(385),
                                        make_drum_note(576)};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {384}, SightRead::Tick {5}}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    track.drum_fills(fills);
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
    const auto begin = points.cbegin();
    const auto fill_start = (begin + 2)->fill_start;

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST(!(begin + 1)->fill_start.has_value());
    BOOST_TEST(fill_start.has_value());
    if (fill_start.has_value()) {
        BOOST_CHECK_CLOSE(fill_start->value(), 1.0, 0.0001);
    }
    BOOST_TEST(!(begin + 3)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_ending_only_in_a_kick_are_not_killed)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0), make_drum_note(1, SightRead::DRUM_KICK)};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {0}, SightRead::Tick {2}}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    track.drum_fills(fills);
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     non_pro_drums_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_ending_only_in_a_double_kick_are_not_killed)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0), make_drum_note(1, SightRead::DRUM_DOUBLE_KICK)};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {0}, SightRead::Tick {2}}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    track.drum_fills(fills);
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(
    fills_ending_in_a_multi_note_have_the_activation_attached_to_the_first_note)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0), make_drum_note(0, SightRead::DRUM_KICK)};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {0}, SightRead::Tick {2}}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    track.drum_fills(fills);
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
    const auto begin = points.cbegin();

    const auto fill_start = begin->fill_start;
    BOOST_CHECK(fill_start.has_value());
    if (fill_start.has_value()) {
        BOOST_CHECK_CLOSE(fill_start->value(), 0.0, 0.0001);
    }
    BOOST_TEST(!(begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_are_attached_to_the_nearest_ending_point)
{
    std::vector<SightRead::Note> notes {make_drum_note(0), make_drum_note(192),
                                        make_drum_note(370),
                                        make_drum_note(384)};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {0}, SightRead::Tick {2}},
        {SightRead::Tick {193}, SightRead::Tick {5}},
        {SightRead::Tick {377}, SightRead::Tick {4}}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    track.drum_fills(fills);
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_TEST(begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
    BOOST_TEST(!(begin + 2)->fill_start.has_value());
    BOOST_TEST((begin + 3)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(fills_attach_to_later_point_in_case_of_a_tie)
{
    std::vector<SightRead::Note> notes {make_drum_note(0), make_drum_note(192)};
    std::vector<SightRead::DrumFill> fills {
        {SightRead::Tick {0}, SightRead::Tick {96}}};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    track.drum_fills(fills);
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_TEST(!begin->fill_start.has_value());
    BOOST_TEST((begin + 1)->fill_start.has_value());
}

BOOST_AUTO_TEST_CASE(cymbals_get_extra_points)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_YELLOW, SightRead::FLAGS_CYMBAL)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(begin->value, 65);
}

BOOST_AUTO_TEST_CASE(dynamics_get_double_points)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED, SightRead::FLAGS_GHOST),
        make_drum_note(192, SightRead::DRUM_RED, SightRead::FLAGS_ACCENT),
        make_drum_note(384, SightRead::DRUM_RED),
        make_drum_note(576, SightRead::DRUM_YELLOW,
                       static_cast<SightRead::NoteFlags>(
                           SightRead::FLAGS_CYMBAL | SightRead::FLAGS_GHOST)),
        make_drum_note(768, SightRead::DRUM_YELLOW,
                       static_cast<SightRead::NoteFlags>(
                           SightRead::FLAGS_CYMBAL | SightRead::FLAGS_ACCENT)),
        make_drum_note(960, SightRead::DRUM_YELLOW, SightRead::FLAGS_CYMBAL)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::Drums,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_drums_pathing_settings()};
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
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_gh1_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin),
                      std::next(begin));
}

BOOST_AUTO_TEST_CASE(returns_next_point_outside_current_sp_for_overlap_engine)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {200}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {
        track, {{{}, SpMode::Measure}, {}, {}}, default_gh1_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin), begin + 2);
}

BOOST_AUTO_TEST_CASE(returns_next_point_always_next_for_non_overlap_engine)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {200}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_unique<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::Measure}, {}, {}},
                     default_guitar_pathing_settings()};
    const auto begin = points.cbegin();

    BOOST_CHECK_EQUAL(points.first_after_current_phrase(begin), begin + 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(fortnite_notes_have_individual_split_points)
{
    std::vector<SightRead::Note> notes {
        make_note(0, 0, SightRead::FIVE_FRET_GREEN),
        make_note(0, 0, SightRead::FIVE_FRET_RED)};

    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FortniteFestival,
                                std::make_shared<SightRead::SongGlobalData>()};
    PointSet points {track,
                     {{{}, SpMode::OdBeat}, {}, {}},
                     default_fortnite_guitar_pathing_settings()};

    BOOST_CHECK_EQUAL(std::distance(points.cbegin(), points.cend()), 2);
}
