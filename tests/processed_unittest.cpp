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

#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "test_helpers.hpp"

namespace {
PathingSettings no_squeeze_negative_video_lag_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            0.0,
            SightRead::DrumSettings::default_settings(),
            {0.0, SightRead::Second {0.0}, SightRead::Second {-0.1},
             SightRead::Second {0.0}}};
}

PathingSettings no_squeeze_positive_video_lag_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            0.0,
            SightRead::DrumSettings::default_settings(),
            {0.0, SightRead::Second {0.0}, SightRead::Second {0.1},
             SightRead::Second {0.0}}};
}
}

BOOST_AUTO_TEST_SUITE(three_arg_total_available_sp_counts_sp_correctly)

BOOST_AUTO_TEST_CASE(phrases_are_counted_correctly)
{
    std::vector<SightRead::Note> notes {
        make_note(0),        make_note(192),  make_note(384),  make_note(576),
        make_note(768, 192), make_note(1152), make_note(1344), make_note(1536)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {768}, SightRead::Tick {400}},
        {SightRead::Tick {1344}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::Measure},
                        default_guitar_pathing_settings(),
                        {},
                        {}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(song.total_available_sp(SightRead::Beat(0.0),
                                              points.cbegin(),
                                              points.cbegin() + 1),
                      SpBar(0.25, 0.25));
    BOOST_CHECK_EQUAL(song.total_available_sp(SightRead::Beat(0.0),
                                              points.cbegin(),
                                              points.cbegin() + 2),
                      SpBar(0.25, 0.25));
    BOOST_CHECK_EQUAL(song.total_available_sp(SightRead::Beat(0.5),
                                              points.cbegin() + 2,
                                              points.cbegin() + 3),
                      SpBar(0.25, 0.25));
}

BOOST_AUTO_TEST_CASE(whammy_is_counted_correctly)
{
    std::vector<SightRead::Note> notes {
        make_note(0),        make_note(192),  make_note(384),  make_note(576),
        make_note(768, 192), make_note(1152), make_note(1344), make_note(1536)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {768}, SightRead::Tick {400}},
        {SightRead::Tick {1344}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::Measure},
                        default_guitar_pathing_settings(),
                        {},
                        {}};
    const auto& points = song.points();
    auto result = song.total_available_sp(
        SightRead::Beat(4.0), points.cbegin() + 4, points.cbegin() + 5);

    BOOST_CHECK_CLOSE(result.min(), 0.0, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.001128472, 0.0001);
}

BOOST_AUTO_TEST_CASE(whammy_is_counted_correctly_even_started_mid_hold)
{
    std::vector<SightRead::Note> notes {
        make_note(0),        make_note(192),  make_note(384),  make_note(576),
        make_note(768, 192), make_note(1152), make_note(1344), make_note(1536)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {768}, SightRead::Tick {400}},
        {SightRead::Tick {1344}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::Measure},
                        default_guitar_pathing_settings(),
                        {},
                        {}};
    const auto& points = song.points();
    auto result = song.total_available_sp(SightRead::Beat(4.5),
                                          points.cend() - 3, points.cend() - 3);

    BOOST_CHECK_CLOSE(result.min(), 0.0, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.01666667, 0.0001);
}

BOOST_AUTO_TEST_CASE(required_whammy_end_is_accounted_for)
{
    std::vector<SightRead::Note> notes {
        make_note(0),        make_note(192),  make_note(384),  make_note(576),
        make_note(768, 192), make_note(1152), make_note(1344), make_note(1536)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {768}, SightRead::Tick {400}},
        {SightRead::Tick {1344}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::Measure},
                        default_guitar_pathing_settings(),
                        {},
                        {}};
    const auto& points = song.points();
    auto result
        = song.total_available_sp(SightRead::Beat(4.0), points.cbegin() + 4,
                                  points.cbegin() + 5, SightRead::Beat(4.02));

    BOOST_CHECK_CLOSE(result.min(), 0.000666667, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.001128472, 0.0001);

    result
        = song.total_available_sp(SightRead::Beat(4.0), points.cbegin() + 4,
                                  points.cbegin() + 5, SightRead::Beat(4.10));

    BOOST_CHECK_CLOSE(result.min(), 0.001128472, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.001128472, 0.0001);
}

BOOST_AUTO_TEST_CASE(sp_does_not_exceed_full_bar)
{
    std::vector<SightRead::Note> notes {
        make_note(0),        make_note(192),  make_note(384),  make_note(576),
        make_note(768, 192), make_note(1152), make_note(1344), make_note(1536)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {768}, SightRead::Tick {400}},
        {SightRead::Tick {1344}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::Measure},
                        default_guitar_pathing_settings(),
                        {},
                        {}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(song.total_available_sp(SightRead::Beat(0.0),
                                              points.cbegin(),
                                              points.cend() - 1),
                      SpBar(1.0, 1.0));
}

BOOST_AUTO_TEST_CASE(
    sp_notes_are_counted_from_first_point_when_start_is_past_middle)
{
    std::vector<SightRead::Note> notes {
        make_note(0),        make_note(192),  make_note(384),  make_note(576),
        make_note(768, 192), make_note(1152), make_note(1344), make_note(1536)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {768}, SightRead::Tick {400}},
        {SightRead::Tick {1344}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::Measure},
                        default_guitar_pathing_settings(),
                        {},
                        {}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(song.total_available_sp(SightRead::Beat(0.05),
                                              points.cbegin(),
                                              points.cbegin() + 1),
                      SpBar(0.25, 0.25));
}

BOOST_AUTO_TEST_CASE(unison_bonuses_are_taken_account_of)
{
    std::vector<SightRead::Note> notes {
        make_note(0),        make_note(192),  make_note(384),  make_note(576),
        make_note(768, 192), make_note(1152), make_note(1344), make_note(1536)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {768}, SightRead::Tick {400}},
        {SightRead::Tick {1344}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::OdBeat},
                        default_rb3_pathing_settings(),
                        {},
                        {{SightRead::Tick {0}, SightRead::Tick {50}}}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(song.total_available_sp(SightRead::Beat(0.0),
                                              points.cbegin(),
                                              points.cbegin() + 1),
                      SpBar(0.5, 0.5));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(
    total_available_sp_with_earliest_pos_counts_sp_correctly_and_gives_earliest_posiiton)
{
    std::vector<SightRead::Note> notes {make_note(0, 1459), make_note(1459)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::Measure},
                        default_guitar_pathing_settings(),
                        {},
                        {}};
    const auto& points = song.points();

    const auto& [sp_bar, pos] = song.total_available_sp_with_earliest_pos(
        SightRead::Beat(0.0), points.cbegin(), std::prev(points.cend()),
        std::prev(points.cend(), 2)->position);

    BOOST_CHECK_CLOSE(sp_bar.max(), 0.5, 0.001);
    BOOST_CHECK_CLOSE(pos.beat.value(), 7.5, 0.001);
}

BOOST_AUTO_TEST_CASE(total_available_sp_with_earliest_pos_counts_unison_bonuses)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {note_track,
                        {{}, SpMode::OdBeat},
                        default_rb3_pathing_settings(),
                        {},
                        {SightRead::Tick {0}}};
    const auto& points = song.points();

    const auto& [sp_bar, pos] = song.total_available_sp_with_earliest_pos(
        SightRead::Beat(0.0), points.cbegin(), std::next(points.cbegin()),
        {SightRead::Beat(0.0), SpMeasure(0.0)});

    BOOST_CHECK_CLOSE(sp_bar.max(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_SUITE(is_candidate_valid_works_with_no_whammy)

BOOST_AUTO_TEST_CASE(full_bar_works_with_time_signatures)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(3072), make_note(6144)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {1.0, 1.0}};

    SightRead::TempoMap tempo_map {{{SightRead::Tick {0}, 3, 4}}, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);
    SightRead::NoteTrack second_note_track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    ProcessedSong second_track {second_note_track,
                                {tempo_map, SpMode::Measure},
                                default_guitar_pathing_settings(),
                                {},
                                {}};
    const auto& second_points = second_track.points();
    ActivationCandidate second_candidate {
        second_points.cbegin(),
        second_points.cbegin() + 3,
        {SightRead::Beat(0.0), SpMeasure(0.0)},
        {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
    BOOST_CHECK_EQUAL(
        second_track.is_candidate_valid(second_candidate).validity,
        ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(half_bar_works_with_time_signatures)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(3072), make_note(6144)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    SightRead::TempoMap tempo_map {{{SightRead::Tick {0}, 3, 4}}, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);
    SightRead::NoteTrack second_note_track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    ProcessedSong second_track {second_note_track,
                                {tempo_map, SpMode::Measure},
                                default_guitar_pathing_settings(),
                                {},
                                {}};
    const auto& second_points = second_track.points();
    ActivationCandidate second_candidate {
        second_points.cbegin(),
        second_points.cbegin() + 2,
        {SightRead::Beat(0.0), SpMeasure(0.0)},
        {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
    BOOST_CHECK_EQUAL(
        second_track.is_candidate_valid(second_candidate).validity,
        ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(below_half_bar_never_works)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(3072), make_note(6144)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.25, 0.25}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(check_next_point_needs_to_not_lie_in_activation)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(3072), make_note(6144)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.6, 0.6}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_CASE(check_intermediate_sp_is_accounted_for)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(3072), make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {3000}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.8, 0.8}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(check_only_reached_intermediate_sp_is_accounted_for)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(6000), make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {6000}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.8, 0.8}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(last_notes_sp_status_is_not_ignored)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(3072), make_note(4000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {3072}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_CASE(sp_bar_does_not_exceed_full_bar)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(2),
                                        make_note(7000)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {2}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(earliest_activation_point_is_considered)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(3072), make_note(6144)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(-2.0), SpMeasure(-0.5)},
                                   {0.53125, 0.53125}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

// There was a bug where if an optimal activation starts on an SP granting note,
// and the note must be squeezed late, then the activation could be drawn too
// far. This happened in the path for Das Neue bleibt beim Alten from CSC
// November 2020.
BOOST_AUTO_TEST_CASE(
    activations_starting_on_an_sp_granting_note_have_the_correct_end)
{
    SightRead::TempoMap tempo_map {{{SightRead::Tick {0}, 4, 4}},
                                   {{SightRead::Tick {0}, 300000}},
                                   {},
                                   192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {
        make_chord(
            384,
            {{SightRead::FIVE_FRET_GREEN, 0}, {SightRead::FIVE_FRET_RED, 0}}),
        make_note(5088), make_note(5136)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {384}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret, global_data};
    ProcessedSong track {note_track,
                         {tempo_map, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(2.24), SpMeasure(0.56)},
                                   {0.5, 0.5}};

    const auto result = track.is_candidate_valid(
        candidate, 1.0, {SightRead::Beat {27}, SpMeasure {6.75}});

    BOOST_CHECK_EQUAL(result.validity, ActValidity::success);
    BOOST_CHECK_LT(result.ending_position.beat.value(), 26.5);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_acknowledges_unison_bonuses)

BOOST_AUTO_TEST_CASE(mid_activation_unison_bonuses_are_accounted_for)
{
    std::vector<SightRead::Note> notes {make_note(192), make_note(5376)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {192}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::OdBeat},
                         default_rb3_pathing_settings(),
                         {},
                         {SightRead::Tick {192}}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    const auto result = track.is_candidate_valid(candidate);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::success);
}

BOOST_AUTO_TEST_CASE(last_note_unison_bonus_accounted_for_excess_sp)
{
    std::vector<SightRead::Note> notes {make_note(192), make_note(5376)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {192}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::OdBeat},
                         default_rb3_pathing_settings(),
                         {},
                         {SightRead::Tick {192}}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin(),
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    const auto result = track.is_candidate_valid(candidate);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_works_with_whammy)

BOOST_AUTO_TEST_CASE(check_whammy_is_counted)
{
    std::vector<SightRead::Note> notes {make_note(0, 960), make_note(3840),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {7000}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

// This comes up in Epidox: otherwise, CHOpt doesn't think you can squeeze to
// the G after the B in the Robotic Buildup activation.
BOOST_AUTO_TEST_CASE(check_whammy_from_end_of_sp_sustain_before_note_is_counted)
{
    std::vector<SightRead::Note> notes {make_note(0, 960), make_note(2880),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {7000}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cend() - 2,
                                   points.cend() - 1,
                                   {SightRead::Beat(1.0), SpMeasure(0.25)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

// There was a bug where an activation starting on an SP sustain would double
// count the whammy around the note, giving impossible activations. An example
// is the last activatiom of Soulless.
BOOST_AUTO_TEST_CASE(
    whammy_around_the_start_of_an_sp_sustain_is_not_doubled_counted)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384, 192), make_note(5260)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {384}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 2,
                                   points.cend() - 1,
                                   {SightRead::Beat(1.0), SpMeasure(0.25)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(check_compressed_activations_are_counted)
{
    std::vector<SightRead::Note> notes {make_note(0, 960), make_note(3840),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {7000}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.9}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_minimum_sp)

BOOST_AUTO_TEST_CASE(lower_sp_is_considered)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(2304), make_note(3072),
                                        make_note(4608)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 1.0}};
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(lower_sp_is_only_considered_down_to_a_half_bar)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(1536),
                                        make_note(2304), make_note(3072),
                                        make_note(4608)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.25, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_squeezing)

BOOST_AUTO_TEST_CASE(
    front_end_and_back_end_of_the_activation_endpoints_are_considered)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(3110)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(next_note_can_be_squeezed_late_to_avoid_going_too_far)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(3034),
                                        make_note(3053)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(intermediate_sp_can_be_hit_early)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(3102),
                                        make_note(4608)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {3100}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(intermediate_sp_can_be_hit_late)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(768),
                                        make_note(6942)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {768}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(is_candidate_valid_handles_very_high_bpm_sp_granting_notes)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(768), make_note(4608),
                                        make_note(5376)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {4608}, SightRead::Tick {50}}};
    SightRead::TempoMap tempo_map {
        {}, {{SightRead::Tick {3840}, 4000000}}, {}, 192};

    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret, global_data};
    ProcessedSong track {note_track,
                         {tempo_map, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 2,
                                   points.cbegin() + 4,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_squeeze_param)

BOOST_AUTO_TEST_CASE(front_end_and_back_end_are_restricted)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(3110)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::insufficient_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(Intermediate_sp_front_end_is_restricted)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(3102),
                                        make_note(4608)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {3100}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::insufficient_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(intermediate_sp_back_end_is_restricted)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(768),
                                        make_note(6942)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {768}, SightRead::Tick {100}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::insufficient_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(next_note_back_end_is_restricted)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(3034),
                                        make_note(3053)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::surplus_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(end_position_is_finite_if_activation_goes_past_last_note)
{
    std::vector<SightRead::Note> notes {make_note(0)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin(),
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {1.0, 1.0}};
    auto result = track.is_candidate_valid(candidate, 1.0);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::success);
    BOOST_CHECK_LT(result.ending_position.beat.value(), 40.0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_no_overlap)

BOOST_AUTO_TEST_CASE(mid_act_phrases_not_collected)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(2688),
                                        make_note(3072), make_note(3840)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {2688}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(end_of_act_phrase_not_collected)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(3072),
                                        make_note(3840)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {3072}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(mid_act_whammy_is_not_collected)
{
    std::vector<SightRead::Note> notes {make_note(0, 1920), make_note(3456)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(mid_act_whammy_is_not_collected_for_end_calculation)
{
    std::vector<SightRead::Note> notes {make_note(0, 2304)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_CLOSE(
        track
            .is_candidate_valid(candidate, 0.0,
                                {SightRead::Beat {1.0}, SpMeasure {0.25}})
            .ending_position.beat.value(),
        16.0, 0.0001);
}

// This happened in GH1 Cheat on the Church second act, causing CHOpt to think
// you could go all the way back to the yellow.
BOOST_AUTO_TEST_CASE(mid_act_whammy_around_sp_granting_note_doesnt_get_added)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(768, 192),
                                        make_note(3168)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {768}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(is_candidate_valid_takes_into_account_forced_whammy)
{
    std::vector<SightRead::Note> notes {make_note(0, 768), make_note(3072),
                                        make_note(3264)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3300}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 2,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(
        track
            .is_candidate_valid(candidate, 1.0,
                                {SightRead::Beat(0.0), SpMeasure(0.0)})
            .validity,
        ActValidity::success);
    BOOST_CHECK_EQUAL(
        track
            .is_candidate_valid(candidate, 1.0,
                                {SightRead::Beat(4.0), SpMeasure(1.0)})
            .validity,
        ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_CASE(
    is_candidate_valid_also_takes_account_of_whammy_from_end_of_sp_sustain_before_note_is_counted)
{
    std::vector<SightRead::Note> notes {make_note(0, 960), make_note(2880),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {7000}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cend() - 2,
                                   points.cend() - 1,
                                   {SightRead::Beat(1.0), SpMeasure(0.25)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.0).validity,
                      ActValidity::success);
}

// This is to stop a bug that appears in Edd Instrument Solo from CTH2: the last
// note is an SP sustain and the last activation was shown as ending during it,
// when it should go past the end of the song.
BOOST_AUTO_TEST_CASE(
    is_candidate_valid_takes_account_of_overlapped_phrase_at_end_if_last_note_is_whammy)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(3456, 192)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}},
        {SightRead::Tick {3456}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 2,
                                   points.cend() - 1,
                                   {SightRead::Beat(1.0), SpMeasure(0.25)},
                                   {0.5, 0.5}};

    auto result = track.is_candidate_valid(candidate, 0.0);

    BOOST_CHECK_GT(result.ending_position.beat.value(), 20.0);
}

// This is to stop a bug that appears in Time Traveler from CB: some of the
// mid-sustain activations were misdrawn. The exact problem is that if we must
// activate past the earliest activation point in order for the activation to
// work, then the minimum sp after hitting the point is not clamped to 0, which
// caused the endpoint to be too early.
BOOST_AUTO_TEST_CASE(is_candidate_valid_correctly_clamps_low_sp)
{
    SightRead::TempoMap tempo_map {{{SightRead::Tick {0}, 1, 4}}, {}, {}, 192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(0, 6720)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret, global_data};
    ProcessedSong track {note_track,
                         {tempo_map, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 500,
                                   points.cbegin() + 750,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {1.0, 1.0}};

    auto result = track.is_candidate_valid(candidate, 0.0);

    BOOST_CHECK_GT(result.ending_position.beat.value(), 27.3);
}

BOOST_AUTO_TEST_CASE(adjusted_hit_window_functions_return_correct_values)
{
    std::vector<SightRead::Note> notes {make_note(0)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();

    BOOST_CHECK_CLOSE(
        track.adjusted_hit_window_start(points.cbegin(), 0.5).beat.value(),
        -0.07, 0.0001);
    BOOST_CHECK_CLOSE(
        track.adjusted_hit_window_start(points.cbegin(), 1.0).beat.value(),
        -0.14, 0.0001);
    BOOST_CHECK_CLOSE(
        track.adjusted_hit_window_end(points.cbegin(), 0.5).beat.value(), 0.07,
        0.0001);
    BOOST_CHECK_CLOSE(
        track.adjusted_hit_window_end(points.cbegin(), 1.0).beat.value(), 0.14,
        0.0001);
}

BOOST_AUTO_TEST_SUITE(video_lag_is_taken_account_of)

BOOST_AUTO_TEST_CASE(effect_on_whammy_is_taken_account_of)
{
    std::vector<SightRead::Note> notes {make_note(192), make_note(384, 192),
                                        make_note(768)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {384}, SightRead::Tick {1}}};
    SightRead::NoteTrack track {notes, phrases, SightRead::TrackType::FiveFret,
                                std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong song {track,
                        {{}, SpMode::Measure},
                        no_squeeze_negative_video_lag_settings(),
                        {},
                        {}};
    const auto& points = song.points();

    auto result = song.total_available_sp(SightRead::Beat(0.0), points.cbegin(),
                                          std::prev(points.cend()));

    BOOST_CHECK_CLOSE(result.max(), 0.29, 0.0001);
}

BOOST_AUTO_TEST_CASE(effect_on_notes_is_taken_account_of)
{
    SightRead::TempoMap tempo_map {
        {{SightRead::Tick {0}, 4, 4}, {SightRead::Tick {3840}, 2, 4}},
        {},
        {},
        192};
    auto global_data = std::make_shared<SightRead::SongGlobalData>();
    global_data->tempo_map(tempo_map);

    std::vector<SightRead::Note> notes {make_note(768), make_note(3840)};
    SightRead::NoteTrack track {
        notes, {}, SightRead::TrackType::FiveFret, global_data};
    ProcessedSong song {track,
                        {tempo_map, SpMode::Measure},
                        no_squeeze_positive_video_lag_settings(),
                        {},
                        {}};
    const auto& points = song.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {SightRead::Beat(0.0), SpMeasure(0.0)},
                                   {0.5, 0.5}};

    auto result = song.is_candidate_valid(candidate);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_drums_returns_the_correct_value)

BOOST_AUTO_TEST_CASE(false_for_guitar)
{
    SightRead::NoteTrack note_track {
        {},
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_drums_pathing_settings(),
                         {},
                         {}};

    BOOST_CHECK(!track.is_drums());
}

BOOST_AUTO_TEST_CASE(true_for_drums)
{
    SightRead::NoteTrack note_track {
        {},
        {},
        SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_drums_pathing_settings(),
                         {},
                         {}};

    BOOST_TEST(track.is_drums());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(path_summary_produces_the_correct_output)

BOOST_AUTO_TEST_CASE(overlap_and_es_are_denoted_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(576),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {6144}, SightRead::Tick {50}}};
    std::vector<SightRead::Solo> solos {
        {SightRead::Tick {0}, SightRead::Tick {50}, 50}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.solos(solos);
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 2, points.cbegin() + 3,
                 SightRead::Beat {0.0}, SightRead::Beat {0.0}}},
               100};

    const char* desired_path_output = "Path: 2(+1)-ES1\n"
                                      "No SP score: 300\n"
                                      "Total score: 400\n"
                                      "Average multiplier: 1.400x\n"
                                      "2(+1): NN (G)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(overlapped_sp_is_handled_correctly_for_non_overlap_games)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(576),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {6144}, SightRead::Tick {50}}};
    std::vector<SightRead::Solo> solos {
        {SightRead::Tick {0}, SightRead::Tick {50}, 50}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.solos(solos);
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_gh1_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 2, points.cbegin() + 3,
                 SightRead::Beat {0.0}, SightRead::Beat {0.0}}},
               100};

    const char* desired_path_output = "Path: 2-S1-ES1\n"
                                      "No SP score: 300\n"
                                      "Total score: 400\n"
                                      "2: NN (G)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(no_overlap_is_denoted_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(576),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {6144}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 3, points.cbegin() + 3,
                 SightRead::Beat {0.0}, SightRead::Beat {0.0}}},
               50};

    const char* desired_path_output = "Path: 3-ES1\n"
                                      "No SP score: 250\n"
                                      "Total score: 300\n"
                                      "Average multiplier: 1.200x\n"
                                      "3: NN (G)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(no_es_is_denoted_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(576),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {6144}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 4, points.cbegin() + 4,
                 SightRead::Beat {0.0}, SightRead::Beat {0.0}}},
               50};

    const char* desired_path_output = "Path: 3(+1)\n"
                                      "No SP score: 250\n"
                                      "Total score: 300\n"
                                      "Average multiplier: 1.200x\n"
                                      "3(+1): 2nd G (G)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(no_sp_is_denoted_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(576),
                                        make_note(6144)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}},
        {SightRead::Tick {384}, SightRead::Tick {50}},
        {SightRead::Tick {6144}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Path path {{}, 0};

    const char* desired_path_output = "Path: None\n"
                                      "No SP score: 250\n"
                                      "Total score: 250\n"
                                      "Average multiplier: 1.000x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(sustains_handled_correctly_for_nn)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192, 192),
                                        make_note(768)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 1, points.cend() - 1, SightRead::Beat {0.0},
                 SightRead::Beat {0.0}}},
               50};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 178\n"
                                      "Total score: 228\n"
                                      "Average multiplier: 1.302x\n"
                                      "2: NN (G)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(mid_sustain_activations_noted_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(768, 192)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 3, points.cend() - 1, SightRead::Beat {0.0},
                 SightRead::Beat {0.0}}},
               28};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 178\n"
                                      "Total score: 206\n"
                                      "Average multiplier: 1.177x\n"
                                      "2: 0.03 beats after NN";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(notes_of_different_colours_are_counted_correctly)
{
    std::vector<SightRead::Note> notes {
        make_note(0), make_note(192), make_note(768),
        make_note(960, 0, SightRead::FIVE_FRET_RED)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 1, points.cend() - 1, SightRead::Beat {0.0},
                 SightRead::Beat {0.0}}},
               50};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 200\n"
                                      "Total score: 250\n"
                                      "Average multiplier: 1.250x\n"
                                      "2: 1st R (R)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(
    note_counting_is_done_correctly_when_intermediate_sustains_exist)
{
    std::vector<SightRead::Note> notes {
        make_note(0), make_note(192), make_note(768, 96),
        make_note(960, 0, SightRead::FIVE_FRET_RED)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 1, points.cend() - 1, SightRead::Beat {0.0},
                 SightRead::Beat {0.0}}},
               50};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 214\n"
                                      "Total score: 264\n"
                                      "Average multiplier: 1.239x\n"
                                      "2: 1st R (R)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(mid_sustain_act_before_notes_are_written_correctly)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192, 192)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {50}},
        {SightRead::Tick {192}, SightRead::Tick {50}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 2, points.cend() - 1, SightRead::Beat {0.0},
                 SightRead::Beat {0.0}}},
               28};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 128\n"
                                      "Total score: 156\n"
                                      "Average multiplier: 1.248x\n"
                                      "2: After 0.03 beats";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(zero_phrase_acts_are_handled)
{
    std::vector<SightRead::Note> notes {make_note(0, 3072), make_note(3264)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {3300}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 3, points.cend() - 3, SightRead::Beat {0.0},
                 SightRead::Beat {0.0}}},
               1};

    const char* desired_path_output = "Path: 0-ES1\n"
                                      "No SP score: 539\n"
                                      "Total score: 540\n"
                                      "Average multiplier: 1.080x\n"
                                      "0: See image";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(average_multiplier_rounds_down)
{
    std::vector<SightRead::Note> notes;
    notes.reserve(11);
    for (auto i = 0; i < 11; ++i) {
        notes.push_back(make_note(i));
    }
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_guitar_pathing_settings(),
                         {},
                         {}};
    Path path {{}, 0};

    const char* desired_path_output = "Path: None\n"
                                      "No SP score: 650\n"
                                      "Total score: 650\n"
                                      "Average multiplier: 1.181x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(average_multiplier_is_correct_for_drums)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0, SightRead::DRUM_RED),
        make_drum_note(192, SightRead::DRUM_DOUBLE_KICK)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_pro_drums_pathing_settings(),
                         {},
                         {}};
    Path path {{}, 0};

    const char* desired_path_output = "Path: None\n"
                                      "No SP score: 50\n"
                                      "Total score: 50\n"
                                      "Average multiplier: 1.000x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(average_multiplier_is_correct_for_zero_notes)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(192, SightRead::DRUM_DOUBLE_KICK)};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_pro_drums_pathing_settings(),
                         {},
                         {}};
    Path path {{}, 0};

    const char* desired_path_output = "Path: None\n"
                                      "No SP score: 0\n"
                                      "Total score: 0\n"
                                      "Average multiplier: 0.000x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(alternative_path_notation_is_used_for_drums)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0),    make_drum_note(192),  make_drum_note(1536),
        make_drum_note(6336), make_drum_note(6528), make_drum_note(6912),
        make_drum_note(9984), make_drum_note(13056)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {192}, SightRead::Tick {1}},
        {SightRead::Tick {6336}, SightRead::Tick {1}},
        {SightRead::Tick {6528}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.generate_drum_fills({});
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_pro_drums_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 2, points.cbegin() + 2,
                 SightRead::Beat {8.0}, SightRead::Beat {24.0}},
                {points.cend() - 1, points.cend() - 1, SightRead::Beat {68.0},
                 SightRead::Beat {84.0}}},
               100};

    const char* desired_path_output = "Path: 0-1\n"
                                      "No SP score: 400\n"
                                      "Total score: 500\n"
                                      "Average multiplier: 1.250x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(alternative_path_notation_l_and_e_are_used_for_drums)
{
    std::vector<SightRead::Note> notes {
        make_drum_note(0),    make_drum_note(390),  make_drum_note(1536),
        make_drum_note(5568), make_drum_note(5755), make_drum_note(6912),
        make_drum_note(9984), make_drum_note(13056)};
    std::vector<SightRead::StarPower> phrases {
        {SightRead::Tick {0}, SightRead::Tick {1}},
        {SightRead::Tick {390}, SightRead::Tick {1}},
        {SightRead::Tick {5568}, SightRead::Tick {1}},
        {SightRead::Tick {5755}, SightRead::Tick {1}}};
    SightRead::NoteTrack note_track {
        notes, phrases, SightRead::TrackType::Drums,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.generate_drum_fills({});
    ProcessedSong track {note_track,
                         {{}, SpMode::Measure},
                         default_pro_drums_pathing_settings(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cbegin() + 2, points.cbegin() + 2,
                 SightRead::Beat {8.0}, SightRead::Beat {24.0}},
                {points.cend() - 1, points.cend() - 1, SightRead::Beat {68.0},
                 SightRead::Beat {84.0}}},
               100};

    const char* desired_path_output = "Path: 0(E)-1(L)\n"
                                      "No SP score: 400\n"
                                      "Total score: 500\n"
                                      "Average multiplier: 1.250x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(average_multiplier_is_ignored_with_rb)
{
    std::vector<SightRead::Note> notes {make_note(0), make_note(192),
                                        make_note(384), make_note(576),
                                        make_note(6144)};
    std::vector<SightRead::Solo> solos {
        {SightRead::Tick {0}, SightRead::Tick {50}, 100}};
    SightRead::NoteTrack note_track {
        notes,
        {},
        SightRead::TrackType::FiveFret,
        std::make_shared<SightRead::SongGlobalData>()};
    note_track.solos(solos);
    ProcessedSong track {note_track,
                         {{}, SpMode::OdBeat},
                         default_rb_pathing_settings(),
                         {},
                         {}};
    Path path {{}, 0};

    const char* desired_path_output = "Path: None\n"
                                      "No SP score: 225\n"
                                      "Total score: 225";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_SUITE_END()
