/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include <boost/test/unit_test.hpp>

#include "chart.hpp"
#include "songparts.hpp"

bool operator!=(const BpmEvent& lhs, const BpmEvent& rhs)
{
    return std::tie(lhs.position, lhs.bpm) != std::tie(rhs.position, rhs.bpm);
}

std::ostream& operator<<(std::ostream& stream, const BpmEvent& event)
{
    stream << "{Pos " << event.position << ", BPM " << event.bpm << '}';
    return stream;
}

bool operator!=(const Event& lhs, const Event& rhs)
{
    return std::tie(lhs.position, lhs.data) != std::tie(rhs.position, rhs.data);
}

std::ostream& operator<<(std::ostream& stream, const Event& event)
{
    stream << "{Pos " << event.position << ", Data " << event.data << '}';
    return stream;
}

bool operator!=(const NoteEvent& lhs, const NoteEvent& rhs)
{
    return std::tie(lhs.position, lhs.fret, lhs.length)
        != std::tie(rhs.position, rhs.fret, rhs.length);
}

std::ostream& operator<<(std::ostream& stream, const NoteEvent& event)
{
    stream << "{Pos " << event.position << ", Fret " << event.fret << ", Length"
           << event.length << '}';
    return stream;
}

bool operator!=(const SpecialEvent& lhs, const SpecialEvent& rhs)
{
    return std::tie(lhs.position, lhs.key, lhs.length)
        != std::tie(rhs.position, rhs.key, rhs.length);
}

std::ostream& operator<<(std::ostream& stream, const SpecialEvent& event)
{
    stream << "{Pos " << event.position << ", Key " << event.key << ", Length"
           << event.length << '}';
    return stream;
}

bool operator!=(const TimeSigEvent& lhs, const TimeSigEvent& rhs)
{
    return std::tie(lhs.position, lhs.numerator, lhs.denominator)
        != std::tie(rhs.position, rhs.numerator, rhs.denominator);
}

std::ostream& operator<<(std::ostream& stream, const TimeSigEvent& ts)
{
    stream << "{Pos " << ts.position << ", " << ts.numerator << '/'
           << ts.denominator << '}';
    return stream;
}

BOOST_AUTO_TEST_CASE(section_names_are_read)
{
    const char* text = "[SectionA]\n{\n}\n[SectionB]\n{\n}\n";

    const auto chart = parse_chart(text);

    BOOST_CHECK_EQUAL(chart.sections.size(), 2);
    BOOST_CHECK_EQUAL(chart.sections[0].name, "SectionA");
    BOOST_CHECK_EQUAL(chart.sections[1].name, "SectionB");
}

BOOST_AUTO_TEST_CASE(parser_skips_utf8_bom)
{
    const char* text = "\xEF\xBB\xBF[Song]\n{\n}\n";

    const auto chart = parse_chart(text);

    BOOST_CHECK_EQUAL(chart.sections.size(), 1);
    BOOST_CHECK_EQUAL(chart.sections[0].name, "Song");
}

BOOST_AUTO_TEST_CASE(chart_can_end_without_newline)
{
    const char* text = "[Song]\n{\n}";

    BOOST_CHECK_NO_THROW([&] { return parse_chart(text); }());
}

BOOST_AUTO_TEST_CASE(parser_does_not_infinite_loop_due_to_unfinished_section)
{
    const char* text = "[UnrecognisedSection]\n{\n";

    BOOST_CHECK_THROW([&] { return parse_chart(text); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(lone_carriage_return_does_not_break_line)
{
    const char* text = "[Section]\r\n{\r\nKey = Value\rOops\r\n}";

    const auto section = parse_chart(text).sections[0];

    BOOST_CHECK_EQUAL(section.key_value_pairs.size(), 1);
    BOOST_CHECK_EQUAL(section.key_value_pairs.at("Key"), "Value\rOops");
}

BOOST_AUTO_TEST_CASE(key_value_pairs_are_read)
{
    const char* text = "[Section]\n{\nKey = Value\nKey2 = Value2\n}";

    const auto section = parse_chart(text).sections[0];

    BOOST_CHECK_EQUAL(section.key_value_pairs.size(), 2);
    BOOST_CHECK_EQUAL(section.key_value_pairs.at("Key"), "Value");
    BOOST_CHECK_EQUAL(section.key_value_pairs.at("Key2"), "Value2");
}

BOOST_AUTO_TEST_CASE(note_events_are_read)
{
    const char* text = "[Section]\n{\n1000 = N 1 0\n}";
    const std::vector<NoteEvent> events {{1000, 1, 0}};

    const auto section = parse_chart(text).sections[0];

    BOOST_CHECK_EQUAL_COLLECTIONS(section.note_events.cbegin(),
                                  section.note_events.cend(), events.cbegin(),
                                  events.cend());
}

BOOST_AUTO_TEST_CASE(note_events_with_extra_spaces_throw)
{
    const char* text = "[Section]\n{\n768 = N  0 0\n}";

    BOOST_CHECK_THROW([&] { return parse_chart(text); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(bpm_events_are_read)
{
    const char* text = "[Section]\n{\n1000 = B 150000\n}";
    const std::vector<BpmEvent> events {{1000, 150000}};

    const auto section = parse_chart(text).sections[0];

    BOOST_CHECK_EQUAL_COLLECTIONS(section.bpm_events.cbegin(),
                                  section.bpm_events.cend(), events.cbegin(),
                                  events.cend());
}

BOOST_AUTO_TEST_CASE(timesig_events_are_read)
{
    const char* text = "[Section]\n{\n1000 = TS 4\n2000 = TS 3 3\n}";
    const std::vector<TimeSigEvent> events {{1000, 4, 2}, {2000, 3, 3}};

    const auto section = parse_chart(text).sections[0];

    BOOST_CHECK_EQUAL_COLLECTIONS(section.ts_events.cbegin(),
                                  section.ts_events.cend(), events.cbegin(),
                                  events.cend());
}

BOOST_AUTO_TEST_CASE(special_events_are_read)
{
    const char* text = "[Section]\n{\n1000 = S 2 700\n}";
    const std::vector<SpecialEvent> events {{1000, 2, 700}};

    const auto section = parse_chart(text).sections[0];

    BOOST_CHECK_EQUAL_COLLECTIONS(section.special_events.cbegin(),
                                  section.special_events.cend(),
                                  events.cbegin(), events.cend());
}

BOOST_AUTO_TEST_CASE(e_events_are_read)
{
    const char* text = "[Section]\n{\n1000 = E soloing\n}";
    const std::vector<Event> events {{1000, "soloing"}};

    const auto section = parse_chart(text).sections[0];

    BOOST_CHECK_EQUAL_COLLECTIONS(section.events.cbegin(),
                                  section.events.cend(), events.cbegin(),
                                  events.cend());
}

BOOST_AUTO_TEST_CASE(other_events_are_ignored)
{
    const char* text = "[Section]\n{\n1105 = A 133\n}";

    const auto section = parse_chart(text).sections[0];

    BOOST_TEST(section.note_events.empty());
    BOOST_TEST(section.note_events.empty());
    BOOST_TEST(section.bpm_events.empty());
    BOOST_TEST(section.ts_events.empty());
    BOOST_TEST(section.events.empty());
}

// Yes, these are actually a thing. Clone Hero accepts them, so I have to.
BOOST_AUTO_TEST_CASE(utf16le_charts_are_read_correctly)
{
    const std::string text {
        "\xFF\xFE\x5B\x00\x53\x00\x6F\x00\x6E\x00\x67\x00\x5D\x00\x0D\x00\x0A"
        "\x00\x7B\x00\x0D\x00\x0A\x00\x7D\x00",
        26};

    const auto chart = parse_chart(text);

    BOOST_CHECK_EQUAL(chart.sections.size(), 1);
    BOOST_CHECK_EQUAL(chart.sections[0].name, "Song");
}

BOOST_AUTO_TEST_CASE(utf16le_charts_must_be_of_even_length)
{
    const std::string text {
        "\xFF\xFE\x5B\x00\x53\x00\x6F\x00\x6E\x00\x67\x00\x5D\x00\x0D\x00\x0A"
        "\x00\x7B\x00\x0D\x00\x0A\x00\x7D\x00\x00",
        27};

    BOOST_CHECK_THROW([&] { return parse_chart(text); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(single_character_headers_should_throw)
{
    BOOST_CHECK_THROW([] { return parse_chart("\n"); }(), ParseError);
}

BOOST_AUTO_TEST_CASE(short_mid_section_lines_throw)
{
    BOOST_CHECK_THROW(
        [&] { return parse_chart("[ExpertGuitar]\n{\n1 1\n}"); }(), ParseError);
    BOOST_CHECK_THROW(
        [&] { return parse_chart("[ExpertGuitar]\n{\n1 = N 1\n}"); }(),
        ParseError);
}
