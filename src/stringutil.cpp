/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024, 2026 Raymond Wright
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
#include <array>
#include <stdexcept>

#include "stringutil.hpp"

std::string to_ordinal(int ordinal)
{
    constexpr int TENS_MODULUS = 10;
    constexpr int HUNDREDS_MODULUS = 100;
    constexpr std::array<int, 3> EXCEPTIONAL_TEENS {11, 12, 13};

    if (ordinal < 0) {
        throw std::runtime_error("ordinal was negative");
    }
    if (std::find(EXCEPTIONAL_TEENS.cbegin(), EXCEPTIONAL_TEENS.cend(),
                  ordinal % HUNDREDS_MODULUS)
        != EXCEPTIONAL_TEENS.cend()) {
        return std::to_string(ordinal) + "th";
    }
    if (ordinal % TENS_MODULUS == 1) {
        return std::to_string(ordinal) + "st";
    }
    if (ordinal % TENS_MODULUS == 2) {
        return std::to_string(ordinal) + "nd";
    }
    if (ordinal % TENS_MODULUS == 3) {
        return std::to_string(ordinal) + "rd";
    }
    return std::to_string(ordinal) + "th";
}
