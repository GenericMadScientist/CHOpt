/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022 Raymond Wright
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

BOOST_AUTO_TEST_CASE(full_enough_to_activate_works_correctly)
{
    SpBar sp_bar {0.49, 0.49};

    BOOST_TEST(!sp_bar.full_enough_to_activate());

    sp_bar = {0.0, 0.5};

    BOOST_TEST(sp_bar.full_enough_to_activate());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(propagate_sp_over_whammy_works_correctly)

BOOST_AUTO_TEST_CASE(works_correctly_over_four_four)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<TimeSignature> time_sigs {{Tick {0}, 4, 4}};
    SpData sp_data {track,
                    {time_sigs, {}, {}, 192},
                    {},
                    SqueezeSettings::default_settings(),
                    ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max({Beat(0.0), Measure(0.0)},
                                             {Beat(4.0), Measure(1.0)}, 0.5),
        0.508333, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max({Beat(1.0), Measure(0.25)},
                                             {Beat(4.0), Measure(1.0)}, 0.5),
        0.50625, 0.0001);
}

BOOST_AUTO_TEST_CASE(works_correctly_over_three_four)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<TimeSignature> time_sigs {{Tick {0}, 3, 4}};
    SpData sp_data {track,
                    {time_sigs, {}, {}, 192},
                    {},
                    SqueezeSettings::default_settings(),
                    ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max(
            {Beat(0.0), Measure(0.0)}, {Beat(4.0), Measure(4.0 / 3)}, 0.5),
        0.466667, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max(
            {Beat(-1.0), Measure(-0.25)}, {Beat(4.0), Measure(4.0 / 3)}, 0.5),
        0.440083, 0.0001);
}

BOOST_AUTO_TEST_CASE(works_correctly_over_changing_time_signatures)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<TimeSignature> time_sigs {{Tick {0}, 4, 4}, {Tick {384}, 3, 4}};
    SpData sp_data {track,
                    {time_sigs, {}, {}, 192},
                    {},
                    SqueezeSettings::default_settings(),
                    ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max(
            {Beat(0.0), Measure(0.0)}, {Beat(4.0), Measure(7.0 / 6)}, 0.5),
        0.4875, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max(
            {Beat(1.0), Measure(0.25)}, {Beat(4.0), Measure(7.0 / 6)}, 0.5),
        0.485417, 0.0001);
}

BOOST_AUTO_TEST_CASE(returns_negative_one_if_sp_runs_out)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    std::vector<TimeSignature> time_sigs {{Tick {0}, 3, 4}, {Tick {384}, 4, 4}};
    SpData sp_data {track,
                    {time_sigs, {}, {}, 192},
                    {},
                    SqueezeSettings::default_settings(),
                    ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max(
            {Beat(0.0), Measure(0.0)}, {Beat(2.0), Measure(2.0 / 3)}, 0.015),
        -1.0, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max(
            {Beat(0.0), Measure(0.0)}, {Beat(10.0), Measure(8.0 / 3)}, 0.015),
        -1.0, 0.0001);
}

BOOST_AUTO_TEST_CASE(works_even_if_some_of_the_range_isnt_whammyable)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max({Beat(0.0), Measure(0.0)},
                                             {Beat(12.0), Measure(3.0)}, 0.5),
        0.496333, 0.0001);
}

BOOST_AUTO_TEST_CASE(sp_bar_does_not_exceed_full_bar)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max({Beat(0.0), Measure(0.0)},
                                             {Beat(10.0), Measure(2.5)}, 1.0),
        1.0, 0.0001);
    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max({Beat(0.0), Measure(0.0)},
                                             {Beat(10.5), Measure(2.625)}, 1.0),
        0.984375, 0.0001);
}

BOOST_AUTO_TEST_CASE(sustains_not_in_a_phrase_do_not_contribute_sp)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.propagate_sp_over_whammy_max({Beat(0.0), Measure(0.0)},
                                             {Beat(4.0), Measure(1.0)}, 1.0),
        0.875, 0.0001);
}

BOOST_AUTO_TEST_CASE(required_whammy_end_is_accounted_for)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112, 576),
                             make_note(3000)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_min(
                          {Beat(0.0), Measure(0.0)}, {Beat(4.0), Measure(1.0)},
                          0.5, {Beat(2.0), Measure(0.5)}),
                      0.441667, 0.0001);
}

BOOST_AUTO_TEST_CASE(
    check_optional_whammy_is_not_used_when_not_asked_for_in_minimum)
{
    constexpr double NEG_INF = -std::numeric_limits<double>::infinity();

    std::vector<Note> notes {make_note(0, 768), make_note(3072)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3100}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_CHECK_CLOSE(sp_data.propagate_sp_over_whammy_min(
                          {Beat(0.0), Measure(0.0)}, {Beat(4.0), Measure(1.0)},
                          0.5, {Beat {NEG_INF}, Measure {NEG_INF}}),
                      0.375, 0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(is_in_whammy_ranges_works_correctly)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {2000}},
                                    {Tick {2112}, Tick {50}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_TEST(sp_data.is_in_whammy_ranges(Beat(1.0)));
    BOOST_TEST(!sp_data.is_in_whammy_ranges(Beat(11.0)));
}

BOOST_AUTO_TEST_SUITE(available_whammy_works_correctly)

BOOST_AUTO_TEST_CASE(max_early_whammy)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112),
                             make_note(2304, 768)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(0.0), Beat(16.0)), 0.471333,
                      0.0001);
    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(10.0), Beat(11.0)), 0.0,
                      0.0001);
    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(1.0), Beat(8.0)), 0.2333333,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(mid_early_whammy)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112),
                             make_note(2304, 768)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {track,
                    {},
                    {},
                    {1.0, 0.5, Second {0.0}, Second {0.0}, Second {0.0}},
                    ChGuitarEngine()};

    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(0.0), Beat(16.0)), 0.469,
                      0.0001);
    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(10.0), Beat(11.0)), 0.0,
                      0.0001);
    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(1.0), Beat(8.0)), 0.2333333,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(negative_early_whammy)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112),
                             make_note(2304, 768)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {track,
                    {},
                    {},
                    {1.0, 0.0, Second {2.5}, Second {0.0}, Second {0.0}},
                    ChGuitarEngine()};

    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(0.0), Beat(10.0)),
                      0.1666667, 0.0001);
    BOOST_CHECK_CLOSE(sp_data.available_whammy(Beat(12.0), Beat(16.0)), 0.0,
                      0.0001);
}

BOOST_AUTO_TEST_CASE(three_argument_version_works_correctly)
{
    std::vector<Note> notes {make_note(0, 1920), make_note(2112),
                             make_note(2304, 768)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {3000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_CHECK_CLOSE(
        sp_data.available_whammy(Beat(0.0), Beat(12.0), Beat(12.0)), 0.3333333,
        0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(activation_end_point_works_correctly)

BOOST_AUTO_TEST_CASE(works_when_sp_is_sufficient)
{
    std::vector<Note> notes {make_note(0)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Position start {Beat(0.0), Measure(0.0)};
    Position end {Beat(1.0), Measure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.5).beat.value(), 1.0,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_sp_is_insufficient)
{
    std::vector<Note> notes {make_note(0)};
    NoteTrack track {notes,
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Position start {Beat(0.0), Measure(0.0)};
    Position end {Beat(1.0), Measure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 0.32,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_adding_whammy_makes_sp_sufficient)
{
    std::vector<Note> notes {make_note(0, 192), make_note(950)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {1000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Position start {Beat(0.0), Measure(0.0)};
    Position end {Beat(1.0), Measure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 1.0,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_whammy_is_present_but_insufficient)
{
    std::vector<Note> notes {make_note(0, 192), make_note(950)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {1000}}};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Position start {Beat(0.0), Measure(0.0)};
    Position end {Beat(2.0), Measure(0.5)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 1.386667,
        0.0001);
}

BOOST_AUTO_TEST_CASE(works_when_whammy_is_present_but_accumulation_is_too_slow)
{
    std::vector<Note> notes {make_note(0, 192), make_note(950)};
    std::vector<StarPower> phrases {{Tick {0}, Tick {1000}}};
    TempoMap tempo_map {{{Tick {0}, 2, 4}}, {}, {}, 192};
    NoteTrack track {notes,
                     phrases,
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {track,
                    tempo_map,
                    {},
                    SqueezeSettings::default_settings(),
                    ChGuitarEngine()};
    Position start {Beat(0.0), Measure(0.0)};
    Position end {Beat(1.0), Measure(0.25)};

    BOOST_CHECK_CLOSE(
        sp_data.activation_end_point(start, end, 0.01).beat.value(), 0.342857,
        0.0001);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(video_lag_is_taken_account_of)

BOOST_AUTO_TEST_CASE(negative_video_lag_is_handled_correctly)
{
    const std::vector<Note> notes {make_note(192, 192)};
    const std::vector<StarPower> phrases {{Tick {0}, Tick {384}}};
    const NoteTrack track {notes,
                           phrases,
                           {},
                           {},
                           {},
                           {},
                           TrackType::FiveFret,
                           std::make_shared<SongGlobalData>()};

    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};

    BOOST_TEST(sp_data.is_in_whammy_ranges(Beat(0.9)));
    BOOST_TEST(sp_data.is_in_whammy_ranges(Beat(1.9)));
}

BOOST_AUTO_TEST_CASE(positive_video_lag_is_handled_correctly)
{
    const std::vector<Note> notes {make_note(192, 192)};
    const std::vector<StarPower> phrases {{Tick {0}, Tick {384}}};
    const NoteTrack track {notes,
                           phrases,
                           {},
                           {},
                           {},
                           {},
                           TrackType::FiveFret,
                           std::make_shared<SongGlobalData>()};

    SpData sp_data {track,
                    {},
                    {},
                    {1.0, 1.0, Second {0.0}, Second {0.1}, Second {0.0}},
                    ChGuitarEngine()};

    BOOST_TEST(!sp_data.is_in_whammy_ranges(Beat(1.0)));
    BOOST_TEST(sp_data.is_in_whammy_ranges(Beat(1.9)));
}

BOOST_AUTO_TEST_SUITE_END()
