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

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>

#include "chart.hpp"

// This represents a bundle of data akin to a NoteTrack, except it is only for
// mid-parser usage. Unlike a NoteTrack, there are no invariants.
struct PreNoteTrack {
    std::vector<Note> notes;
    std::vector<StarPower> sp_phrases;
    std::vector<Solo> solos;
};

static bool is_empty(const PreNoteTrack& track)
{
    return track.notes.empty() && track.sp_phrases.empty();
}

// This represents a bundle of data akin to a SyncTrack, except it is only for
// mid-parser usage. Unlike a SyncTrack, there are no invariants.
struct PreSyncTrack {
    std::vector<TimeSignature> time_sigs;
    std::vector<BPM> bpms;
};

// This represents a bundle of data akin to a SongHeader, except it also has the
// resolution built in.
struct PreSongHeader {
    static constexpr int DEFAULT_RESOLUTION = 192;

    std::string name {"Unknown Song"};
    std::string artist {"Unknown Artist"};
    std::string charter {"Unknown Charter"};
    int resolution = DEFAULT_RESOLUTION;
};

NoteTrack::NoteTrack(std::vector<Note> notes, std::vector<StarPower> sp_phrases,
                     std::vector<Solo> solos)
{
    std::stable_sort(notes.begin(), notes.end(),
                     [](const auto& lhs, const auto& rhs) {
                         return std::tie(lhs.position, lhs.colour)
                             < std::tie(rhs.position, rhs.colour);
                     });

    if (!notes.empty()) {
        auto prev_note = notes.cbegin();
        for (auto p = notes.cbegin() + 1; p < notes.cend(); ++p) {
            if (p->position != prev_note->position
                || p->colour != prev_note->colour) {
                m_notes.push_back(*prev_note);
            }
            prev_note = p;
        }
        m_notes.push_back(*prev_note);
    }

    std::stable_sort(sp_phrases.begin(), sp_phrases.end(),
                     [](const auto& lhs, const auto& rhs) {
                         return lhs.position < rhs.position;
                     });

    for (auto p = sp_phrases.begin(); p < sp_phrases.end(); ++p) {
        const auto next_phrase = p + 1;
        if (next_phrase == sp_phrases.end()) {
            continue;
        }
        p->length = std::min(p->length, next_phrase->position - p->position);
    }

    for (const auto& phrase : sp_phrases) {
        const auto first_note = std::lower_bound(
            m_notes.cbegin(), m_notes.cend(), phrase.position,
            [](const auto& lhs, const auto& rhs) {
                return lhs.position < rhs;
            });
        if ((first_note != m_notes.cend())
            && (first_note->position < (phrase.position + phrase.length))) {
            m_sp_phrases.push_back(phrase);
        }
    }

    std::stable_sort(
        solos.begin(), solos.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.start < rhs.start; });

    m_solos = std::move(solos);
}

SyncTrack::SyncTrack(std::vector<TimeSignature> time_sigs,
                     std::vector<BPM> bpms)
{
    constexpr auto DEFAULT_BPM = 120000;

    std::stable_sort(
        bpms.begin(), bpms.end(),
        [](const auto& x, const auto& y) { return x.position < y.position; });
    BPM prev_bpm {0, DEFAULT_BPM};
    for (auto p = bpms.cbegin(); p < bpms.cend(); ++p) {
        if (p->position != prev_bpm.position) {
            m_bpms.push_back(prev_bpm);
        }
        prev_bpm = *p;
    }
    m_bpms.push_back(prev_bpm);

    std::stable_sort(
        time_sigs.begin(), time_sigs.end(),
        [](const auto& x, const auto& y) { return x.position < y.position; });
    TimeSignature prev_ts {0, 4, 4};
    for (auto p = time_sigs.cbegin(); p < time_sigs.cend(); ++p) {
        if (p->position != prev_ts.position) {
            m_time_sigs.push_back(prev_ts);
        }
        prev_ts = *p;
    }
    m_time_sigs.push_back(prev_ts);
}

static bool string_starts_with(std::string_view input, std::string_view pattern)
{
    if (input.size() < pattern.size()) {
        return false;
    }

    return input.substr(0, pattern.size()) == pattern;
}

static std::string_view skip_whitespace(std::string_view input)
{
    const auto first_non_ws_location = input.find_first_not_of(" \f\n\r\t\v");
    input.remove_prefix(std::min(first_non_ws_location, input.size()));
    return input;
}

// This returns a string_view from the start of input until a carriage return
// or newline. input is changed to point to the first character past the
// detected newline character that is not a whitespace character.
static std::string_view break_off_newline(std::string_view& input)
{
    const auto newline_location = input.find_first_of("\r\n");
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

// Split input by space characters, similar to .Split(' ') in C#. Note that
// the lifetime of the string_views in the output is the same as that of the
// input.
static std::vector<std::string_view> split_by_space(std::string_view input)
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

// Convert a string_view to an int. If there are any problems with the input,
// this function throws.
static std::optional<int> string_view_to_int(std::string_view input)
{
    int result = 0;
    const char* last = input.data() + input.size();
    auto [p, ec] = std::from_chars(input.data(), last, result);
    if ((ec != std::errc()) || (p != last)) {
        return {};
    }
    return result;
}

// Return the substring with no leading or trailing quotation marks.
static std::string_view trim_quotes(std::string_view input)
{
    const auto first_non_quote = input.find_first_not_of('"');
    if (first_non_quote == std::string_view::npos) {
        return input.substr(0, 0);
    }
    const auto last_non_quote = input.find_last_not_of('"');
    return input.substr(first_non_quote, last_non_quote - first_non_quote + 1);
}

static std::string_view skip_section(std::string_view input)
{
    auto next_line = break_off_newline(input);
    if (next_line != "{") {
        throw std::runtime_error("Section does not open with {");
    }

    do {
        next_line = break_off_newline(input);
    } while (next_line != "}");

    return input;
}

static std::string_view read_song_header(std::string_view input,
                                         PreSongHeader& header)
{
    if (break_off_newline(input) != "{") {
        throw std::runtime_error("[Song] does not open with {");
    }

    while (true) {
        auto line = break_off_newline(input);
        if (line == "}") {
            break;
        }

        if (string_starts_with(line, "Resolution = ")) {
            constexpr auto RESOLUTION_LEN = 13;
            line.remove_prefix(RESOLUTION_LEN);
            const auto result = string_view_to_int(line);
            if (result) {
                header.resolution = *result;
            }
        } else if (string_starts_with(line, "Name = ")) {
            constexpr auto NAME_LEN = 7;
            line.remove_prefix(NAME_LEN);
            line = trim_quotes(line);
            header.name = line;
        } else if (string_starts_with(line, "Artist = ")) {
            constexpr auto ARTIST_LEN = 9;
            line.remove_prefix(ARTIST_LEN);
            line = trim_quotes(line);
            header.artist = line;
        } else if (string_starts_with(line, "Charter = ")) {
            constexpr auto CHARTER_LEN = 10;
            line.remove_prefix(CHARTER_LEN);
            line = trim_quotes(line);
            header.charter = line;
        }
    }

    return input;
}

static std::string_view read_sync_track(std::string_view input,
                                        PreSyncTrack& sync_track)
{
    if (break_off_newline(input) != "{") {
        throw std::runtime_error("[SyncTrack] does not open with {");
    }

    while (true) {
        const auto line = break_off_newline(input);
        if (line == "}") {
            break;
        }

        const auto split_string = split_by_space(line);
        if (split_string.size() < 4) {
            throw std::invalid_argument("Event missing data");
        }
        const auto pre_position = string_view_to_int(split_string[0]);
        if (!pre_position) {
            continue;
        }
        const auto position = *pre_position;

        const auto type = split_string[2];

        if (type == "TS") {
            const auto numerator = string_view_to_int(split_string[3]);
            if (!numerator) {
                continue;
            }
            int denominator_base = 2;
            if (split_string.size() > 4) {
                const auto result = string_view_to_int(split_string[4]);
                if (result) {
                    denominator_base = *result;
                } else {
                    continue;
                }
            }
            // TODO: Check how Clone Hero reacts to a base outside the range of
            // [0, 31]. For now, yes clang-tidy, we are technically in a state
            // of sin.
            int denominator = 1 << denominator_base; // NOLINT
            sync_track.time_sigs.push_back({position, *numerator, denominator});
        } else if (type == "B") {
            const auto bpm = string_view_to_int(split_string[3]);
            if (!bpm) {
                continue;
            }
            sync_track.bpms.push_back({position, *bpm});
        }
    }

    return input;
}

static std::string_view read_single_track(std::string_view input,
                                          PreNoteTrack& track)
{
    if (!is_empty(track)) {
        return skip_section(input);
    }

    constexpr auto GREEN_CODE = 0;
    constexpr auto RED_CODE = 1;
    constexpr auto YELLOW_CODE = 2;
    constexpr auto BLUE_CODE = 3;
    constexpr auto ORANGE_CODE = 4;
    constexpr auto FORCED_CODE = 5;
    constexpr auto TAP_CODE = 6;
    constexpr auto OPEN_CODE = 7;

    constexpr int SOLO_NOTE_VALUE = 100;

    if (break_off_newline(input) != "{") {
        throw std::runtime_error("A [*Single] track does not open with {");
    }

    // Pairs are (location, x) where x is 0 for solo and 1 for soloend.
    std::vector<std::tuple<int, int>> solo_events;

    while (true) {
        const auto line = break_off_newline(input);
        if (line == "}") {
            break;
        }

        const auto split_string = split_by_space(line);
        if (split_string.size() < 4) {
            throw std::invalid_argument("Event missing data");
        }
        const auto pre_position = string_view_to_int(split_string[0]);
        if (!pre_position) {
            continue;
        }
        const auto position = *pre_position;
        const auto type = split_string[2];

        if (type == "N") {
            constexpr auto NOTE_EVENT_LENGTH = 5;
            if (split_string.size() < NOTE_EVENT_LENGTH) {
                throw std::invalid_argument("Note event missing data");
            }
            const auto fret_type = string_view_to_int(split_string[3]);
            if (!fret_type) {
                throw std::invalid_argument("Note has invalid fret");
            }
            const auto pre_length = string_view_to_int(split_string[4]);
            if (!pre_length) {
                throw std::invalid_argument("Note has invalid length");
            }
            const auto length = *pre_length;
            switch (*fret_type) {
            case GREEN_CODE:
                track.notes.push_back({position, length, NoteColour::Green});
                break;
            case RED_CODE:
                track.notes.push_back({position, length, NoteColour::Red});
                break;
            case YELLOW_CODE:
                track.notes.push_back({position, length, NoteColour::Yellow});
                break;
            case BLUE_CODE:
                track.notes.push_back({position, length, NoteColour::Blue});
                break;
            case ORANGE_CODE:
                track.notes.push_back({position, length, NoteColour::Orange});
                break;
            case FORCED_CODE:
            case TAP_CODE:
                break;
            case OPEN_CODE:
                track.notes.push_back({position, length, NoteColour::Open});
                break;
            default:
                throw std::invalid_argument("Invalid note type");
            }
        } else if (type == "S") {
            constexpr auto SP_EVENT_LENGTH = 5;
            if (split_string.size() < SP_EVENT_LENGTH) {
                throw std::invalid_argument("SP event missing data");
            }
            if (string_view_to_int(split_string[3]) != 2) {
                continue;
            }
            const auto pre_length = string_view_to_int(split_string[4]);
            if (!pre_length) {
                continue;
            }
            const auto length = *pre_length;
            track.sp_phrases.push_back({position, length});
        } else if (type == "E") {
            if (split_string[3] == "solo") {
                solo_events.emplace_back(position, 0);
            } else if (split_string[3] == "soloend") {
                solo_events.emplace_back(position, 1);
            }
        }
    }

    std::sort(solo_events.begin(), solo_events.end());

    int start = 0;
    int end = 0;
    bool in_solo = false;
    for (auto [pos, type] : solo_events) {
        if (type == 0 && !in_solo) {
            in_solo = true;
            start = pos;
        } else if (type == 1 && in_solo) {
            in_solo = false;
            end = pos;
            std::set<int> positions_in_solo;
            for (const auto& note : track.notes) {
                if (note.position >= start && note.position <= end) {
                    positions_in_solo.insert(note.position);
                }
            }
            if (positions_in_solo.empty()) {
                continue;
            }
            auto notes = static_cast<int>(positions_in_solo.size());
            track.solos.push_back({start, end, SOLO_NOTE_VALUE * notes});
        }
    }

    return input;
}

Chart Chart::parse_chart(std::string_view input)
{
    Chart chart;

    PreSongHeader pre_song_header;
    PreSyncTrack pre_sync_track;
    std::map<Difficulty, PreNoteTrack> pre_tracks;

    // Trim off UTF-8 BOM if present
    if (string_starts_with(input, "\xEF\xBB\xBF")) {
        input.remove_prefix(3);
    }

    while (!input.empty()) {
        const auto header = break_off_newline(input);
        if (header == "[Song]") {
            input = read_song_header(input, pre_song_header);
        } else if (header == "[SyncTrack]") {
            input = read_sync_track(input, pre_sync_track);
        } else if (header == "[EasySingle]") {
            input = read_single_track(input, pre_tracks[Difficulty::Easy]);
        } else if (header == "[MediumSingle]") {
            input = read_single_track(input, pre_tracks[Difficulty::Medium]);
        } else if (header == "[HardSingle]") {
            input = read_single_track(input, pre_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertSingle]") {
            input = read_single_track(input, pre_tracks[Difficulty::Expert]);
        } else {
            input = skip_section(input);
        }
    }

    chart.m_resolution = pre_song_header.resolution;
    chart.m_song_header.name = pre_song_header.name;
    chart.m_song_header.artist = pre_song_header.artist;
    chart.m_song_header.charter = pre_song_header.charter;

    chart.m_sync_track = SyncTrack(std::move(pre_sync_track.time_sigs),
                                   std::move(pre_sync_track.bpms));

    for (auto& key_track : pre_tracks) {
        auto diff = key_track.first;
        auto& track = key_track.second;
        if (track.notes.empty()) {
            continue;
        }
        auto new_track
            = NoteTrack(std::move(track.notes), std::move(track.sp_phrases),
                        std::move(track.solos));
        chart.m_note_tracks.emplace(diff, std::move(new_track));
    }

    if (chart.m_note_tracks.empty()) {
        throw std::invalid_argument("Chart has no notes");
    }

    return chart;
}

Chart Chart::from_midi(const Midi& midi)
{
    (void)midi;
    throw std::invalid_argument("Midi files currently unsupported");
}
