/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2025 Raymond Wright
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

#include "sp.hpp"
#include "test_helpers.hpp"

namespace {
PathingSettings mid_early_whammy_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            1.0,
            0.5,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings(),
            {SightRead::Second {0.0}}};
}

PathingSettings negative_early_whammy_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            1.0,
            0.0,
            SightRead::Second {2.5},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings(),
            {SightRead::Second {0.0}}};
}
}

BOOST_AUTO_TEST_SUITE(spbar_methods)

BOOST_AUTO_TEST_CASE(add_phrase_works_correctly)
{
    SpBar sp_bar {0.0, 0.25};
    sp_bar.add_phrase();

    BOOST_CHECK_CLOSE(sp_bar.min(), 0.25, 0.0001);
    BOOST_CHECK_CLOSE(sp_bar.max(), 0.5, 0.0001);

    sp_bar = {0.8, 1.0};
    sp_bar.add_phrase();

    BOOST_CHECK_CLOSE(sp_bar.min(), 1.0, 0.0001);
    BOOST_CHECK_CLOSE(sp_bar.max(), 1.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(full_enough_to_activate_works_with_half_bar_act_engines)
{
    SpBar sp_bar {0.49, 0.49};

    BOOST_TEST(!sp_bar.full_enough_to_activate(0.5));

    sp_bar = {0.0, 0.5};

    BOOST_TEST(sp_bar.full_enough_to_activate(0.5));
}

BOOST_AUTO_TEST_CASE(full_enough_to_activate_works_with_quarter_bar_act_engines)
{
    SpBar sp_bar {0.24, 0.24};

    BOOST_TEST(!sp_bar.full_enough_to_activate(0.25));

    sp_bar = {0.0, 0.25};

    BOOST_TEST(sp_bar.full_enough_to_activate(0.25));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(propagate_sp_over_whammy_works_correctly)

BOOST_AUTO_TEST_CASE(works_correctly_over_four_four)
{
    std::vector<SightRead::TimeSignature> time_sigs {
        {SightRead::Tick {0}, 4, 4}};
    SightRead::TempoMap tempo_map {time_sigs, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                global_data};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(4.0), SpMeasure(1.0)}, 0.5),
                      0.508333, 0.0001);
    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(1.0), SpMeasure(0.25)},
                          {SightRead::Beat(4.0), SpMeasure(1.0)}, 0.5),
                      0.50625, 0.0001);
}

BOOST_AUTO_TEST_CASE(works_correctly_over_three_four)
{
    std::vector<SightRead::TimeSignature> time_sigs {
        {SightRead::Tick {0}, 3, 4}};
    SightRead::TempoMap tempo_map {time_sigs, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                global_data};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(4.0), SpMeasure(4.0 / 3)}, 0.5),
                      0.466667, 0.0001);
    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(-1.0), SpMeasure(-0.25)},
                          {SightRead::Beat(4.0), SpMeasure(4.0 / 3)}, 0.5),
                      0.440083, 0.0001);
}

BOOST_AUTO_TEST_CASE(works_correctly_over_changing_time_signatures)
{
    std::vector<SightRead::TimeSignature> time_sigs {
        {SightRead::Tick {0}, 4, 4}, {SightRead::Tick {384}, 3, 4}};
    SightRead::TempoMap tempo_map {time_sigs, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                global_data};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(4.0), SpMeasure(7.0 / 6)}, 0.5),
                      0.4875, 0.0001);
    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(1.0), SpMeasure(0.25)},
                          {SightRead::Beat(4.0), SpMeasure(7.0 / 6)}, 0.5),
                      0.485417, 0.0001);
}

BOOST_AUTO_TEST_CASE(returns_negative_one_if_sp_runs_out)
{
    std::vector<SightRead::TimeSignature> time_sigs {
        {SightRead::Tick {0}, 3, 4}, {SightRead::Tick {384}, 4, 4}};
    SightRead::TempoMap tempo_map {time_sigs, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                global_data};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(2.0), SpMeasure(2.0 / 3)}, 0.015),
                      -1.0, 0.0001);
    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(10.0), SpMeasure(8.0 / 3)}, 0.015),
                      -1.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(works_even_if_some_of_the_range_isnt_whammyable)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(12.0), SpMeasure(3.0)}, 0.5),
                      0.496333, 0.0001);
}

BOOST_AUTO_TEST_CASE(sp_bar_does_not_exceed_full_bar)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(10.0), SpMeasure(2.5)}, 1.0),
                      1.0, 0.0001);
    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(10.5), SpMeasure(2.625)}, 1.0),
                      0.984375, 0.0001);
}

BOOST_AUTO_TEST_CASE(sustains_not_in_a_phrase_do_not_contribute_sp)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_max(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(4.0), SpMeasure(1.0)}, 1.0),
                      0.875, 0.0001);
}

BOOST_AUTO_TEST_CASE(required_whammy_end_is_accounted_for)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920),
                                        make_note(2112, 576), make_note(3000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_min(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(4.0), SpMeasure(1.0)}, 0.5,
                          {SightRead::Beat(2.0), SpMeasure(0.5)}),
                      0.441667, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    check_optional_whammy_is_not_used_when_not_asked_for_in_minimum)
{
    constexpr double NEG_INF = -std::numeric_limits<double>::infinity();

    std::vector<SightRead::Note> notes {make_note(0, 768), make_note(3072)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3100}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_min(
                          {SightRead::Beat(0.0), SpMeasure(0.0)},
                          {SightRead::Beat(4.0), SpMeasure(1.0)}, 0.5,
                          {SightRead::Beat {NEG_INF}, SpMeasure {NEG_INF}}),
                      0.375, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(is_in_whammy_ranges_works_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920), make_note(2112)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {2000}},
        {SightRead::Tick {2112}, SightRead::Tick {50}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_TEST(sp_data.is_in_whammy_ranges(SightRead::Beat(1.0)));
    BOOST_TEST(!sp_data.is_in_whammy_ranges(SightRead::Beat(11.0)));
}

BOOST_AUTO_TEST_SUITE(available_whammy_works_correctly)

BOOST_AUTO_TEST_CASE(max_early_whammy)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920), make_note(2112),
                                        make_note(2304, 768)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(0.0), SightRead::Beat(16.0)),
        0.471333, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(10.0), SightRead::Beat(11.0)),
        0.0, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(1.0), SightRead::Beat(8.0)),
        0.2333333, 0.0001);
}

BOOST_AUTO_TEST_CASE(mid_early_whammy)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920), make_note(2112),
                                        make_note(2304, 768)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, mid_early_whammy_settings()};

    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(0.0), SightRead::Beat(16.0)),
        0.469, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(10.0), SightRead::Beat(11.0)),
        0.0, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(1.0), SightRead::Beat(8.0)),
        0.2333333, 0.0001);
}

BOOST_AUTO_TEST_CASE(negative_early_whammy)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920), make_note(2112),
                                        make_note(2304, 768)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, negative_early_whammy_settings()};

    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(0.0), SightRead::Beat(10.0)),
        0.1666667, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(SightRead::Beat(12.0), SightRead::Beat(16.0)),
        0.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(three_argument_version_works_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920), make_note(2112),
                                        make_note(2304, 768)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_CHECK_CLOSE(sp_data.available_whammy(SightRead::Beat(0.0),
                                               SightRead::Beat(12.0),
                                               SightRead::Beat(12.0)),
                      0.3333333, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(activation_end_point_works_correctly)

BOOST_AUTO_TEST_CASE(works_when_sp_is_sufficient)
{
    std::vector<SightRead::Note> notes {make_note(0)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};
    SpPosition start {SightRead::Beat(0.0), SpMeasure(0.0)};
    SpPosition end {SightRead::Beat(1.0), SpMeasure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.5).beat.value(), 1.0,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_sp_is_insufficient)
{
    std::vector<SightRead::Note> notes {make_note(0)};
    SightRead::NoteTrack track {notes,
                                {},
                                SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};
    SpPosition start {SightRead::Beat(0.0), SpMeasure(0.0)};
    SpPosition end {SightRead::Beat(1.0), SpMeasure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 0.32,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_adding_whammy_makes_sp_sufficient)
{
    std::vector<SightRead::Note> notes {make_note(0, 192), make_note(950)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};
    SpPosition start {SightRead::Beat(0.0), SpMeasure(0.0)};
    SpPosition end {SightRead::Beat(1.0), SpMeasure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 1.0,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_whammy_is_present_but_insufficient)
{
    std::vector<SightRead::Note> notes {make_note(0, 192), make_note(950)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};
    SpPosition start {SightRead::Beat(0.0), SpMeasure(0.0)};
    SpPosition end {SightRead::Beat(2.0), SpMeasure(0.5)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 1.386667,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_whammy_is_present_but_accumulation_is_too_slow)
{
    SightRead::TempoMap tempo_map {{{SightRead::Tick {0}, 2, 4}}, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(0, 192), make_note(950)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1000}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                global_data};
    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};
    SpPosition start {SightRead::Beat(0.0), SpMeasure(0.0)};
    SpPosition end {SightRead::Beat(1.0), SpMeasure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 0.342857,
        0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(video_lag_is_taken_account_of)

BOOST_AUTO_TEST_CASE(negative_video_lag_is_handled_correctly)
{
    const std::vector<SightRead::Note> notes {make_note(192, 192)};
    const std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {384}}};
    const SightRead::NoteTrack track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};

    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, default_guitar_pathing_settings()};

    BOOST_TEST(sp_data.is_in_whammy_ranges(SightRead::Beat(0.9)));
    BOOST_TEST(sp_data.is_in_whammy_ranges(SightRead::Beat(1.9)));
}

BOOST_AUTO_TEST_CASE(positive_video_lag_is_handled_correctly)
{
    const std::vector<SightRead::Note> notes {make_note(192, 192)};
    const std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {384}}};
    const SightRead::NoteTrack track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};

    SpData sp_data {
        track, {{}, SpMode::Measure}, {}, positive_video_lag_settings()};

    BOOST_TEST(!sp_data.is_in_whammy_ranges(SightRead::Beat(1.0)));
    BOOST_TEST(sp_data.is_in_whammy_ranges(SightRead::Beat(1.9)));
}

BOOST_AUTO_TEST_SUITE_END()
