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

#include "chart.hpp"

TEST_CASE("Section names are read")
{
    const char* text = "[SectionA]\n{\n}\n[SectionB]\n{\n}";

    const auto chart = parse_chart(text);

    REQUIRE(chart.sections.size() == 2);
    REQUIRE(chart.sections[0].name == "SectionA");
    REQUIRE(chart.sections[1].name == "SectionB");
}
