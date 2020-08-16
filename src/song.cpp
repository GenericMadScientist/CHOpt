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

#include <charconv>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <optional>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "song.hpp"

static bool ends_in_suffix(const std::string& string, const char* suffix)
{
    const auto suffix_len = std::strlen(suffix);

    if (string.size() < suffix_len) {
        return false;
    }
    return string.substr(string.size() - suffix_len) == suffix;
}

Song Song::from_filename(const std::string& filename)
{
    if (ends_in_suffix(filename, ".chart")) {
        std::ifstream in {filename};
        if (!in.is_open()) {
            throw std::invalid_argument("File did not open");
        }
        std::string contents {std::istreambuf_iterator<char>(in),
                              std::istreambuf_iterator<char>()};
        return Song::parse_chart(contents);
    }
    if (ends_in_suffix(filename, ".mid")) {
        std::ifstream in {filename, std::ios::binary};
        if (!in.is_open()) {
            throw std::invalid_argument("File did not open");
        }
        std::vector<std::uint8_t> buffer {std::istreambuf_iterator<char>(in),
                                          std::istreambuf_iterator<char>()};
        return Song::from_midi(parse_midi(buffer));
    }
    throw std::invalid_argument("file should be .chart or .mid");
}

// This represents a bundle of data akin to a NoteTrack, except it is only for
// mid-parser usage. Unlike a NoteTrack, there are no invariants.
template <typename T> struct PreNoteTrack {
    std::vector<Note<T>> notes;
    std::vector<StarPower> sp_phrases;
    std::vector<Solo> solos;
};

template <typename T> static bool is_empty(const PreNoteTrack<T>& track)
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
    if (input.empty()) {
        throw std::invalid_argument("No lines left");
    }

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

// Takes a sequence of points where some note type/event is turned on, and a
// sequence where said type is turned off, and returns a tuple of intervals
// where the event is on.
static std::vector<std::tuple<int, int>>
combine_on_off_events(const std::vector<int>& on_events,
                      const std::vector<int>& off_events)
{
    std::vector<std::tuple<int, int>> ranges;

    auto on_iter = on_events.cbegin();
    auto off_iter = off_events.cbegin();

    while (on_iter < on_events.cend() && off_iter < off_events.cend()) {
        if (*on_iter >= *off_iter) {
            ++off_iter;
            continue;
        }
        ranges.emplace_back(*on_iter, *off_iter);
        while (on_iter < on_events.cend() && *on_iter < *off_iter) {
            ++on_iter;
        }
    }

    if (on_iter != on_events.cend()) {
        throw std::invalid_argument("on event has no corresponding off event");
    }

    return ranges;
}

template <typename T>
static std::vector<Solo>
form_solo_vector(const std::vector<int>& solo_on_events,
                 const std::vector<int>& solo_off_events,
                 const std::vector<Note<T>>& notes)
{
    constexpr int SOLO_NOTE_VALUE = 100;

    std::vector<Solo> solos;

    for (auto [start, end] :
         combine_on_off_events(solo_on_events, solo_off_events)) {
        std::set<int> positions_in_solo;
        for (const auto& note : notes) {
            if (note.position >= start && note.position <= end) {
                positions_in_solo.insert(note.position);
            }
        }
        if (positions_in_solo.empty()) {
            continue;
        }
        auto note_count = static_cast<int>(positions_in_solo.size());
        solos.push_back({start, end, SOLO_NOTE_VALUE * note_count});
    }

    return solos;
}

static std::optional<Note<NoteColour>>
note_from_note_colour(int position, int length, int fret_type)
{
    constexpr auto GREEN_CODE = 0;
    constexpr auto RED_CODE = 1;
    constexpr auto YELLOW_CODE = 2;
    constexpr auto BLUE_CODE = 3;
    constexpr auto ORANGE_CODE = 4;
    constexpr auto FORCED_CODE = 5;
    constexpr auto TAP_CODE = 6;
    constexpr auto OPEN_CODE = 7;

    switch (fret_type) {
    case GREEN_CODE:
        return Note<NoteColour> {position, length, NoteColour::Green};
    case RED_CODE:
        return Note<NoteColour> {position, length, NoteColour::Red};
    case YELLOW_CODE:
        return Note<NoteColour> {position, length, NoteColour::Yellow};
    case BLUE_CODE:
        return Note<NoteColour> {position, length, NoteColour::Blue};
    case ORANGE_CODE:
        return Note<NoteColour> {position, length, NoteColour::Orange};
    case FORCED_CODE:
    case TAP_CODE:
        return {};
    case OPEN_CODE:
        return Note<NoteColour> {position, length, NoteColour::Open};
    default:
        throw std::invalid_argument("Invalid note type");
    }
}

static std::optional<Note<GHLNoteColour>>
note_from_ghl_note_colour(int position, int length, int fret_type)
{
    constexpr auto WHITE_LOW_CODE = 0;
    constexpr auto WHITE_MID_CODE = 1;
    constexpr auto WHITE_HIGH_CODE = 2;
    constexpr auto BLACK_LOW_CODE = 3;
    constexpr auto BLACK_MID_CODE = 4;
    constexpr auto FORCED_CODE = 5;
    constexpr auto TAP_CODE = 6;
    constexpr auto OPEN_CODE = 7;
    constexpr auto BLACK_HIGH_CODE = 8;

    switch (fret_type) {
    case WHITE_LOW_CODE:
        return Note<GHLNoteColour> {position, length, GHLNoteColour::WhiteLow};
    case WHITE_MID_CODE:
        return Note<GHLNoteColour> {position, length, GHLNoteColour::WhiteMid};
    case WHITE_HIGH_CODE:
        return Note<GHLNoteColour> {position, length, GHLNoteColour::WhiteHigh};
    case BLACK_LOW_CODE:
        return Note<GHLNoteColour> {position, length, GHLNoteColour::BlackLow};
    case BLACK_MID_CODE:
        return Note<GHLNoteColour> {position, length, GHLNoteColour::BlackMid};
    case BLACK_HIGH_CODE:
        return Note<GHLNoteColour> {position, length, GHLNoteColour::BlackHigh};
    case FORCED_CODE:
    case TAP_CODE:
        return {};
    case OPEN_CODE:
        return Note<GHLNoteColour> {position, length, GHLNoteColour::Open};
    default:
        throw std::invalid_argument("Invalid note type");
    }
}

static std::optional<Note<DrumNoteColour>>
note_from_drum_note_colour(int position, int length, int fret_type)
{
    constexpr auto KICK_CODE = 0;
    constexpr auto RED_CODE = 1;
    constexpr auto YELLOW_CODE = 2;
    constexpr auto BLUE_CODE = 3;
    constexpr auto GREEN_CODE = 4;
    constexpr auto YELLOW_CYMBAL_CODE = 66;
    constexpr auto BLUE_CYMBAL_CODE = 67;
    constexpr auto GREEN_CYMBAL_CODE = 68;

    (void)length;

    switch (fret_type) {
    case KICK_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::Kick};
    case RED_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::Red};
    case YELLOW_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::Yellow};
    case BLUE_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::Blue};
    case GREEN_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::Green};
    case YELLOW_CYMBAL_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::YellowCymbal};
    case BLUE_CYMBAL_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::BlueCymbal};
    case GREEN_CYMBAL_CODE:
        return Note<DrumNoteColour> {position, 0, DrumNoteColour::GreenCymbal};
    default:
        throw std::invalid_argument("Invalid note type");
    }
}

template <typename T>
static std::string_view read_single_track(std::string_view input,
                                          PreNoteTrack<T>& track)
{
    if (!is_empty(track)) {
        return skip_section(input);
    }

    if (break_off_newline(input) != "{") {
        throw std::runtime_error("A [*Single] track does not open with {");
    }

    std::vector<int> solo_on_events;
    std::vector<int> solo_off_events;

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
            if constexpr (std::is_same_v<T, NoteColour>) {
                const auto note
                    = note_from_note_colour(position, length, *fret_type);
                if (note.has_value()) {
                    track.notes.push_back(*note);
                }
            } else if constexpr (std::is_same_v<T, GHLNoteColour>) {
                const auto note
                    = note_from_ghl_note_colour(position, length, *fret_type);
                if (note.has_value()) {
                    track.notes.push_back(*note);
                }
            } else if constexpr (std::is_same_v<T, DrumNoteColour>) {
                const auto note
                    = note_from_drum_note_colour(position, length, *fret_type);
                if (note.has_value()) {
                    track.notes.push_back(*note);
                }
            } else {
                static_assert(
                    std::is_same_v<
                        T,
                        NoteColour> || std::is_same_v<T, GHLNoteColour> || std::is_same_v<T, DrumNoteColour>,
                    "Invalid note type");
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
                solo_on_events.push_back(position);
            } else if (split_string[3] == "soloend") {
                solo_off_events.push_back(position);
            }
        }
    }

    // Handle cymbals
    if constexpr (std::is_same_v<T, DrumNoteColour>) {
        std::set<unsigned int> deletion_spots;
        for (auto i = 0U; i < track.notes.size(); ++i) {
            const auto& cymbal_note = track.notes[i];
            DrumNoteColour non_cymbal_colour = DrumNoteColour::Kick;
            switch (cymbal_note.colour) {
            case DrumNoteColour::YellowCymbal:
                non_cymbal_colour = DrumNoteColour::Yellow;
                break;
            case DrumNoteColour::BlueCymbal:
                non_cymbal_colour = DrumNoteColour::Blue;
                break;
            case DrumNoteColour::GreenCymbal:
                non_cymbal_colour = DrumNoteColour::Green;
                break;
            case DrumNoteColour::Red:
            case DrumNoteColour::Yellow:
            case DrumNoteColour::Blue:
            case DrumNoteColour::Green:
            case DrumNoteColour::Kick:
                continue;
            }
            bool delete_cymbal = true;
            for (auto j = 0U; j < track.notes.size(); ++j) {
                const auto& non_cymbal_note = track.notes[j];
                if (non_cymbal_note.position != cymbal_note.position) {
                    continue;
                }
                if (non_cymbal_note.colour != non_cymbal_colour) {
                    continue;
                }
                delete_cymbal = false;
                deletion_spots.insert(j);
            }
            if (delete_cymbal) {
                deletion_spots.insert(i);
            }
        }
        std::vector<Note<DrumNoteColour>> new_notes;
        for (auto i = 0U; i < track.notes.size(); ++i) {
            if (deletion_spots.count(i) == 0) {
                new_notes.push_back(track.notes[i]);
            }
        }
        track.notes = std::move(new_notes);
    }

    std::sort(solo_on_events.begin(), solo_on_events.end());
    std::sort(solo_off_events.begin(), solo_off_events.end());
    track.solos
        = form_solo_vector(solo_on_events, solo_off_events, track.notes);

    return input;
}

template <typename T>
static std::map<Difficulty, NoteTrack<T>>
tracks_from_pre_tracks(std::map<Difficulty, PreNoteTrack<T>> pre_tracks)
{
    std::map<Difficulty, NoteTrack<T>> tracks;

    for (auto& key_track : pre_tracks) {
        auto diff = key_track.first;
        auto& track = key_track.second;
        if (track.notes.empty()) {
            continue;
        }
        NoteTrack<T> new_track {std::move(track.notes),
                                std::move(track.sp_phrases),
                                std::move(track.solos)};
        tracks.emplace(diff, std::move(new_track));
    }

    return tracks;
}

Song Song::parse_chart(std::string_view input)
{
    Song song;

    PreSongHeader pre_song_header;
    PreSyncTrack pre_sync_track;
    std::map<Difficulty, PreNoteTrack<NoteColour>> pre_bass_tracks;
    std::map<Difficulty, PreNoteTrack<NoteColour>> pre_guitar_tracks;
    std::map<Difficulty, PreNoteTrack<NoteColour>> pre_guitar_coop_tracks;
    std::map<Difficulty, PreNoteTrack<NoteColour>> pre_keys_tracks;
    std::map<Difficulty, PreNoteTrack<NoteColour>> pre_rhythm_tracks;
    std::map<Difficulty, PreNoteTrack<GHLNoteColour>> pre_ghl_guitar_tracks;
    std::map<Difficulty, PreNoteTrack<GHLNoteColour>> pre_ghl_bass_tracks;
    std::map<Difficulty, PreNoteTrack<DrumNoteColour>> pre_drum_tracks;

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
            input
                = read_single_track(input, pre_guitar_tracks[Difficulty::Easy]);
        } else if (header == "[MediumSingle]") {
            input = read_single_track(input,
                                      pre_guitar_tracks[Difficulty::Medium]);
        } else if (header == "[HardSingle]") {
            input
                = read_single_track(input, pre_guitar_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertSingle]") {
            input = read_single_track(input,
                                      pre_guitar_tracks[Difficulty::Expert]);
        } else if (header == "[EasyDoubleGuitar]") {
            input = read_single_track(input,
                                      pre_guitar_coop_tracks[Difficulty::Easy]);
        } else if (header == "[MediumDoubleGuitar]") {
            input = read_single_track(
                input, pre_guitar_coop_tracks[Difficulty::Medium]);
        } else if (header == "[HardDoubleGuitar]") {
            input = read_single_track(input,
                                      pre_guitar_coop_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertDoubleGuitar]") {
            input = read_single_track(
                input, pre_guitar_coop_tracks[Difficulty::Expert]);
        } else if (header == "[EasyDoubleBass]") {
            input = read_single_track(input, pre_bass_tracks[Difficulty::Easy]);
        } else if (header == "[MediumDoubleBass]") {
            input
                = read_single_track(input, pre_bass_tracks[Difficulty::Medium]);
        } else if (header == "[HardDoubleBass]") {
            input = read_single_track(input, pre_bass_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertDoubleBass]") {
            input
                = read_single_track(input, pre_bass_tracks[Difficulty::Expert]);
        } else if (header == "[EasyDoubleRhythm]") {
            input
                = read_single_track(input, pre_rhythm_tracks[Difficulty::Easy]);
        } else if (header == "[MediumDoubleRhythm]") {
            input = read_single_track(input,
                                      pre_rhythm_tracks[Difficulty::Medium]);
        } else if (header == "[HardDoubleRhythm]") {
            input
                = read_single_track(input, pre_rhythm_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertDoubleRhythm]") {
            input = read_single_track(input,
                                      pre_rhythm_tracks[Difficulty::Expert]);
        } else if (header == "[EasyKeyboard]") {
            input = read_single_track(input, pre_keys_tracks[Difficulty::Easy]);
        } else if (header == "[MediumKeyboard]") {
            input
                = read_single_track(input, pre_keys_tracks[Difficulty::Medium]);
        } else if (header == "[HardKeyboard]") {
            input = read_single_track(input, pre_keys_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertKeyboard]") {
            input
                = read_single_track(input, pre_keys_tracks[Difficulty::Expert]);
        } else if (header == "[EasyGHLGuitar]") {
            input = read_single_track(input,
                                      pre_ghl_guitar_tracks[Difficulty::Easy]);
        } else if (header == "[MediumGHLGuitar]") {
            input = read_single_track(
                input, pre_ghl_guitar_tracks[Difficulty::Medium]);
        } else if (header == "[HardGHLGuitar]") {
            input = read_single_track(input,
                                      pre_ghl_guitar_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertGHLGuitar]") {
            input = read_single_track(
                input, pre_ghl_guitar_tracks[Difficulty::Expert]);
        } else if (header == "[EasyGHLBass]") {
            input = read_single_track(input,
                                      pre_ghl_bass_tracks[Difficulty::Easy]);
        } else if (header == "[MediumGHLBass]") {
            input = read_single_track(input,
                                      pre_ghl_bass_tracks[Difficulty::Medium]);
        } else if (header == "[HardGHLBass]") {
            input = read_single_track(input,
                                      pre_ghl_bass_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertGHLBass]") {
            input = read_single_track(input,
                                      pre_ghl_bass_tracks[Difficulty::Expert]);
        } else if (header == "[EasyDrums]") {
            input = read_single_track(input, pre_drum_tracks[Difficulty::Easy]);
        } else if (header == "[MediumDrums]") {
            input
                = read_single_track(input, pre_drum_tracks[Difficulty::Medium]);
        } else if (header == "[HardDrums]") {
            input = read_single_track(input, pre_drum_tracks[Difficulty::Hard]);
        } else if (header == "[ExpertDrums]") {
            input
                = read_single_track(input, pre_drum_tracks[Difficulty::Expert]);
        } else {
            input = skip_section(input);
        }
    }

    song.m_resolution = pre_song_header.resolution;
    song.m_song_header.name = pre_song_header.name;
    song.m_song_header.artist = pre_song_header.artist;
    song.m_song_header.charter = pre_song_header.charter;

    song.m_sync_track = SyncTrack(std::move(pre_sync_track.time_sigs),
                                  std::move(pre_sync_track.bpms));
    song.m_guitar_note_tracks
        = tracks_from_pre_tracks(std::move(pre_guitar_tracks));
    song.m_guitar_coop_note_tracks
        = tracks_from_pre_tracks(std::move(pre_guitar_coop_tracks));
    song.m_bass_note_tracks
        = tracks_from_pre_tracks(std::move(pre_bass_tracks));
    song.m_rhythm_note_tracks
        = tracks_from_pre_tracks(std::move(pre_rhythm_tracks));
    song.m_keys_note_tracks
        = tracks_from_pre_tracks(std::move(pre_keys_tracks));
    song.m_ghl_guitar_note_tracks
        = tracks_from_pre_tracks(std::move(pre_ghl_guitar_tracks));
    song.m_ghl_bass_note_tracks
        = tracks_from_pre_tracks(std::move(pre_ghl_bass_tracks));
    song.m_drum_note_tracks
        = tracks_from_pre_tracks(std::move(pre_drum_tracks));

    if (song.m_guitar_note_tracks.empty()
        && song.m_guitar_coop_note_tracks.empty()
        && song.m_bass_note_tracks.empty() && song.m_rhythm_note_tracks.empty()
        && song.m_keys_note_tracks.empty()
        && song.m_ghl_guitar_note_tracks.empty()
        && song.m_ghl_bass_note_tracks.empty()
        && song.m_drum_note_tracks.empty()) {
        throw std::invalid_argument("Chart has no notes");
    }

    return song;
}

static std::string
get_with_default(const std::map<std::string, std::string>& map,
                 const std::string& key, std::string default_value)
{
    const auto iter = map.find(key);
    if (iter == map.end()) {
        return default_value;
    }
    return iter->second;
}

Song Song::from_chart(const Chart& chart)
{
    Song song;

    for (const auto& section : chart.sections) {
        if (section.name == "Song") {
            song.m_resolution = std::stoi(
                get_with_default(section.key_value_pairs, "Resolution", "192"));
            song.m_song_header.name = trim_quotes(get_with_default(
                section.key_value_pairs, "Name", "Unknown Song"));
            song.m_song_header.artist = trim_quotes(get_with_default(
                section.key_value_pairs, "Artist", "Unknown Artist"));
            song.m_song_header.charter = trim_quotes(get_with_default(
                section.key_value_pairs, "Charter", "Unknown Charter"));
        }
    }

    return song;
}

static bool has_track_title(const MidiTrack& track,
                            std::string_view desired_title)
{
    if (track.events.empty()) {
        return false;
    }
    const auto* meta_event = std::get_if<MetaEvent>(&track.events[0].event);
    if (meta_event == nullptr) {
        return false;
    }
    if (meta_event->type != 3) {
        return false;
    }
    return std::equal(meta_event->data.cbegin(), meta_event->data.cend(),
                      desired_title.cbegin(), desired_title.cend());
}

template <typename T>
static std::optional<Difficulty> difficulty_from_key(std::uint8_t key)
{
    if constexpr (std::is_same_v<
                      T, NoteColour> || std::is_same_v<T, DrumNoteColour>) {
        constexpr int EASY_GREEN = 60;
        constexpr int EASY_ORANGE = 64;
        constexpr int EXPERT_GREEN = 96;
        constexpr int EXPERT_ORANGE = 100;
        constexpr int HARD_GREEN = 84;
        constexpr int HARD_ORANGE = 88;
        constexpr int MEDIUM_GREEN = 72;
        constexpr int MEDIUM_ORANGE = 76;

        if (key >= EXPERT_GREEN && key <= EXPERT_ORANGE) {
            return {Difficulty::Expert};
        }
        if (key >= HARD_GREEN && key <= HARD_ORANGE) {
            return {Difficulty::Hard};
        }
        if (key >= MEDIUM_GREEN && key <= MEDIUM_ORANGE) {
            return {Difficulty::Medium};
        }
        if (key >= EASY_GREEN && key <= EASY_ORANGE) {
            return {Difficulty::Easy};
        }
        return {};
    } else if constexpr (std::is_same_v<T, GHLNoteColour>) {
        constexpr int EASY_OPEN = 58;
        constexpr int EASY_BLACK_HIGH = 64;
        constexpr int EXPERT_OPEN = 94;
        constexpr int EXPERT_BLACK_HIGH = 100;
        constexpr int HARD_OPEN = 82;
        constexpr int HARD_BLACK_HIGH = 88;
        constexpr int MEDIUM_OPEN = 70;
        constexpr int MEDIUM_BLACK_HIGH = 76;

        if (key >= EXPERT_OPEN && key <= EXPERT_BLACK_HIGH) {
            return {Difficulty::Expert};
        }
        if (key >= HARD_OPEN && key <= HARD_BLACK_HIGH) {
            return {Difficulty::Hard};
        }
        if (key >= MEDIUM_OPEN && key <= MEDIUM_BLACK_HIGH) {
            return {Difficulty::Medium};
        }
        if (key >= EASY_OPEN && key <= EASY_BLACK_HIGH) {
            return {Difficulty::Easy};
        }
        return {};
    }
}

template <typename T> static T colour_from_key(std::uint8_t key)
{
    if constexpr (std::is_same_v<T, NoteColour>) {
        constexpr int EASY_GREEN = 60;
        constexpr int EASY_ORANGE = 64;
        constexpr int EXPERT_GREEN = 96;
        constexpr int EXPERT_ORANGE = 100;
        constexpr int HARD_GREEN = 84;
        constexpr int HARD_ORANGE = 88;
        constexpr int MEDIUM_GREEN = 72;
        constexpr int MEDIUM_ORANGE = 76;

        constexpr std::array<NoteColour, 5> NOTE_COLOURS {
            NoteColour::Green, NoteColour::Red, NoteColour::Yellow,
            NoteColour::Blue, NoteColour::Orange};

        if (key >= EXPERT_GREEN && key <= EXPERT_ORANGE) {
            key -= EXPERT_GREEN;
        } else if (key >= HARD_GREEN && key <= HARD_ORANGE) {
            key -= HARD_GREEN;
        } else if (key >= MEDIUM_GREEN && key <= MEDIUM_ORANGE) {
            key -= MEDIUM_GREEN;
        } else if (key >= EASY_GREEN && key <= EASY_ORANGE) {
            key -= EASY_GREEN;
        } else {
            throw std::invalid_argument("Invalid key for note");
        }

        return NOTE_COLOURS.at(key);
    } else if constexpr (std::is_same_v<T, GHLNoteColour>) {
        constexpr int EASY_OPEN = 58;
        constexpr int EASY_BLACK_HIGH = 64;
        constexpr int EXPERT_OPEN = 94;
        constexpr int EXPERT_BLACK_HIGH = 100;
        constexpr int HARD_OPEN = 82;
        constexpr int HARD_BLACK_HIGH = 88;
        constexpr int MEDIUM_OPEN = 70;
        constexpr int MEDIUM_BLACK_HIGH = 76;

        constexpr std::array<GHLNoteColour, 7> GHL_NOTE_COLOURS {
            GHLNoteColour::Open,     GHLNoteColour::WhiteLow,
            GHLNoteColour::WhiteMid, GHLNoteColour::WhiteHigh,
            GHLNoteColour::BlackLow, GHLNoteColour::BlackMid,
            GHLNoteColour::BlackHigh};

        if (key >= EXPERT_OPEN && key <= EXPERT_BLACK_HIGH) {
            key -= EXPERT_OPEN;
        } else if (key >= HARD_OPEN && key <= HARD_BLACK_HIGH) {
            key -= HARD_OPEN;
        } else if (key >= MEDIUM_OPEN && key <= MEDIUM_BLACK_HIGH) {
            key -= MEDIUM_OPEN;
        } else if (key >= EASY_OPEN && key <= EASY_BLACK_HIGH) {
            key -= EASY_OPEN;
        } else {
            throw std::invalid_argument("Invalid key for note");
        }

        return GHL_NOTE_COLOURS.at(key);
    } else if constexpr (std::is_same_v<T, DrumNoteColour>) {
        constexpr int EASY_KICK = 60;
        constexpr int EASY_GREEN = 64;
        constexpr int EXPERT_KICK = 96;
        constexpr int EXPERT_GREEN = 100;
        constexpr int HARD_KICK = 84;
        constexpr int HARD_GREEN = 88;
        constexpr int MEDIUM_KICK = 72;
        constexpr int MEDIUM_GREEN = 76;

        constexpr std::array<DrumNoteColour, 5> DRUM_NOTE_COLOURS {
            DrumNoteColour::Kick, DrumNoteColour::Red,
            DrumNoteColour::YellowCymbal, DrumNoteColour::BlueCymbal,
            DrumNoteColour::GreenCymbal};

        if (key >= EXPERT_KICK && key <= EXPERT_GREEN) {
            key -= EXPERT_KICK;
        } else if (key >= HARD_KICK && key <= HARD_GREEN) {
            key -= HARD_KICK;
        } else if (key >= MEDIUM_KICK && key <= MEDIUM_GREEN) {
            key -= MEDIUM_KICK;
        } else if (key >= EASY_KICK && key <= EASY_GREEN) {
            key -= EASY_KICK;
        } else {
            throw std::invalid_argument("Invalid key for note");
        }

        return DRUM_NOTE_COLOURS.at(key);
    }
}

static bool is_open_event_sysex(const SysexEvent& event)
{
    constexpr std::array<std::tuple<std::size_t, int>, 6> REQUIRED_BYTES {
        std::tuple {0, 0x50}, {1, 0x53}, {2, 0}, {3, 0}, {5, 1}, {7, 0xF7}};
    constexpr std::array<std::tuple<std::size_t, int>, 2> UPPER_BOUNDS {
        std::tuple {4, 3}, {6, 1}};
    constexpr int SYSEX_DATA_SIZE = 8;

    if (event.data.size() != SYSEX_DATA_SIZE) {
        return false;
    }
    for (const auto& pair : REQUIRED_BYTES) {
        if (event.data[std::get<0>(pair)] != std::get<1>(pair)) {
            return false;
        }
    }
    for (const auto& pair : UPPER_BOUNDS) {
        if (event.data[std::get<0>(pair)] > std::get<1>(pair)) {
            return false;
        }
    }
    return true;
}

static std::tuple<std::string, SyncTrack>
read_first_midi_track(const MidiTrack& track)
{
    constexpr int SET_TEMPO_ID = 0x51;
    constexpr int TEXT_EVENT_ID = 1;
    constexpr int TIME_SIG_ID = 0x58;

    std::vector<BPM> tempos;
    std::vector<TimeSignature> time_sigs;
    std::string name {"Unknown Song"};
    for (const auto& event : track.events) {
        const auto* meta_event = std::get_if<MetaEvent>(&event.event);
        if (meta_event == nullptr) {
            continue;
        }
        switch (meta_event->type) {
        case TEXT_EVENT_ID:
            name = std::string {meta_event->data.cbegin(),
                                meta_event->data.cend()};
            break;
        case SET_TEMPO_ID: {
            const auto us_per_quarter = meta_event->data[0] << 16
                | meta_event->data[1] << 8 | meta_event->data[2];
            const auto bpm = 60000000000 / us_per_quarter;
            tempos.push_back({event.time, static_cast<int>(bpm)});
            break;
        }
        case TIME_SIG_ID:
            time_sigs.push_back(
                {event.time, meta_event->data[0], 1 << meta_event->data[1]});
            break;
        }
    }

    SyncTrack sync_track {std::move(time_sigs), std::move(tempos)};
    return {name, sync_track};
}

template <typename T> struct InstrumentMidiTrack {
    std::map<std::tuple<Difficulty, T>, std::vector<int>> note_on_events;
    std::map<std::tuple<Difficulty, T>, std::vector<int>> note_off_events;
    std::map<Difficulty, std::vector<int>> open_on_events;
    std::map<Difficulty, std::vector<int>> open_off_events;
    std::vector<int> yellow_tom_on_events;
    std::vector<int> yellow_tom_off_events;
    std::vector<int> blue_tom_on_events;
    std::vector<int> blue_tom_off_events;
    std::vector<int> green_tom_on_events;
    std::vector<int> green_tom_off_events;
    std::vector<int> solo_on_events;
    std::vector<int> solo_off_events;
    std::vector<int> sp_on_events;
    std::vector<int> sp_off_events;
};

template <typename T>
static void add_sysex_event(InstrumentMidiTrack<T>& track,
                            const SysexEvent& event, int time)
{
    constexpr std::array<Difficulty, 4> OPEN_EVENT_DIFFS {
        Difficulty::Easy, Difficulty::Medium, Difficulty::Hard,
        Difficulty::Expert};
    constexpr int SYSEX_ON_INDEX = 6;

    if (!is_open_event_sysex(event)) {
        return;
    }
    Difficulty diff = OPEN_EVENT_DIFFS.at(event.data[4]);
    if (event.data[SYSEX_ON_INDEX] == 0) {
        track.open_off_events[diff].push_back(time);
    } else {
        track.open_on_events[diff].push_back(time);
    }
}

template <typename T>
static void add_note_off_event(InstrumentMidiTrack<T>& track,
                               const std::array<std::uint8_t, 2>& data,
                               int time)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;

    const auto diff = difficulty_from_key<T>(data[0]);
    if (diff.has_value()) {
        const auto colour = colour_from_key<T>(data[0]);
        track.note_off_events[{*diff, colour}].push_back(time);
    } else if (data[0] == YELLOW_TOM_ID) {
        track.yellow_tom_off_events.push_back(time);
    } else if (data[0] == BLUE_TOM_ID) {
        track.blue_tom_off_events.push_back(time);
    } else if (data[0] == GREEN_TOM_ID) {
        track.green_tom_off_events.push_back(time);
    } else if (data[0] == SOLO_NOTE_ID) {
        track.solo_off_events.push_back(time);
    } else if (data[0] == SP_NOTE_ID) {
        track.sp_off_events.push_back(time);
    }
}

template <typename T>
static void add_note_on_event(InstrumentMidiTrack<T>& track,
                              const std::array<std::uint8_t, 2>& data, int time)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;

    // Velocity 0 Note On events are counted as Note Off events.
    if (data[1] == 0) {
        add_note_off_event(track, data, time);
        return;
    }

    const auto diff = difficulty_from_key<T>(data[0]);
    if (diff.has_value()) {
        const auto colour = colour_from_key<T>(data[0]);
        track.note_on_events[{*diff, colour}].push_back(time);
    } else if (data[0] == YELLOW_TOM_ID) {
        track.yellow_tom_on_events.push_back(time);
    } else if (data[0] == BLUE_TOM_ID) {
        track.blue_tom_on_events.push_back(time);
    } else if (data[0] == GREEN_TOM_ID) {
        track.green_tom_on_events.push_back(time);
    } else if (data[0] == SOLO_NOTE_ID) {
        track.solo_on_events.push_back(time);
    } else if (data[0] == SP_NOTE_ID) {
        track.sp_on_events.push_back(time);
    }
}

template <typename T>
static InstrumentMidiTrack<T>
read_instrument_midi_track(const MidiTrack& midi_track)
{
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    InstrumentMidiTrack<T> event_track;

    for (const auto& event : midi_track.events) {
        const auto* midi_event = std::get_if<MidiEvent>(&event.event);
        if (midi_event == nullptr) {
            const auto* sysex_event = std::get_if<SysexEvent>(&event.event);
            if (sysex_event != nullptr) {
                add_sysex_event(event_track, *sysex_event, event.time);
            }
            continue;
        }
        switch (midi_event->status & UPPER_NIBBLE_MASK) {
        case NOTE_OFF_ID:
            add_note_off_event(event_track, midi_event->data, event.time);
            break;
        case NOTE_ON_ID:
            add_note_on_event(event_track, midi_event->data, event.time);
            break;
        }
    }

    return event_track;
}

static std::map<Difficulty, NoteTrack<NoteColour>>
note_tracks_from_midi(const MidiTrack& midi_track, int resolution)
{
    constexpr int DEFAULT_RESOLUTION = 192;
    constexpr int DEFAULT_SUST_CUTOFF = 64;

    const auto event_track = read_instrument_midi_track<NoteColour>(midi_track);

    std::map<Difficulty, std::vector<std::tuple<int, int>>> open_events;
    for (const auto& [diff, open_ons] : event_track.open_on_events) {
        const auto& open_offs = event_track.open_off_events.at(diff);
        open_events[diff] = combine_on_off_events(open_ons, open_offs);
    }

    std::map<Difficulty, std::vector<Note<NoteColour>>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour] = key;
        const auto& note_offs = event_track.note_off_events.at(key);
        for (const auto& [pos, end] :
             combine_on_off_events(note_ons, note_offs)) {
            auto note_length = end - pos;
            if (note_length
                <= (DEFAULT_SUST_CUTOFF * resolution) / DEFAULT_RESOLUTION) {
                note_length = 0;
            }
            auto note_colour = colour;
            for (const auto& [open_start, open_end] : open_events[diff]) {
                if (pos >= open_start && pos < open_end) {
                    note_colour = NoteColour::Open;
                }
            }
            notes[diff].push_back({pos, note_length, note_colour});
        }
    }

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({start, end - start});
    }

    std::map<Difficulty, NoteTrack<NoteColour>> note_tracks;
    for (const auto& [diff, note_set] : notes) {
        auto solos = form_solo_vector(event_track.solo_on_events,
                                      event_track.solo_off_events, note_set);
        note_tracks[diff] = {note_set, sp_phrases, solos};
    }

    return note_tracks;
}

static std::map<Difficulty, NoteTrack<GHLNoteColour>>
ghl_note_tracks_from_midi(const MidiTrack& midi_track, int resolution)
{
    constexpr int DEFAULT_RESOLUTION = 192;
    constexpr int DEFAULT_SUST_CUTOFF = 64;

    const auto event_track
        = read_instrument_midi_track<GHLNoteColour>(midi_track);

    std::map<Difficulty, std::vector<Note<GHLNoteColour>>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour] = key;
        const auto& note_offs = event_track.note_off_events.at(key);
        for (const auto& [pos, end] :
             combine_on_off_events(note_ons, note_offs)) {
            auto note_length = end - pos;
            if (note_length
                <= (DEFAULT_SUST_CUTOFF * resolution) / DEFAULT_RESOLUTION) {
                note_length = 0;
            }
            notes[diff].push_back({pos, note_length, colour});
        }
    }

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({start, end - start});
    }

    std::map<Difficulty, NoteTrack<GHLNoteColour>> note_tracks;
    for (const auto& [diff, note_set] : notes) {
        auto solos = form_solo_vector(event_track.solo_on_events,
                                      event_track.solo_off_events, note_set);
        note_tracks[diff] = {note_set, sp_phrases, solos};
    }

    return note_tracks;
}

static std::map<Difficulty, NoteTrack<DrumNoteColour>>
drum_note_tracks_from_midi(const MidiTrack& midi_track)
{
    const auto event_track
        = read_instrument_midi_track<DrumNoteColour>(midi_track);

    const auto yellow_tom_events = combine_on_off_events(
        event_track.yellow_tom_on_events, event_track.yellow_tom_off_events);
    const auto blue_tom_events = combine_on_off_events(
        event_track.blue_tom_on_events, event_track.blue_tom_off_events);
    const auto green_tom_events = combine_on_off_events(
        event_track.green_tom_on_events, event_track.green_tom_off_events);

    std::map<Difficulty, std::vector<Note<DrumNoteColour>>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour] = key;
        const auto& note_offs = event_track.note_off_events.at(key);
        for (const auto& [pos, end] :
             combine_on_off_events(note_ons, note_offs)) {
            auto note_colour = colour;
            if (note_colour == DrumNoteColour::YellowCymbal) {
                for (const auto& [open_start, open_end] : yellow_tom_events) {
                    if (pos >= open_start && pos < open_end) {
                        note_colour = DrumNoteColour::Yellow;
                    }
                }
            } else if (note_colour == DrumNoteColour::BlueCymbal) {
                for (const auto& [open_start, open_end] : blue_tom_events) {
                    if (pos >= open_start && pos < open_end) {
                        note_colour = DrumNoteColour::Blue;
                    }
                }
            } else if (note_colour == DrumNoteColour::GreenCymbal) {
                for (const auto& [open_start, open_end] : green_tom_events) {
                    if (pos >= open_start && pos < open_end) {
                        note_colour = DrumNoteColour::Green;
                    }
                }
            }
            notes[diff].push_back({pos, 0, note_colour});
        }
    }

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({start, end - start});
    }

    std::map<Difficulty, NoteTrack<DrumNoteColour>> note_tracks;
    for (const auto& [diff, note_set] : notes) {
        auto solos = form_solo_vector(event_track.solo_on_events,
                                      event_track.solo_off_events, note_set);
        note_tracks[diff] = {note_set, sp_phrases, solos};
    }

    return note_tracks;
}

Song Song::from_midi(const Midi& midi)
{
    if (midi.ticks_per_quarter_note == 0) {
        throw std::invalid_argument("Resolution must be > 0");
    }

    Song song;
    song.m_resolution = midi.ticks_per_quarter_note;

    if (midi.tracks.empty()) {
        return song;
    }

    const auto& [name, sync_track] = read_first_midi_track(midi.tracks[0]);
    song.m_song_header.name = name;
    song.m_sync_track = sync_track;

    for (const auto& track : midi.tracks) {
        if (has_track_title(track, "PART GUITAR")
            || has_track_title(track, "T1 GEMS")) {
            song.m_guitar_note_tracks
                = note_tracks_from_midi(track, song.m_resolution);
        } else if (has_track_title(track, "PART GUITAR COOP")) {
            song.m_guitar_coop_note_tracks
                = note_tracks_from_midi(track, song.m_resolution);
        } else if (has_track_title(track, "PART BASS")) {
            song.m_bass_note_tracks
                = note_tracks_from_midi(track, song.m_resolution);
        } else if (has_track_title(track, "PART RHYTHM")) {
            song.m_rhythm_note_tracks
                = note_tracks_from_midi(track, song.m_resolution);
        } else if (has_track_title(track, "PART KEYS")) {
            song.m_keys_note_tracks
                = note_tracks_from_midi(track, song.m_resolution);
        } else if (has_track_title(track, "PART GUITAR GHL")) {
            song.m_ghl_guitar_note_tracks
                = ghl_note_tracks_from_midi(track, song.m_resolution);
        } else if (has_track_title(track, "PART BASS GHL")) {
            song.m_ghl_bass_note_tracks
                = ghl_note_tracks_from_midi(track, song.m_resolution);
        } else if (has_track_title(track, "PART DRUMS")) {
            song.m_drum_note_tracks = drum_note_tracks_from_midi(track);
        }
    }

    return song;
}
