/*
 * chopt - Star Power optimiser for Clone Hero
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
TEST_CASE("propagate_sp_over_whammy works correctly", "Whammy SP")
{
    std::vector<Note> notes {{0, 1920}, {2112, 576}, {3000}};
    std::vector<StarPower> phrases {{0, 3000}};
    NoteTrack track(notes, phrases);

    SECTION("Works correctly over 4/4")
    {
        std::vector<TimeSignature> time_sigs {{0, 4, 4}};
        SpData sp_data(track, 192, SyncTrack(time_sigs));

        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(1.0),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.508333));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(1.0), Beat(4.0),
                                              Measure(0.25), Measure(1.0),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.50625));
    }

    SECTION("Works correctly over 3/4")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}};
        SpData sp_data(track, 192, SyncTrack(time_sigs));

        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(4.0 / 3),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.466667));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(-1.0), Beat(4.0),
                                              Measure(-0.25), Measure(4.0 / 3),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.435417));
    }

    SECTION("Works correctly over changing time signatures")
    {
        std::vector<TimeSignature> time_sigs {{0, 4, 4}, {384, 3, 4}};
        SpData sp_data(track, 192, SyncTrack(time_sigs));

        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(7.0 / 6),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.4875));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(1.0), Beat(4.0),
                                              Measure(0.25), Measure(7.0 / 6),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.485417));
    }

    SECTION("Returns -1 if SP runs out")
    {
        std::vector<TimeSignature> time_sigs {{0, 3, 4}, {384, 4, 4}};
        SpData sp_data(track, 192, SyncTrack(time_sigs));

        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(2.0),
                                              Measure(0.0), Measure(2.0 / 3),
                                              {0.015, 0.015})
                    .max()
                == Approx(-1.0));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(10.0),
                                              Measure(0.0), Measure(8.0 / 3),
                                              {0.015, 0.015})
                    .max()
                == Approx(-1.0));
    }

    SECTION("Works even if some of the range isn't whammyable")
    {
        SpData sp_data(track, 192, SyncTrack());

        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(12.0),
                                              Measure(0.0), Measure(3.0),
                                              {0.5, 0.5})
                    .max()
                == Approx(0.491667));
    }

    SECTION("SP bar does not exceed full bar")
    {
        SpData sp_data(track, 192, SyncTrack());

        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(10.0),
                                              Measure(0.0), Measure(2.5),
                                              {1.0, 1.0})
                    .max()
                == Approx(1.0));
        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(10.5),
                                              Measure(0.0), Measure(2.625),
                                              {1.0, 1.0})
                    .max()
                == Approx(0.984375));
    }

    SECTION("Hold notes not in a phrase do not contribute SP")
    {
        NoteTrack no_sp_note_track(notes, {});
        SpData sp_data(no_sp_note_track, 192, SyncTrack());

        REQUIRE(sp_data
                    .propagate_sp_over_whammy(Beat(0.0), Beat(4.0),
                                              Measure(0.0), Measure(1.0),
                                              {1.0, 1.0})
                    .max()
                == Approx(0.875));
    }
}

TEST_CASE("is_in_whammy_ranges works correctly", "Whammy ranges")
{
    std::vector<Note> notes {{0, 1920}, {2112}};
    std::vector<StarPower> phrases {{0, 2000}, {2112, 50}};
    NoteTrack track(notes, phrases);
    SpData sp_data(track, 192, SyncTrack());

    REQUIRE(sp_data.is_in_whammy_ranges(Beat(1.0)));
    REQUIRE(!sp_data.is_in_whammy_ranges(Beat(11.0)));
}
