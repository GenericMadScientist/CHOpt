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

#include <tuple>

#include "catch.hpp"

#include "chart.hpp"

static bool operator==(const ChartEvent& lhs, const ChartEvent& rhs)
{
    return std::tie(lhs.position, lhs.event_type, lhs.data)
        == std::tie(rhs.position, rhs.event_type, rhs.data);
}

TEST_CASE("Section names are read")
{
    const char* text = "[SectionA]\n{\n}\n[SectionB]\n{\n}\n";

    const auto chart = parse_chart(text);

    REQUIRE(chart.sections.size() == 2);
    REQUIRE(chart.sections[0].name == "SectionA");
    REQUIRE(chart.sections[1].name == "SectionB");
}

TEST_CASE("Parser skips UTF-8 BOM")
{
    const char* text = "\xEF\xBB\xBF[Song]\n{\n}\n";

    const auto chart = parse_chart(text);

    REQUIRE(chart.sections.size() == 1);
    REQUIRE(chart.sections[0].name == "Song");
}

TEST_CASE("Chart can end without a newline")
{
    const char* text = "[Song]\n{\n}";

    REQUIRE_NOTHROW([&] { return parse_chart(text); }());
}

TEST_CASE("Key value pairs are read")
{
    const char* text = "[Section]\n{\nKey = Value\nKey2 = Value2\n}";
    const std::map<std::string, std::string> pairs {{"Key", "Value"},
                                                    {"Key2", "Value2"}};

    const auto section = parse_chart(text).sections[0];

    REQUIRE(section.key_value_pairs == pairs);
}

TEST_CASE("Section events are read")
{
    const char* text = "[Section]\n{\n1105 = A 133\n}";
    const std::vector<ChartEvent> events {{1105, "A", "133"}};

    const auto section = parse_chart(text).sections[0];

    REQUIRE(section.events == events);
}
