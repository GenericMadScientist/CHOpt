/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020 Raymond Wright
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

#include "catch.hpp"

#include "sp.hpp"

TEST_CASE("SpBar methods", "SpBar")
{
    SECTION("add_phrase() works correctly")
    {
        SpBar sp_bar {0.0, 0.25};
        sp_bar.add_phrase();

        REQUIRE(sp_bar == SpBar {0.25, 0.5});

        sp_bar = {0.8, 1.0};
        sp_bar.add_phrase();

        REQUIRE(sp_bar == SpBar {1.0, 1.0});
    }

    SECTION("full_enough_to_activate() works correctly")
    {
        SpBar sp_bar {0.49, 0.49};

        REQUIRE(!sp_bar.full_enough_to_activate());

        sp_bar = {0.0, 0.5};

        REQUIRE(sp_bar.full_enough_to_activate());
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("propagate_sp_over_whammy works correctly")
{
    std::vector<Note<NoteColour>> notes {{0, 1920}, {2112, 576}, {3000}};
    std::vector<StarPower> phrases {{0, 3000}};
    NoteTrack<NoteColour> track {notes, phrases, {}};

    SECTION("Works correctly over 4/4")
    {
        std::vector<TimeSignature> time_sigs {{0, 4, 4}};
        SpData sp_data {track, 192, {time_sigs, {}}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(4.0), Measure(1.0)}, 0.5)
                    .max()
                == Approx(0.508333));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(1.0), Measure(0.25)},
                                              {Beat(4.0), Measure(1.0)}, 0.5)
                    .max()
                == Approx(0.50625));
    }

    SECTION("Works correctly over 3/4")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}};
        SpData sp_data {track, 192, {time_sigs, {}}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(4.0), Measure(4.0 / 3)},
                                              0.5)
                    .max()
                == Approx(0.466667));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(-1.0), Measure(-0.25)},
                                              {Beat(4.0), Measure(4.0 / 3)},
                                              0.5)
                    .max()
                == Approx(0.440083));
    }

    SECTION("Works correctly over changing time signatures")
    {
        std::vector<TimeSignature> time_sigs {{0, 4, 4}, {384, 3, 4}};
        SpData sp_data {track, 192, {time_sigs, {}}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(4.0), Measure(7.0 / 6)},
                                              0.5)
                    .max()
                == Approx(0.4875));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(1.0), Measure(0.25)},
                                              {Beat(4.0), Measure(7.0 / 6)},
                                              0.5)
                    .max()
                == Approx(0.485417));
    }

    SECTION("Returns -1 if SP runs out")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}, {384, 4, 4}};
        SpData sp_data {track, 192, {time_sigs, {}}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(2.0), Measure(2.0 / 3)},
                                              0.015)
                    .max()
                == Approx(-1.0));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(10.0), Measure(8.0 / 3)},
                                              0.015)
                    .max()
                == Approx(-1.0));
    }

    SECTION("Works even if some of the range isn't whammyable")
    {
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(12.0), Measure(3.0)}, 0.5)
                    .max()
                == Approx(0.496333));
    }

    SECTION("SP bar does not exceed full bar")
    {
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(10.0), Measure(2.5)}, 1.0)
                    .max()
                == Approx(1.0));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(10.5), Measure(2.625)}, 1.0)
                    .max()
                == Approx(0.984375));
    }

    SECTION("Hold notes not in a phrase do not contribute SP")
    {
        NoteTrack<NoteColour> no_sp_note_track {notes, {}, {}};
        SpData sp_data {no_sp_note_track, 192, {}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(4.0), Measure(1.0)}, 1.0)
                    .max()
                == Approx(0.875));
    }

    SECTION("required_whammy_end is accounted for")
    {
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(4.0), Measure(1.0)}, 0.5,
                                              {Beat(2.0), Measure(0.5)})
                    .min()
                == Approx(0.441667));
    }

    SECTION("Check optional whammy is not used when not asked for in minimum")
    {
        std::vector<Note<NoteColour>> second_notes {{0, 768}, {3072}};
        std::vector<StarPower> second_phrases {{0, 3100}};
        NoteTrack<NoteColour> second_track {second_notes, second_phrases, {}};
        SpData sp_data {second_track, 192, {}, 1.0, Second(0.0)};

        REQUIRE(sp_data
                    .propagate_sp_over_whammy({Beat(0.0), Measure(0.0)},
                                              {Beat(4.0), Measure(1.0)}, 0.5)
                    .min()
                == Approx(0.375));
    }
}

TEST_CASE("is_in_whammy_ranges works correctly", "Whammy ranges")
{
    std::vector<Note<NoteColour>> notes {{0, 1920}, {2112}};
    std::vector<StarPower> phrases {{0, 2000}, {2112, 50}};
    NoteTrack<NoteColour> track {notes, phrases, {}};
    SpData sp_data {track, 192, {}, 1.0, Second(0.0)};

    REQUIRE(sp_data.is_in_whammy_ranges(Beat(1.0)));
    REQUIRE(!sp_data.is_in_whammy_ranges(Beat(11.0)));
}

TEST_CASE("available_whammy works correctly", "Available whammy")
{
    std::vector<Note<NoteColour>> notes {{0, 1920}, {2112}, {2304, 768}};
    std::vector<StarPower> phrases {{0, 3000}};
    NoteTrack<NoteColour> track {notes, phrases, {}};

    SECTION("100% early whammy")
    {
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};

        REQUIRE(sp_data.available_whammy(Beat(0.0), Beat(16.0))
                == Approx(0.471333));
        REQUIRE(sp_data.available_whammy(Beat(10.0), Beat(11.0))
                == Approx(0.0));
        REQUIRE(sp_data.available_whammy(Beat(1.0), Beat(8.0))
                == Approx(0.233333));
    }

    SECTION("50% early whammy")
    {
        SpData sp_data {track, 192, {}, 0.5, Second(0.0)};

        REQUIRE(sp_data.available_whammy(Beat(0.0), Beat(16.0))
                == Approx(0.469));
        REQUIRE(sp_data.available_whammy(Beat(10.0), Beat(11.0))
                == Approx(0.0));
        REQUIRE(sp_data.available_whammy(Beat(1.0), Beat(8.0))
                == Approx(0.233333));
    }

    SECTION("Negative early whammy")
    {
        SpData sp_data {track, 192, {}, 0.0, Second(2.5)};

        REQUIRE(sp_data.available_whammy(Beat(0.0), Beat(10.0))
                == Approx(0.166667));
        REQUIRE(sp_data.available_whammy(Beat(12.0), Beat(16.0))
                == Approx(0.0));
    }
}

TEST_CASE("activation_end_point works correctly")
{
    SECTION("Works when SP is sufficient")
    {
        std::vector<Note<NoteColour>> notes {{0}};
        NoteTrack<NoteColour> track {notes, {}, {}};
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};
        Position start {Beat(0.0), Measure(0.0)};
        Position end {Beat(1.0), Measure(0.25)};

        REQUIRE(sp_data.activation_end_point(start, end, 0.5).beat.value()
                == Approx(1.0));
    }

    SECTION("Works when SP is insufficient")
    {
        std::vector<Note<NoteColour>> notes {{0}};
        NoteTrack<NoteColour> track {notes, {}, {}};
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};
        Position start {Beat(0.0), Measure(0.0)};
        Position end {Beat(1.0), Measure(0.25)};

        REQUIRE(sp_data.activation_end_point(start, end, 0.01).beat.value()
                == Approx(0.32));
    }

    SECTION("Works when adding whammy makes SP sufficient")
    {
        std::vector<Note<NoteColour>> notes {{0, 192}, {950}};
        std::vector<StarPower> phrases {{0, 1000}};
        NoteTrack<NoteColour> track {notes, phrases, {}};
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};
        Position start {Beat(0.0), Measure(0.0)};
        Position end {Beat(1.0), Measure(0.25)};

        REQUIRE(sp_data.activation_end_point(start, end, 0.01).beat.value()
                == Approx(1.0));
    }

    SECTION("Works when whammy is present but insufficient")
    {
        std::vector<Note<NoteColour>> notes {{0, 192}, {950}};
        std::vector<StarPower> phrases {{0, 1000}};
        NoteTrack<NoteColour> track {notes, phrases, {}};
        SpData sp_data {track, 192, {}, 1.0, Second(0.0)};
        Position start {Beat(0.0), Measure(0.0)};
        Position end {Beat(2.0), Measure(0.5)};

        REQUIRE(sp_data.activation_end_point(start, end, 0.01).beat.value()
                == Approx(1.38667));
    }

    SECTION("Works when whammy is present but accumulation is too slow")
    {
        std::vector<Note<NoteColour>> notes {{0, 192}, {950}};
        std::vector<StarPower> phrases {{0, 1000}};
        SyncTrack sync_track {{{0, 2, 4}}, {}};
        NoteTrack<NoteColour> track {notes, phrases, {}};
        SpData sp_data {track, 192, sync_track, 1.0, Second(0.0)};
        Position start {Beat(0.0), Measure(0.0)};
        Position end {Beat(1.0), Measure(0.25)};

        REQUIRE(sp_data.activation_end_point(start, end, 0.01).beat.value()
                == Approx(0.342857));
    }
}
