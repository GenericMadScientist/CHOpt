#include <charconv>
#include <optional>

#include <sightread/songparts.hpp>

#include "chart.hpp"

namespace {
std::string_view skip_whitespace(std::string_view input)
{
    const auto first_non_ws_location = input.find_first_not_of(" \f\n\r\t\v");
    input.remove_prefix(std::min(first_non_ws_location, input.size()));
    return input;
}

std::string_view break_off_newline(std::string_view& input)
{
    if (input.empty()) {
        throw SightRead::ParseError("No lines left");
    }

    const auto newline_location
        = std::min(input.find('\n'), input.find("\r\n"));
    if (newline_location == std::string_view::npos) {
        const auto line = input;
        input.remove_prefix(input.size());
        return line;
    }

    const auto line = input.substr(0, newline_location);
    input.remove_prefix(newline_location);
    input = skip_whitespace(input);
    return line;
}

std::string_view strip_square_brackets(std::string_view input)
{
    if (input.empty()) {
        throw SightRead::ParseError("Header string empty");
    }
    return input.substr(1, input.size() - 2);
}

// Convert a string_view to an int. If there are any problems with the input,
// this function returns std::nullopt.
std::optional<int> string_view_to_int(std::string_view input)
{
    int result = 0;
    const char* last = input.data() + input.size();
    auto [p, ec] = std::from_chars(input.data(), last, result);
    if ((ec != std::errc()) || (p != last)) {
        return std::nullopt;
    }
    return result;
}

// Split input by space characters, similar to .Split(' ') in C#. Note that
// the lifetime of the string_views in the output is the same as that of the
// input.
std::vector<std::string_view> split_by_space(std::string_view input)
{
    std::vector<std::string_view> substrings;

    while (true) {
        const auto space_location = input.find(' ');
        if (space_location == std::string_view::npos) {
            break;
        }
        substrings.push_back(input.substr(0, space_location));
        input.remove_prefix(space_location + 1);
    }

    substrings.push_back(input);
    return substrings;
}

SightRead::Detail::NoteEvent
convert_line_to_note(int position,
                     const std::vector<std::string_view>& split_line)
{
    constexpr int MAX_NORMAL_EVENT_SIZE = 5;

    if (split_line.size() < MAX_NORMAL_EVENT_SIZE) {
        throw SightRead::ParseError("Line incomplete");
    }
    const auto fret = string_view_to_int(split_line[3]);
    const auto length = string_view_to_int(split_line[4]);
    if (!fret.has_value() || !length.has_value()) {
        throw SightRead::ParseError("Bad note event");
    }
    return {position, *fret, *length};
}

SightRead::Detail::SpecialEvent
convert_line_to_special(int position,
                        const std::vector<std::string_view>& split_line)
{
    constexpr int MAX_NORMAL_EVENT_SIZE = 5;

    if (split_line.size() < MAX_NORMAL_EVENT_SIZE) {
        throw SightRead::ParseError("Line incomplete");
    }
    const auto sp_key = string_view_to_int(split_line[3]);
    const auto length = string_view_to_int(split_line[4]);
    if (!sp_key.has_value() || !length.has_value()) {
        throw SightRead::ParseError("Bad SP event");
    }
    return {position, *sp_key, *length};
}

SightRead::Detail::BpmEvent
convert_line_to_bpm(int position,
                    const std::vector<std::string_view>& split_line)
{
    if (split_line.size() < 4) {
        throw SightRead::ParseError("Line incomplete");
    }
    const auto bpm = string_view_to_int(split_line[3]);
    if (!bpm.has_value()) {
        throw SightRead::ParseError("Bad BPM event");
    }
    return {position, *bpm};
}

SightRead::Detail::TimeSigEvent
convert_line_to_timesig(int position,
                        const std::vector<std::string_view>& split_line)
{
    constexpr int MAX_NORMAL_EVENT_SIZE = 5;

    if (split_line.size() < 4) {
        throw SightRead::ParseError("Line incomplete");
    }
    const auto numer = string_view_to_int(split_line[3]);
    std::optional<int> denom = 2;
    if (split_line.size() >= MAX_NORMAL_EVENT_SIZE) {
        denom = string_view_to_int(split_line[4]);
    }
    if (!numer.has_value() || !denom.has_value()) {
        throw SightRead::ParseError("Bad TS event");
    }
    return {position, *numer, *denom};
}

SightRead::Detail::Event
convert_line_to_event(int position,
                      const std::vector<std::string_view>& split_line)
{
    if (split_line.size() < 4) {
        throw SightRead::ParseError("Line incomplete");
    }
    return {position, std::string {split_line[3]}};
}

SightRead::Detail::ChartSection read_section(std::string_view& input)
{
    SightRead::Detail::ChartSection section;
    section.name = strip_square_brackets(break_off_newline(input));

    if (break_off_newline(input) != "{") {
        throw SightRead::ParseError("Section does not open with {");
    }

    while (true) {
        const auto next_line = break_off_newline(input);
        if (next_line == "}") {
            break;
        }
        const auto separated_line = split_by_space(next_line);
        if (separated_line.size() < 3) {
            throw SightRead::ParseError("Line incomplete");
        }
        const auto key = separated_line[0];
        const auto key_val = string_view_to_int(key);
        if (key_val.has_value()) {
            const auto pos = *key_val;
            if (separated_line[2] == "N") {
                section.note_events.push_back(
                    convert_line_to_note(pos, separated_line));
            } else if (separated_line[2] == "S") {
                section.special_events.push_back(
                    convert_line_to_special(pos, separated_line));
            } else if (separated_line[2] == "B") {
                section.bpm_events.push_back(
                    convert_line_to_bpm(pos, separated_line));
            } else if (separated_line[2] == "TS") {
                section.ts_events.push_back(
                    convert_line_to_timesig(pos, separated_line));
            } else if (separated_line[2] == "E") {
                section.events.push_back(
                    convert_line_to_event(pos, separated_line));
            }
        } else {
            std::string value {separated_line[2]};
            for (auto i = 3U; i < separated_line.size(); ++i) {
                value.append(separated_line[i]);
            }
            section.key_value_pairs[std::string(key)] = value;
        }
    }

    return section;
}
}

SightRead::Detail::Chart SightRead::Detail::parse_chart(std::string_view data)
{
    SightRead::Detail::Chart chart;

    while (!data.empty()) {
        chart.sections.push_back(read_section(data));
    }

    return chart;
}
