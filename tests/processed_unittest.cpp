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

#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "processed.hpp"

static bool operator==(const SpBar& lhs, const SpBar& rhs)
{
    return std::abs(lhs.min() - rhs.min()) < 0.000001
        && std::abs(lhs.max() - rhs.max()) < 0.000001;
}

static std::ostream& operator<<(std::ostream& stream, const SpBar& sp)
{
    stream << "{Min " << sp.min() << ", Max " << sp.max() << '}';
    return stream;
}

static std::ostream& operator<<(std::ostream& stream, ActValidity validity)
{
    stream << static_cast<int>(validity);
    return stream;
}

BOOST_AUTO_TEST_SUITE(three_arg_total_available_sp_counts_sp_correctly)

BOOST_AUTO_TEST_CASE(phrases_are_counted_correctly)
{
    std::vector<Note<NoteColour>> notes {{0},        {192},  {384},  {576},
                                         {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(song.total_available_sp(Beat(0.0), points.cbegin(),
                                              points.cbegin() + 1),
                      SpBar(0.25, 0.25));
    BOOST_CHECK_EQUAL(song.total_available_sp(Beat(0.0), points.cbegin(),
                                              points.cbegin() + 2),
                      SpBar(0.25, 0.25));
    BOOST_CHECK_EQUAL(song.total_available_sp(Beat(0.5), points.cbegin() + 2,
                                              points.cbegin() + 3),
                      SpBar(0.25, 0.25));
}

BOOST_AUTO_TEST_CASE(whammy_is_counted_correctly)
{
    std::vector<Note<NoteColour>> notes {{0},        {192},  {384},  {576},
                                         {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();
    auto result = song.total_available_sp(Beat(4.0), points.cbegin() + 4,
                                          points.cbegin() + 5);

    BOOST_CHECK_CLOSE(result.min(), 0.0, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.001128472, 0.0001);
}

BOOST_AUTO_TEST_CASE(whammy_is_counted_correctly_even_started_mid_hold)
{
    std::vector<Note<NoteColour>> notes {{0},        {192},  {384},  {576},
                                         {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();
    auto result = song.total_available_sp(Beat(4.5), points.cend() - 3,
                                          points.cend() - 3);

    BOOST_CHECK_CLOSE(result.min(), 0.0, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.01666667, 0.0001);
}

BOOST_AUTO_TEST_CASE(required_whammy_end_is_accounted_for)
{
    std::vector<Note<NoteColour>> notes {{0},        {192},  {384},  {576},
                                         {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();
    auto result = song.total_available_sp(Beat(4.0), points.cbegin() + 4,
                                          points.cbegin() + 5, Beat(4.02));

    BOOST_CHECK_CLOSE(result.min(), 0.000666667, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.001128472, 0.0001);

    result = song.total_available_sp(Beat(4.0), points.cbegin() + 4,
                                     points.cbegin() + 5, Beat(4.10));

    BOOST_CHECK_CLOSE(result.min(), 0.001128472, 0.0001);
    BOOST_CHECK_CLOSE(result.max(), 0.001128472, 0.0001);
}

BOOST_AUTO_TEST_CASE(sp_does_not_exceed_full_bar)
{
    std::vector<Note<NoteColour>> notes {{0},        {192},  {384},  {576},
                                         {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(
        song.total_available_sp(Beat(0.0), points.cbegin(), points.cend() - 1),
        SpBar(1.0, 1.0));
}

BOOST_AUTO_TEST_CASE(
    sp_notes_are_counted_from_first_point_when_start_is_past_middle)
{
    std::vector<Note<NoteColour>> notes {{0},        {192},  {384},  {576},
                                         {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(song.total_available_sp(Beat(0.05), points.cbegin(),
                                              points.cbegin() + 1),
                      SpBar(0.25, 0.25));
}

BOOST_AUTO_TEST_CASE(unison_bonuses_are_taken_account_of)
{
    std::vector<Note<NoteColour>> notes {{0},        {192},  {384},  {576},
                                         {768, 192}, {1152}, {1344}, {1536}};
    std::vector<StarPower> phrases {{0, 50}, {384, 50}, {768, 400}, {1344, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        Rb3Engine(),
                        {},
                        {{0, 50}}};
    const auto& points = song.points();

    BOOST_CHECK_EQUAL(song.total_available_sp(Beat(0.0), points.cbegin(),
                                              points.cbegin() + 1),
                      SpBar(0.5, 0.5));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(
    total_available_sp_with_earliest_pos_counts_sp_correctly_and_gives_earliest_posiiton)
{
    std::vector<Note<NoteColour>> notes {{0, 1459}, {1459}};
    std::vector<StarPower> phrases {{0, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();

    const auto& [sp_bar, pos] = song.total_available_sp_with_earliest_pos(
        Beat(0.0), points.cbegin(), std::prev(points.cend()),
        std::prev(points.cend(), 2)->position);

    BOOST_CHECK_CLOSE(sp_bar.max(), 0.5, 0.001);
    BOOST_CHECK_CLOSE(pos.beat.value(), 7.5, 0.001);
}

BOOST_AUTO_TEST_CASE(total_available_sp_with_earliest_pos_counts_unison_bonuses)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}};
    std::vector<StarPower> phrases {{0, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {note_track,
                        {},
                        SqueezeSettings::default_settings(),
                        DrumSettings::default_settings(),
                        Rb3Engine(),
                        {},
                        {0}};
    const auto& points = song.points();

    const auto& [sp_bar, pos] = song.total_available_sp_with_earliest_pos(
        Beat(0.0), points.cbegin(), std::next(points.cbegin()),
        {Beat(0.0), Measure(0.0)});

    BOOST_CHECK_CLOSE(sp_bar.max(), 0.5, 0.0001);
}

BOOST_AUTO_TEST_SUITE(is_candidate_valid_works_with_no_whammy)

BOOST_AUTO_TEST_CASE(full_bar_works_with_time_signatures)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {Beat(0.0), Measure(0.0)},
                                   {1.0, 1.0}};
    ProcessedSong second_track {note_track,
                                SyncTrack({{0, 3, 4}}, {}),
                                SqueezeSettings::default_settings(),
                                DrumSettings::default_settings(),
                                ChGuitarEngine(),
                                {},
                                {}};
    const auto& second_points = second_track.points();
    ActivationCandidate second_candidate {second_points.cbegin(),
                                          second_points.cbegin() + 3,
                                          {Beat(0.0), Measure(0.0)},
                                          {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
    BOOST_CHECK_EQUAL(
        second_track.is_candidate_valid(second_candidate).validity,
        ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(half_bar_works_with_time_signatures)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};
    ProcessedSong second_track {note_track,
                                SyncTrack({{0, 3, 4}}, {}),
                                SqueezeSettings::default_settings(),
                                DrumSettings::default_settings(),
                                ChGuitarEngine(),
                                {},
                                {}};
    const auto& second_points = second_track.points();
    ActivationCandidate second_candidate {second_points.cbegin(),
                                          second_points.cbegin() + 2,
                                          {Beat(0.0), Measure(0.0)},
                                          {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
    BOOST_CHECK_EQUAL(
        second_track.is_candidate_valid(second_candidate).validity,
        ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(below_half_bar_never_works)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.25, 0.25}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(check_next_point_needs_to_not_lie_in_activation)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.6, 0.6}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_CASE(check_intermediate_sp_is_accounted_for)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {3072}, {6144}};
    std::vector<StarPower> phrases {{3000, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.8, 0.8}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(check_only_reached_intermediate_sp_is_accounted_for)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {6000}, {6144}};
    std::vector<StarPower> phrases {{6000, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.8, 0.8}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(last_notes_sp_status_is_not_ignored)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {3072}, {4000}};
    std::vector<StarPower> phrases {{3072, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_CASE(sp_bar_does_not_exceed_full_bar)
{
    std::vector<Note<NoteColour>> notes {{0}, {2}, {7000}};
    std::vector<StarPower> phrases {{0, 1}, {2, 1}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(earliest_activation_point_is_considered)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {3072}, {6144}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(-2.0), Measure(-0.5)},
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
    std::vector<Note<NoteColour>> notes {
        {384}, {384, 0, NoteColour::Red}, {5088}, {5136}};
    std::vector<StarPower> phrases {{384, 1}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    SyncTrack sync_track {{{0, 4, 4}}, {{0, 300000}}};
    ProcessedSong track {note_track,
                         sync_track,
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(2.24), Measure(0.56)},
                                   {0.5, 0.5}};

    const auto result
        = track.is_candidate_valid(candidate, 1.0, {Beat {27}, Measure {6.75}});

    BOOST_CHECK_EQUAL(result.validity, ActValidity::success);
    BOOST_CHECK_LT(result.ending_position.beat.value(), 26.5);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_acknowledges_unison_bonuses)

BOOST_AUTO_TEST_CASE(mid_activation_unison_bonuses_are_accounted_for)
{
    std::vector<Note<NoteColour>> notes {{192}, {5376}};
    std::vector<StarPower> phrases {{192, 1}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         Rb3Engine(),
                         {},
                         {192}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    const auto result = track.is_candidate_valid(candidate);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::success);
}

BOOST_AUTO_TEST_CASE(last_note_unison_bonus_accounted_for_excess_sp)
{
    std::vector<Note<NoteColour>> notes {{192}, {5376}};
    std::vector<StarPower> phrases {{192, 1}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         Rb3Engine(),
                         {},
                         {192}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin(),
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    const auto result = track.is_candidate_valid(candidate);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_works_with_whammy)

BOOST_AUTO_TEST_CASE(check_whammy_is_counted)
{
    std::vector<Note<NoteColour>> notes {{0, 960}, {3840}, {6144}};
    std::vector<StarPower> phrases {{0, 7000}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

// This comes up in Epidox: otherwise, CHOpt doesn't think you can squeeze to
// the G after the B in the Robotic Buildup activation.
BOOST_AUTO_TEST_CASE(check_whammy_from_end_of_sp_sustain_before_note_is_counted)
{
    std::vector<Note<NoteColour>> notes {{0, 960}, {2880}, {6144}};
    std::vector<StarPower> phrases {{0, 7000}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cend() - 2,
                                   points.cend() - 1,
                                   {Beat(1.0), Measure(0.25)},
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
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384, 192}, {5260}};
    std::vector<StarPower> phrases {{384, 1}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 2,
                                   points.cend() - 1,
                                   {Beat(1.0), Measure(0.25)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_CASE(check_compressed_activations_are_counted)
{
    std::vector<Note<NoteColour>> notes {{0, 960}, {3840}, {6144}};
    std::vector<StarPower> phrases {{0, 7000}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.9}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_minimum_sp)

BOOST_AUTO_TEST_CASE(lower_sp_is_considered)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {2304}, {3072}, {4608}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 3,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 1.0}};
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(lower_sp_is_only_considered_down_to_a_half_bar)
{
    std::vector<Note<NoteColour>> notes {{0}, {1536}, {2304}, {3072}, {4608}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.25, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_squeezing)

BOOST_AUTO_TEST_CASE(
    front_end_and_back_end_of_the_activation_endpoints_are_considered)
{
    std::vector<Note<NoteColour>> notes {{0}, {3110}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(next_note_can_be_squeezed_late_to_avoid_going_too_far)
{
    std::vector<Note<NoteColour>> notes {{0}, {3034}, {3053}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(intermediate_sp_can_be_hit_early)
{
    std::vector<Note<NoteColour>> notes {{0}, {3102}, {4608}};
    std::vector<StarPower> phrases {{3100, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(intermediate_sp_can_be_hit_late)
{
    std::vector<Note<NoteColour>> notes {{0}, {768}, {6942}};
    std::vector<StarPower> phrases {{768, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(is_candidate_valid_handles_very_high_bpm_sp_granting_notes)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {768}, {4608}, {5376}};
    std::vector<StarPower> phrases {{4608, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    SyncTrack sync_track {{}, {{3840, 4000000}}};
    ProcessedSong track {note_track,
                         sync_track,
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 2,
                                   points.cbegin() + 4,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_squeeze_param)

BOOST_AUTO_TEST_CASE(front_end_and_back_end_are_restricted)
{
    std::vector<Note<NoteColour>> notes {{0}, {3110}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::insufficient_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(Intermediate_sp_front_end_is_restricted)
{
    std::vector<Note<NoteColour>> notes {{0}, {3102}, {4608}};
    std::vector<StarPower> phrases {{3100, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::insufficient_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(intermediate_sp_back_end_is_restricted)
{
    std::vector<Note<NoteColour>> notes {{0}, {768}, {6942}};
    std::vector<StarPower> phrases {{768, 100}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {1.0, 1.0}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::insufficient_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(next_note_back_end_is_restricted)
{
    std::vector<Note<NoteColour>> notes {{0}, {3034}, {3053}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 0.5).validity,
                      ActValidity::surplus_sp);
    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(end_position_is_finite_if_activation_goes_past_last_note)
{
    std::vector<Note<NoteColour>> notes {{0}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin(),
                                   {Beat(0.0), Measure(0.0)},
                                   {1.0, 1.0}};
    auto result = track.is_candidate_valid(candidate, 1.0);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::success);
    BOOST_CHECK_LT(result.ending_position.beat.value(), 40.0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_candidate_valid_takes_into_account_no_overlap)

BOOST_AUTO_TEST_CASE(mid_act_phrases_not_collected)
{
    std::vector<Note<NoteColour>> notes {{0}, {2688}, {3072}, {3840}};
    std::vector<StarPower> phrases {{2688, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         Gh1Engine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_CASE(end_of_act_phrase_not_collected)
{
    std::vector<Note<NoteColour>> notes {{0}, {3072}, {3840}};
    std::vector<StarPower> phrases {{3072, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         Gh1Engine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(track.is_candidate_valid(candidate, 1.0).validity,
                      ActValidity::success);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(is_candidate_valid_takes_into_account_forced_whammy)
{
    std::vector<Note<NoteColour>> notes {{0, 768}, {3072}, {3264}};
    std::vector<StarPower> phrases {{0, 3300}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cend() - 2,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    BOOST_CHECK_EQUAL(
        track.is_candidate_valid(candidate, 1.0, {Beat(0.0), Measure(0.0)})
            .validity,
        ActValidity::success);
    BOOST_CHECK_EQUAL(
        track.is_candidate_valid(candidate, 1.0, {Beat(4.0), Measure(1.0)})
            .validity,
        ActValidity::surplus_sp);
}

BOOST_AUTO_TEST_CASE(
    is_candidate_valid_also_takes_account_of_whammy_from_end_of_sp_sustain_before_note_is_counted)
{
    std::vector<Note<NoteColour>> notes {{0, 960}, {2880}, {6144}};
    std::vector<StarPower> phrases {{0, 7000}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cend() - 2,
                                   points.cend() - 1,
                                   {Beat(1.0), Measure(0.25)},
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
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {3456, 192}};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}, {3456, 1}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 2,
                                   points.cend() - 1,
                                   {Beat(1.0), Measure(0.25)},
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
    std::vector<Note<NoteColour>> notes {{0, 6720}};
    std::vector<StarPower> phrases {{0, 1}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    SyncTrack sync_track {{{0, 1, 4}}, {}};
    ProcessedSong track {note_track,
                         sync_track,
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    ActivationCandidate candidate {points.cbegin() + 500,
                                   points.cbegin() + 750,
                                   {Beat(0.0), Measure(0.0)},
                                   {1.0, 1.0}};

    auto result = track.is_candidate_valid(candidate, 0.0);

    BOOST_CHECK_GT(result.ending_position.beat.value(), 27.3);
}

BOOST_AUTO_TEST_CASE(adjusted_hit_window_functions_return_correct_values)
{
    std::vector<Note<NoteColour>> notes {{0}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
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
    std::vector<Note<NoteColour>> notes {{192}, {384, 192}, {768}};
    std::vector<StarPower> phrases {{384, 1}};
    NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong song {track,
                        {},
                        {0.0, 0.0, Second {0.0}, Second {-0.1}, Second {0.0}},
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();

    auto result = song.total_available_sp(Beat(0.0), points.cbegin(),
                                          std::prev(points.cend()));

    BOOST_CHECK_CLOSE(result.max(), 0.29, 0.0001);
}

BOOST_AUTO_TEST_CASE(effect_on_notes_is_taken_account_of)
{
    std::vector<Note<NoteColour>> notes {{768}, {3840}};
    NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
    SyncTrack sync_track {{{0, 4, 4}, {3840, 2, 4}}, {}};
    ProcessedSong song {track,
                        sync_track,
                        {0.0, 0.0, Second {0.0}, Second {0.1}, Second {0.0}},
                        DrumSettings::default_settings(),
                        ChGuitarEngine(),
                        {},
                        {}};
    const auto& points = song.points();
    ActivationCandidate candidate {points.cbegin(),
                                   points.cbegin() + 1,
                                   {Beat(0.0), Measure(0.0)},
                                   {0.5, 0.5}};

    auto result = song.is_candidate_valid(candidate);

    BOOST_CHECK_EQUAL(result.validity, ActValidity::insufficient_sp);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(is_drums_returns_the_correct_value)

BOOST_AUTO_TEST_CASE(false_for_guitar)
{
    NoteTrack<NoteColour> note_track {{}, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChDrumEngine(),
                         {},
                         {}};

    BOOST_CHECK(!track.is_drums());
}

BOOST_AUTO_TEST_CASE(true_for_drums)
{
    NoteTrack<DrumNoteColour> note_track {{}, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChDrumEngine(),
                         {},
                         {}};

    BOOST_TEST(track.is_drums());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(path_summary_produces_the_correct_output)

BOOST_AUTO_TEST_CASE(overlap_and_es_are_denoted_correctly)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {576}, {6144}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {384, 50}, {6144, 50}};
    std::vector<Solo> solos {{0, 50, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, solos, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {
        {{points.cbegin() + 2, points.cbegin() + 3, Beat {0.0}, Beat {0.0}}},
        100};

    const char* desired_path_output = "Path: 2(+1)-ES1\n"
                                      "No SP score: 300\n"
                                      "Total score: 400\n"
                                      "Average multiplier: 1.400x\n"
                                      "2(+1): NN (G)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(no_overlap_is_denoted_correctly)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {576}, {6144}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {384, 50}, {6144, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {
        {{points.cbegin() + 3, points.cbegin() + 3, Beat {0.0}, Beat {0.0}}},
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
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {576}, {6144}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {384, 50}, {6144, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {
        {{points.cbegin() + 4, points.cbegin() + 4, Beat {0.0}, Beat {0.0}}},
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
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {576}, {6144}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}, {384, 50}, {6144, 50}};
    NoteTrack<NoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
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
    std::vector<Note<NoteColour>> notes {{0}, {192, 192}, {768}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 1, points.cend() - 1, Beat {0.0}, Beat {0.0}}},
               50};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 178\n"
                                      "Total score: 228\n"
                                      "Average multiplier: 1.303x\n"
                                      "2: NN (G)";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(mid_sustain_activations_noted_correctly)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {768, 192}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {
        {{points.cbegin() + 3, points.cend() - 1, Beat {0.0}, Beat {0.0}}}, 28};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 178\n"
                                      "Total score: 206\n"
                                      "Average multiplier: 1.177x\n"
                                      "2: 0.03 beats after NN";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(notes_of_different_colours_are_counted_correctly)
{
    std::vector<Note<NoteColour>> notes {
        {0}, {192}, {768}, {960, 0, NoteColour::Red}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 1, points.cend() - 1, Beat {0.0}, Beat {0.0}}},
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
    std::vector<Note<NoteColour>> notes {
        {0}, {192}, {768, 96}, {960, 0, NoteColour::Red}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 1, points.cend() - 1, Beat {0.0}, Beat {0.0}}},
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
    std::vector<Note<NoteColour>> notes {{0}, {192, 192}};
    std::vector<StarPower> phrases {{0, 50}, {192, 50}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {
        {{points.cbegin() + 2, points.cend() - 1, Beat {0.0}, Beat {0.0}}}, 28};

    const char* desired_path_output = "Path: 2\n"
                                      "No SP score: 128\n"
                                      "Total score: 156\n"
                                      "Average multiplier: 1.248x\n"
                                      "2: After 0.03 beats";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(zero_phrase_acts_are_handled)
{
    std::vector<Note<NoteColour>> notes {{0, 3072}, {3264}};
    std::vector<StarPower> phrases {{0, 3300}};
    NoteTrack<NoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         ChGuitarEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {{{points.cend() - 3, points.cend() - 3, Beat {0.0}, Beat {0.0}}},
               1};

    const char* desired_path_output = "Path: 0-ES1\n"
                                      "No SP score: 539\n"
                                      "Total score: 540\n"
                                      "Average multiplier: 1.080x\n"
                                      "0: See image";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(average_multiplier_is_correct_for_drums)
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red}, {192, 0, DrumNoteColour::DoubleKick}};
    NoteTrack<DrumNoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         {false, false, true, false},
                         ChDrumEngine(),
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
    std::vector<Note<DrumNoteColour>> notes {
        {192, 0, DrumNoteColour::DoubleKick}};
    NoteTrack<DrumNoteColour> note_track {notes, {}, {}, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         {false, false, true, false},
                         ChDrumEngine(),
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
    std::vector<Note<DrumNoteColour>> notes {{0},    {192},  {1536}, {6336},
                                             {6528}, {6912}, {9984}, {13056}};
    std::vector<StarPower> phrases {{0, 1}, {192, 1}, {6336, 1}, {6528, 1}};
    NoteTrack<DrumNoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    note_track.generate_drum_fills({{}, 192, ChDrumEngine(), {}});
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         {false, false, true, false},
                         ChDrumEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {
        {{points.cbegin() + 2, points.cbegin() + 2, Beat {8.0}, Beat {24.0}},
         {points.cend() - 1, points.cend() - 1, Beat {68.0}, Beat {84.0}}},
        100};

    const char* desired_path_output = "Path: 0-1\n"
                                      "No SP score: 400\n"
                                      "Total score: 500\n"
                                      "Average multiplier: 1.250x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(alternative_path_notation_l_and_e_are_used_for_drums)
{
    std::vector<Note<DrumNoteColour>> notes {{0},    {390},  {1536}, {5568},
                                             {5755}, {6912}, {9984}, {13056}};
    std::vector<StarPower> phrases {{0, 1}, {390, 1}, {5568, 1}, {5755, 1}};
    NoteTrack<DrumNoteColour> note_track {notes, phrases, {}, {}, {}, {}, 192};
    note_track.generate_drum_fills({{}, 192, ChDrumEngine(), {}});
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         {false, false, true, false},
                         ChDrumEngine(),
                         {},
                         {}};
    const auto& points = track.points();
    Path path {
        {{points.cbegin() + 2, points.cbegin() + 2, Beat {8.0}, Beat {24.0}},
         {points.cend() - 1, points.cend() - 1, Beat {68.0}, Beat {84.0}}},
        100};

    const char* desired_path_output = "Path: 0(E)-1(L)\n"
                                      "No SP score: 400\n"
                                      "Total score: 500\n"
                                      "Average multiplier: 1.250x";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_CASE(average_multiplier_is_ignored_with_rb)
{
    std::vector<Note<NoteColour>> notes {{0}, {192}, {384}, {576}, {6144}};
    std::vector<Solo> solos {{0, 50, 100}};
    NoteTrack<NoteColour> note_track {notes, {}, solos, {}, {}, {}, 192};
    ProcessedSong track {note_track,
                         {},
                         SqueezeSettings::default_settings(),
                         DrumSettings::default_settings(),
                         RbEngine(),
                         {},
                         {}};
    Path path {{}, 0};

    const char* desired_path_output = "Path: None\n"
                                      "No SP score: 225\n"
                                      "Total score: 225";

    BOOST_CHECK_EQUAL(track.path_summary(path), desired_path_output);
}

BOOST_AUTO_TEST_SUITE_END()
