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

#include <array>

#include "catch.hpp"

#include "chart.hpp"
#include "time.hpp"

// Last checked: 24.0.1555-master
TEST_CASE("Beats to seconds conversion", "Beats<->S")
{
    const auto track = SyncTrack({{0, 4, 4}}, {{0, 150000}, {800, 200000}});
    const auto header = SongHeader(0.F, 200);
    const auto converter = TimeConverter(track, header);
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.0};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 1.9};

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.beats_to_seconds({beats.at(i)})
                == Approx(seconds.at(i)));
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.seconds_to_beats(seconds.at(i)).value
                == Approx(beats.at(i)));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Beats to measures conversion", "Beats<->Measures")
{
    const auto track = SyncTrack({{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}}, {});
    const auto header = SongHeader(0.F, 200);
    const auto converter = TimeConverter(track, header);
    constexpr std::array beats {-1.0, 0.0, 3.0, 5.5, 6.5};
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.beats_to_measures({beats.at(i)})
                == Approx(measures.at(i)));
    }

    for (auto i = 0U; i < beats.size(); ++i) {
        REQUIRE(converter.measures_to_beats(measures.at(i)).value
                == Approx(beats.at(i)));
    }
}

// Last checked: 24.0.1555-master
TEST_CASE("Measures to seconds conversion", "Measures<->S")
{
    const auto track = SyncTrack({{0, 5, 4}, {1000, 4, 4}, {1200, 4, 16}},
                                 {{0, 150000}, {800, 200000}});
    const auto header = SongHeader(0.F, 200);
    const auto converter = TimeConverter(track, header);
    constexpr std::array measures {-0.25, 0.0, 0.6, 1.125, 1.75};
    constexpr std::array seconds {-0.5, 0.0, 1.2, 2.05, 2.35};

    for (auto i = 0U; i < measures.size(); ++i) {
        REQUIRE(converter.measures_to_seconds(measures.at(i))
                == Approx(seconds.at(i)));
    }

    for (auto i = 0U; i < measures.size(); ++i) {
        REQUIRE(converter.seconds_to_measures(seconds.at(i))
                == Approx(measures.at(i)));
    }
}
