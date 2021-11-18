/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021 Raymond Wright
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

#include <climits>
#include <filesystem>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <type_traits>

#include "nowide_wrapper.hpp"

#include "song.hpp"

static bool starts_with_prefix(const std::string& string,
                               std::string_view prefix)
{
    return string.substr(0, prefix.size()) == prefix;
}

static bool ends_with_suffix(const std::string& string, std::string_view suffix)
{
    if (string.size() < suffix.size()) {
        return false;
    }
    return string.substr(string.size() - suffix.size()) == suffix;
}

Song Song::from_filename(const std::string& filename)
{
    std::string ini_file;
    const std::filesystem::path song_path {filename};
    const auto song_directory = song_path.parent_path();
    const auto ini_path = song_directory / "song.ini";
    nowide::ifstream ini_in {ini_path.string()};
    if (ini_in.is_open()) {
        ini_file = std::string {std::istreambuf_iterator<char>(ini_in),
                                std::istreambuf_iterator<char>()};
    }
    const auto ini = parse_ini(ini_file);

    if (ends_with_suffix(filename, ".chart")) {
        nowide::ifstream in {filename};
        if (!in.is_open()) {
            throw std::invalid_argument("File did not open");
        }
        std::string contents {std::istreambuf_iterator<char>(in),
                              std::istreambuf_iterator<char>()};
        return Song::from_chart(parse_chart(contents), ini);
    }
    if (ends_with_suffix(filename, ".mid")) {
        nowide::ifstream in {filename, std::ios::binary};
        if (!in.is_open()) {
            throw std::invalid_argument("File did not open");
        }
        std::vector<std::uint8_t> buffer {std::istreambuf_iterator<char>(in),
                                          std::istreambuf_iterator<char>()};
        return Song::from_midi(parse_midi(buffer), ini);
    }
    throw std::invalid_argument("file should be .chart or .mid");
}

// Takes a sequence of points where some note type/event is turned on, and a
// sequence where said type is turned off, and returns a tuple of intervals
// where the event is on.
static std::vector<std::tuple<int, int>>
combine_solo_events(const std::vector<int>& on_events,
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

    return ranges;
}

template <typename T>
static std::vector<Solo>
form_solo_vector(const std::vector<int>& solo_on_events,
                 const std::vector<int>& solo_off_events,
                 const std::vector<Note<T>>& notes, bool is_midi)
{
    constexpr int SOLO_NOTE_VALUE = 100;

    std::vector<Solo> solos;

    for (auto [start, end] :
         combine_solo_events(solo_on_events, solo_off_events)) {
        std::set<int> positions_in_solo;
        auto note_count = 0;
        for (const auto& note : notes) {
            if ((note.position >= start && note.position < end)
                || (note.position == end && !is_midi)) {
                positions_in_solo.insert(note.position);
                ++note_count;
            }
        }
        if (positions_in_solo.empty()) {
            continue;
        }
        if constexpr (!std::is_same_v<T, DrumNoteColour>) {
            note_count = static_cast<int>(positions_in_solo.size());
        }
        solos.push_back({start, end, SOLO_NOTE_VALUE * note_count});
    }

    return solos;
}

template <typename T>
static std::optional<Note<T>> note_from_note_colour(int position, int length,
                                                    int fret_type)
{
    if constexpr (std::is_same_v<T, NoteColour>) {
        static const std::array<std::optional<NoteColour>, 8> COLOURS {
            NoteColour::Green, NoteColour::Red,    NoteColour::Yellow,
            NoteColour::Blue,  NoteColour::Orange, std::nullopt,
            std::nullopt,      NoteColour::Open};
        if (static_cast<std::size_t>(fret_type) >= COLOURS.size()) {
            return std::nullopt;
        }
        const auto colour = COLOURS.at(static_cast<std::size_t>(fret_type));
        if (!colour.has_value()) {
            return std::nullopt;
        }
        return Note<NoteColour> {position, length, *colour};
    } else if constexpr (std::is_same_v<T, GHLNoteColour>) {
        static const std::array<std::optional<GHLNoteColour>, 9> COLOURS {
            GHLNoteColour::WhiteLow,
            GHLNoteColour::WhiteMid,
            GHLNoteColour::WhiteHigh,
            GHLNoteColour::BlackLow,
            GHLNoteColour::BlackMid,
            std::nullopt,
            std::nullopt,
            GHLNoteColour::Open,
            GHLNoteColour::BlackHigh};
        if (static_cast<std::size_t>(fret_type) >= COLOURS.size()) {
            return std::nullopt;
        }
        const auto colour = COLOURS.at(static_cast<std::size_t>(fret_type));
        if (!colour.has_value()) {
            return std::nullopt;
        }
        return Note<GHLNoteColour> {position, length, *colour};
    } else if constexpr (std::is_same_v<T, DrumNoteColour>) {
        const std::map<int, DrumNoteColour> COLOURS {
            {0, DrumNoteColour::Kick},
            {1, DrumNoteColour::Red},
            {2, DrumNoteColour::Yellow},
            {3, DrumNoteColour::Blue},
            {4, DrumNoteColour::Green},
            {32, DrumNoteColour::DoubleKick},
            {66, DrumNoteColour::YellowCymbal},
            {67, DrumNoteColour::BlueCymbal},
            {68, DrumNoteColour::GreenCymbal}};
        (void)length;
        const auto colour_iter = COLOURS.find(fret_type);
        if (colour_iter == COLOURS.end()) {
            return std::nullopt;
        }
        return Note<DrumNoteColour> {position, 0, colour_iter->second};
    }
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

static std::optional<std::tuple<Difficulty, Instrument>>
diff_inst_from_header(const std::string& header)
{
    using namespace std::literals;

    static const std::array<std::tuple<std::string_view, Difficulty>, 4>
        DIFFICULTIES {std::tuple {"Easy"sv, Difficulty::Easy},
                      {"Medium"sv, Difficulty::Medium},
                      {"Hard"sv, Difficulty::Hard},
                      {"Expert"sv, Difficulty::Expert}};
    static const std::array<std::tuple<std::string_view, Instrument>, 8>
        INSTRUMENTS {std::tuple {"Single"sv, Instrument::Guitar},
                     {"DoubleGuitar"sv, Instrument::GuitarCoop},
                     {"DoubleBass"sv, Instrument::Bass},
                     {"DoubleRhythm"sv, Instrument::Rhythm},
                     {"Keyboard"sv, Instrument::Keys},
                     {"GHLGuitar"sv, Instrument::GHLGuitar},
                     {"GHLBass"sv, Instrument::GHLBass},
                     {"Drums"sv, Instrument::Drums}};
    // NOLINT is required because following clang-tidy here causes the VS2017
    // compile to fail.
    auto diff_iter = std::find_if( // NOLINT
        DIFFICULTIES.cbegin(), DIFFICULTIES.cend(), [&](const auto& pair) {
            return starts_with_prefix(header, std::get<0>(pair));
        });
    if (diff_iter == DIFFICULTIES.cend()) {
        return std::nullopt;
    }
    auto inst_iter = std::find_if( // NOLINT
        INSTRUMENTS.cbegin(), INSTRUMENTS.cend(), [&](const auto& pair) {
            return ends_with_suffix(header, std::get<0>(pair));
        });
    if (inst_iter == INSTRUMENTS.cend()) {
        return std::nullopt;
    }
    return std::tuple {std::get<1>(*diff_iter), std::get<1>(*inst_iter)};
}

static std::vector<Note<DrumNoteColour>>
add_fifth_lane_greens(std::vector<Note<DrumNoteColour>> notes,
                      const std::vector<NoteEvent>& note_events)
{
    constexpr int FIVE_LANE_GREEN = 5;

    std::set<int> green_positions;
    for (const auto& note : notes) {
        if (note.colour == DrumNoteColour::Green) {
            green_positions.insert(note.position);
        }
    }
    for (const auto& note_event : note_events) {
        if (note_event.fret != FIVE_LANE_GREEN) {
            continue;
        }
        if (green_positions.count(note_event.position) != 0) {
            notes.push_back({note_event.position, 0, DrumNoteColour::Blue});
        } else {
            notes.push_back({note_event.position, 0, DrumNoteColour::Green});
        }
    }
    return notes;
}

static std::vector<Note<DrumNoteColour>>
apply_cymbal_events(const std::vector<Note<DrumNoteColour>>& notes)
{
    std::set<unsigned int> deletion_spots;
    for (auto i = 0U; i < notes.size(); ++i) {
        const auto& cymbal_note = notes[i];
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
        case DrumNoteColour::DoubleKick:
            continue;
        }
        bool delete_cymbal = true;
        for (auto j = 0U; j < notes.size(); ++j) {
            const auto& non_cymbal_note = notes[j];
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
    for (auto i = 0U; i < notes.size(); ++i) {
        if (deletion_spots.count(i) == 0) {
            new_notes.push_back(notes[i]);
        }
    }
    return new_notes;
}

template <typename T>
static NoteTrack<T> note_track_from_section(const ChartSection& section,
                                            int resolution)
{
    constexpr int DISCO_FLIP_START_SIZE = 13;
    constexpr int DISCO_FLIP_END_SIZE = 12;
    constexpr int DRUM_FILL_KEY = 64;
    constexpr std::array<std::uint8_t, 4> MIX {{'m', 'i', 'x', '_'}};
    constexpr std::array<std::uint8_t, 6> DRUMS {
        {'_', 'd', 'r', 'u', 'm', 's'}};

    std::vector<Note<T>> notes;
    for (const auto& note_event : section.note_events) {
        const auto note = note_from_note_colour<T>(
            note_event.position, note_event.length, note_event.fret);
        if (note.has_value()) {
            notes.push_back(*note);
        }
    }
    if constexpr (std::is_same_v<T, DrumNoteColour>) {
        notes = add_fifth_lane_greens(std::move(notes), section.note_events);
        notes = apply_cymbal_events(notes);
    }

    std::vector<DrumFill> fills;
    std::vector<StarPower> sp;
    for (const auto& phrase : section.special_events) {
        if (phrase.key == 2) {
            sp.push_back(StarPower {phrase.position, phrase.length});
        } else if (phrase.key == DRUM_FILL_KEY) {
            fills.push_back(DrumFill {phrase.position, phrase.length});
        }
    }
    if constexpr (!std::is_same_v<T, DrumNoteColour>) {
        fills.clear();
        fills.shrink_to_fit();
    }

    std::vector<int> solo_on_events;
    std::vector<int> solo_off_events;
    std::vector<int> disco_flip_on_events;
    std::vector<int> disco_flip_off_events;
    for (const auto& event : section.events) {
        if (event.data == "solo") {
            solo_on_events.push_back(event.position);
        } else if (event.data == "soloend") {
            solo_off_events.push_back(event.position);
        } else if (event.data.size() >= DISCO_FLIP_END_SIZE) {
            if (!std::equal(MIX.cbegin(), MIX.cend(), event.data.cbegin())
                || !std::equal(DRUMS.cbegin(), DRUMS.cend(),
                               event.data.cbegin() + MIX.size() + 1)) {
                continue;
            }
            if (event.data.size() == DISCO_FLIP_END_SIZE) {
                disco_flip_off_events.push_back(event.position);
            } else if (event.data.size() == DISCO_FLIP_START_SIZE
                       && event.data.back() == 'd') {
                disco_flip_on_events.push_back(event.position);
            }
        }
    }
    std::sort(solo_on_events.begin(), solo_on_events.end());
    std::sort(solo_off_events.begin(), solo_off_events.end());
    auto solos
        = form_solo_vector(solo_on_events, solo_off_events, notes, false);
    std::sort(disco_flip_on_events.begin(), disco_flip_on_events.end());
    std::sort(disco_flip_off_events.begin(), disco_flip_off_events.end());
    std::vector<DiscoFlip> disco_flips;
    for (auto [start, end] :
         combine_solo_events(disco_flip_on_events, disco_flip_off_events)) {
        disco_flips.push_back({start, end - start});
    }

    return NoteTrack<T> {
        std::move(notes), std::move(sp),          std::move(solos),
        std::move(fills), std::move(disco_flips), {},
        resolution};
}

std::vector<Instrument> Song::instruments() const
{
    std::set<Instrument> instrument_set;
    for (const auto& [key, val] : m_five_fret_tracks) {
        instrument_set.insert(std::get<0>(key));
    }
    for (const auto& [key, val] : m_six_fret_tracks) {
        instrument_set.insert(std::get<0>(key));
    }
    if (!m_drum_note_tracks.empty()) {
        instrument_set.insert(Instrument::Drums);
    }

    std::vector<Instrument> instruments {instrument_set.cbegin(),
                                         instrument_set.cend()};
    std::sort(instruments.begin(), instruments.end());
    return instruments;
}

static bool is_six_fret_instrument(Instrument instrument)
{
    return instrument == Instrument::GHLGuitar
        || instrument == Instrument::GHLBass;
}

std::vector<Difficulty> Song::difficulties(Instrument instrument) const
{
    std::vector<Difficulty> difficulties;
    if (instrument == Instrument::Drums) {
        for (const auto& [key, val] : m_drum_note_tracks) {
            difficulties.push_back(key);
        }
    } else if (is_six_fret_instrument(instrument)) {
        for (const auto& [key, val] : m_six_fret_tracks) {
            if (std::get<0>(key) == instrument) {
                difficulties.push_back(std::get<1>(key));
            }
        }
    } else {
        for (const auto& [key, val] : m_five_fret_tracks) {
            if (std::get<0>(key) == instrument) {
                difficulties.push_back(std::get<1>(key));
            }
        }
    }
    std::sort(difficulties.begin(), difficulties.end());
    return difficulties;
}

static SyncTrack sync_track_from_section(const ChartSection& section)
{
    std::vector<BPM> bpms;
    for (const auto& bpm : section.bpm_events) {
        bpms.push_back({bpm.position, bpm.bpm});
    }
    std::vector<TimeSignature> tses;
    for (const auto& ts : section.ts_events) {
        if (static_cast<std::size_t>(ts.denominator)
            >= (CHAR_BIT * sizeof(int))) {
            throw ParseError("Invalid Time Signature denominator");
        }
        tses.push_back({ts.position, ts.numerator, 1 << ts.denominator});
    }
    return {std::move(tses), std::move(bpms)};
}

void Song::append_instrument_track(Instrument inst, Difficulty diff,
                                   const ChartSection& section)
{
    if (is_six_fret_instrument(inst)) {
        auto note_track
            = note_track_from_section<GHLNoteColour>(section, m_resolution);
        if (!note_track.notes().empty()) {
            m_six_fret_tracks.insert({{inst, diff}, note_track});
        }
    } else if (inst == Instrument::Drums) {
        auto note_track
            = note_track_from_section<DrumNoteColour>(section, m_resolution);
        if (!note_track.notes().empty()) {
            m_drum_note_tracks.insert({diff, note_track});
        }
    } else {
        auto note_track
            = note_track_from_section<NoteColour>(section, m_resolution);
        if (!note_track.notes().empty()) {
            m_five_fret_tracks.insert({{inst, diff}, note_track});
        }
    }
}

Song Song::from_chart(const Chart& chart, const IniValues& ini)
{
    Song song;

    song.m_name = ini.name;
    song.m_artist = ini.artist;
    song.m_charter = ini.charter;

    for (const auto& section : chart.sections) {
        if (section.name == "Song") {
            try {
                song.m_resolution = std::stoi(get_with_default(
                    section.key_value_pairs, "Resolution", "192"));
            } catch (const std::invalid_argument&) {
                // CH just ignores this kind of parsing mistake.
                // TODO: Use from_chars instead to avoid having to use
                // exceptions as control flow.
            }
        } else if (section.name == "SyncTrack") {
            song.m_sync_track = sync_track_from_section(section);
        } else {
            auto pair = diff_inst_from_header(section.name);
            if (!pair.has_value()) {
                continue;
            }
            auto [diff, inst] = *pair;
            song.append_instrument_track(inst, diff, section);
        }
    }

    if (song.m_five_fret_tracks.empty() && song.m_six_fret_tracks.empty()
        && song.m_drum_note_tracks.empty()) {
        throw ParseError("Chart has no notes");
    }

    return song;
}

// Like combine_solo_events, but never skips on events to suit Midi parsing and
// checks if there is an unmatched on event.
// The tuples are a pair of the form (position, rank), where events later in the
// file have a higher rank. This is in case of the Note Off event being right
// after the corresponding Note On event in the file, but at the same tick.
static std::vector<std::tuple<int, int>>
combine_note_on_off_events(const std::vector<std::tuple<int, int>>& on_events,
                           const std::vector<std::tuple<int, int>>& off_events)
{
    std::vector<std::tuple<int, int>> ranges;

    auto on_iter = on_events.cbegin();
    auto off_iter = off_events.cbegin();

    while (on_iter < on_events.cend() && off_iter < off_events.cend()) {
        if (*on_iter >= *off_iter) {
            ++off_iter;
            continue;
        }
        ranges.emplace_back(std::get<0>(*on_iter), std::get<0>(*off_iter));
        ++on_iter;
        ++off_iter;
    }

    if (on_iter != on_events.cend()) {
        throw ParseError("on event has no corresponding off event");
    }

    return ranges;
}

template <typename T>
static std::optional<Difficulty> difficulty_from_key(std::uint8_t key)
{
    if constexpr (std::is_same_v<T, NoteColour>) {
        constexpr std::array<std::tuple<int, int, Difficulty>, 4> diff_ranges {
            {{96, 100, Difficulty::Expert},
             {84, 88, Difficulty::Hard},
             {72, 76, Difficulty::Medium},
             {60, 64, Difficulty::Easy}}};

        for (const auto& [min, max, diff] : diff_ranges) {
            if (key >= min && key <= max) {
                return {diff};
            }
        }
    } else if constexpr (std::is_same_v<T, GHLNoteColour>) {
        constexpr std::array<std::tuple<int, int, Difficulty>, 4> diff_ranges {
            {{94, 100, Difficulty::Expert},
             {82, 88, Difficulty::Hard},
             {70, 76, Difficulty::Medium},
             {58, 64, Difficulty::Easy}}};

        for (const auto& [min, max, diff] : diff_ranges) {
            if (key >= min && key <= max) {
                return {diff};
            }
        }
    } else if constexpr (std::is_same_v<T, DrumNoteColour>) {
        constexpr std::array<std::tuple<int, int, Difficulty>, 4> diff_ranges {
            {{95, 101, Difficulty::Expert},
             {83, 89, Difficulty::Hard},
             {71, 77, Difficulty::Medium},
             {59, 65, Difficulty::Easy}}};

        for (const auto& [min, max, diff] : diff_ranges) {
            if (key >= min && key <= max) {
                return {diff};
            }
        }
    }

    return std::nullopt;
}

template <typename T, std::size_t N>
static T
colour_from_key_and_bounds(std::uint8_t key,
                           const std::array<unsigned int, 4>& diff_ranges,
                           const std::array<T, N>& colours)
{
    for (auto min : diff_ranges) {
        if (key >= min && (key - min) < colours.size()) {
            return colours.at(key - min);
        }
    }

    throw ParseError("Invalid key for note");
}

template <typename T>
static T colour_from_key(std::uint8_t key, bool from_five_lane)
{
    if constexpr (std::is_same_v<T, NoteColour>) {
        constexpr std::array<unsigned int, 4> diff_ranges {96, 84, 72, 60};
        constexpr std::array NOTE_COLOURS {NoteColour::Green, NoteColour::Red,
                                           NoteColour::Yellow, NoteColour::Blue,
                                           NoteColour::Orange};
        return colour_from_key_and_bounds(key, diff_ranges, NOTE_COLOURS);
    } else if constexpr (std::is_same_v<T, GHLNoteColour>) {
        constexpr std::array<unsigned int, 4> diff_ranges {94, 82, 70, 58};
        constexpr std::array GHL_NOTE_COLOURS {
            GHLNoteColour::Open,     GHLNoteColour::WhiteLow,
            GHLNoteColour::WhiteMid, GHLNoteColour::WhiteHigh,
            GHLNoteColour::BlackLow, GHLNoteColour::BlackMid,
            GHLNoteColour::BlackHigh};
        return colour_from_key_and_bounds(key, diff_ranges, GHL_NOTE_COLOURS);
    } else if constexpr (std::is_same_v<T, DrumNoteColour>) {
        constexpr std::array<unsigned int, 4> diff_ranges {95, 83, 71, 59};
        constexpr std::array DRUM_NOTE_COLOURS {
            DrumNoteColour::DoubleKick, DrumNoteColour::Kick,
            DrumNoteColour::Red,        DrumNoteColour::YellowCymbal,
            DrumNoteColour::BlueCymbal, DrumNoteColour::GreenCymbal};
        constexpr std::array FIVE_LANE_COLOURS {
            DrumNoteColour::DoubleKick, DrumNoteColour::Kick,
            DrumNoteColour::Red,        DrumNoteColour::YellowCymbal,
            DrumNoteColour::Blue,       DrumNoteColour::GreenCymbal,
            DrumNoteColour::Green};
        if (from_five_lane) {
            return colour_from_key_and_bounds(key, diff_ranges,
                                              FIVE_LANE_COLOURS);
        }
        return colour_from_key_and_bounds(key, diff_ranges, DRUM_NOTE_COLOURS);
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
    if (std::any_of(REQUIRED_BYTES.cbegin(), REQUIRED_BYTES.cend(),
                    [&](const auto& pair) {
                        return event.data[std::get<0>(pair)]
                            != std::get<1>(pair);
                    })) {
        return false;
    }
    return std::all_of(
        UPPER_BOUNDS.cbegin(), UPPER_BOUNDS.cend(), [&](const auto& pair) {
            return event.data[std::get<0>(pair)] <= std::get<1>(pair);
        });
}

static SyncTrack read_first_midi_track(const MidiTrack& track)
{
    constexpr int SET_TEMPO_ID = 0x51;
    constexpr int TIME_SIG_ID = 0x58;

    std::vector<BPM> tempos;
    std::vector<TimeSignature> time_sigs;
    for (const auto& event : track.events) {
        const auto* meta_event = std::get_if<MetaEvent>(&event.event);
        if (meta_event == nullptr) {
            continue;
        }
        switch (meta_event->type) {
        case SET_TEMPO_ID: {
            if (meta_event->data.size() < 3) {
                throw ParseError("Tempo meta event too short");
            }
            const auto us_per_quarter = meta_event->data[0] << 16
                | meta_event->data[1] << 8 | meta_event->data[2];
            const auto bpm = 60000000000 / us_per_quarter;
            tempos.push_back({event.time, static_cast<int>(bpm)});
            break;
        }
        case TIME_SIG_ID:
            if (meta_event->data.size() < 2) {
                throw ParseError("Tempo meta event too short");
            }
            if (meta_event->data[1] >= (CHAR_BIT * sizeof(int))) {
                throw ParseError("Time sig denominator too large");
            }
            time_sigs.push_back(
                {event.time, meta_event->data[0], 1 << meta_event->data[1]});
            break;
        }
    }

    return {std::move(time_sigs), std::move(tempos)};
}

template <typename T> struct InstrumentMidiTrack {
    std::map<std::tuple<Difficulty, T>, std::vector<std::tuple<int, int>>>
        note_on_events;
    std::map<std::tuple<Difficulty, T>, std::vector<std::tuple<int, int>>>
        note_off_events;
    std::map<Difficulty, std::vector<std::tuple<int, int>>> open_on_events;
    std::map<Difficulty, std::vector<std::tuple<int, int>>> open_off_events;
    std::vector<std::tuple<int, int>> yellow_tom_on_events;
    std::vector<std::tuple<int, int>> yellow_tom_off_events;
    std::vector<std::tuple<int, int>> blue_tom_on_events;
    std::vector<std::tuple<int, int>> blue_tom_off_events;
    std::vector<std::tuple<int, int>> green_tom_on_events;
    std::vector<std::tuple<int, int>> green_tom_off_events;
    std::vector<std::tuple<int, int>> solo_on_events;
    std::vector<std::tuple<int, int>> solo_off_events;
    std::vector<std::tuple<int, int>> sp_on_events;
    std::vector<std::tuple<int, int>> sp_off_events;
    std::vector<std::tuple<int, int>> fill_on_events;
    std::vector<std::tuple<int, int>> fill_off_events;
    std::map<Difficulty, std::vector<std::tuple<int, int>>>
        disco_flip_on_events;
    std::map<Difficulty, std::vector<std::tuple<int, int>>>
        disco_flip_off_events;
};

template <typename T>
static void add_sysex_event(InstrumentMidiTrack<T>& track,
                            const SysexEvent& event, int time, int rank)
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
        track.open_off_events[diff].push_back({time, rank});
    } else {
        track.open_on_events[diff].push_back({time, rank});
    }
}

template <typename T>
static void add_note_off_event(InstrumentMidiTrack<T>& track,
                               const std::array<std::uint8_t, 2>& data,
                               int time, int rank, bool from_five_lane)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;
    constexpr int DRUM_FILL_ID = 120;

    const auto diff = difficulty_from_key<T>(data[0]);
    if (diff.has_value()) {
        const auto colour = colour_from_key<T>(data[0], from_five_lane);
        track.note_off_events[{*diff, colour}].push_back({time, rank});
    } else if (data[0] == YELLOW_TOM_ID) {
        track.yellow_tom_off_events.push_back({time, rank});
    } else if (data[0] == BLUE_TOM_ID) {
        track.blue_tom_off_events.push_back({time, rank});
    } else if (data[0] == GREEN_TOM_ID) {
        track.green_tom_off_events.push_back({time, rank});
    } else if (data[0] == SOLO_NOTE_ID) {
        track.solo_off_events.push_back({time, rank});
    } else if (data[0] == SP_NOTE_ID) {
        track.sp_off_events.push_back({time, rank});
    } else if (data[0] == DRUM_FILL_ID) {
        track.fill_off_events.push_back({time, rank});
    }
}

template <typename T>
static void add_note_on_event(InstrumentMidiTrack<T>& track,
                              const std::array<std::uint8_t, 2>& data, int time,
                              int rank, bool from_five_lane)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;
    constexpr int DRUM_FILL_ID = 120;

    // Velocity 0 Note On events are counted as Note Off events.
    if (data[1] == 0) {
        add_note_off_event(track, data, time, rank, from_five_lane);
        return;
    }

    const auto diff = difficulty_from_key<T>(data[0]);
    if (diff.has_value()) {
        const auto colour = colour_from_key<T>(data[0], from_five_lane);
        track.note_on_events[{*diff, colour}].push_back({time, rank});
    } else if (data[0] == YELLOW_TOM_ID) {
        track.yellow_tom_on_events.push_back({time, rank});
    } else if (data[0] == BLUE_TOM_ID) {
        track.blue_tom_on_events.push_back({time, rank});
    } else if (data[0] == GREEN_TOM_ID) {
        track.green_tom_on_events.push_back({time, rank});
    } else if (data[0] == SOLO_NOTE_ID) {
        track.solo_on_events.push_back({time, rank});
    } else if (data[0] == SP_NOTE_ID) {
        track.sp_on_events.push_back({time, rank});
    } else if (data[0] == DRUM_FILL_ID) {
        track.fill_on_events.push_back({time, rank});
    }
}

static void append_disco_flip(InstrumentMidiTrack<DrumNoteColour>& event_track,
                              const MetaEvent& meta_event, int time, int rank)
{
    constexpr int FLIP_START_SIZE = 15;
    constexpr int FLIP_END_SIZE = 14;
    constexpr int TEXT_EVENT_ID = 1;
    constexpr std::array<std::uint8_t, 5> MIX {{'[', 'm', 'i', 'x', ' '}};
    constexpr std::array<std::uint8_t, 6> DRUMS {
        {' ', 'd', 'r', 'u', 'm', 's'}};

    if (meta_event.type != TEXT_EVENT_ID) {
        return;
    }
    if (meta_event.data.size() != FLIP_START_SIZE
        && meta_event.data.size() != FLIP_END_SIZE) {
        return;
    }
    if (!std::equal(MIX.cbegin(), MIX.cend(), meta_event.data.cbegin())) {
        return;
    }
    if (!std::equal(DRUMS.cbegin(), DRUMS.cend(),
                    meta_event.data.cbegin() + MIX.size() + 1)) {
        return;
    }
    const auto diff
        = static_cast<Difficulty>(meta_event.data[MIX.size()] - '0');
    if (meta_event.data.size() == FLIP_END_SIZE
        && meta_event.data[FLIP_END_SIZE - 1] == ']') {
        event_track.disco_flip_off_events[diff].push_back({time, rank});
    } else if (meta_event.data.size() == FLIP_START_SIZE
               && meta_event.data[FLIP_START_SIZE - 2] == 'd'
               && meta_event.data[FLIP_START_SIZE - 1] == ']') {
        event_track.disco_flip_on_events[diff].push_back({time, rank});
    }
}

static bool is_five_lane_green_note(const TimedEvent& event)
{
    constexpr std::array<std::uint8_t, 4> GREEN_LANE_KEYS {65, 77, 89, 101};
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    const auto* midi_event = std::get_if<MidiEvent>(&event.event);
    if (midi_event == nullptr) {
        return false;
    }
    const auto event_type = midi_event->status & UPPER_NIBBLE_MASK;
    if (event_type != NOTE_ON_ID && event_type != NOTE_OFF_ID) {
        return false;
    }
    const auto key = midi_event->data[0];
    return std::find(GREEN_LANE_KEYS.cbegin(), GREEN_LANE_KEYS.cend(), key)
        != GREEN_LANE_KEYS.cend();
}

template <typename T>
static InstrumentMidiTrack<T>
read_instrument_midi_track(const MidiTrack& midi_track)
{
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    bool from_five_lane = false;
    if constexpr (std::is_same_v<T, DrumNoteColour>) {
        from_five_lane
            = std::find_if(midi_track.events.cbegin(), midi_track.events.cend(),
                           is_five_lane_green_note)
            != midi_track.events.cend();
    }

    InstrumentMidiTrack<T> event_track;
    event_track.disco_flip_on_events[Difficulty::Easy] = {};
    event_track.disco_flip_off_events[Difficulty::Easy] = {};
    event_track.disco_flip_on_events[Difficulty::Medium] = {};
    event_track.disco_flip_off_events[Difficulty::Medium] = {};
    event_track.disco_flip_on_events[Difficulty::Hard] = {};
    event_track.disco_flip_off_events[Difficulty::Hard] = {};
    event_track.disco_flip_on_events[Difficulty::Expert] = {};
    event_track.disco_flip_off_events[Difficulty::Expert] = {};

    int rank = 0;
    for (const auto& event : midi_track.events) {
        ++rank;
        const auto* midi_event = std::get_if<MidiEvent>(&event.event);
        if (midi_event == nullptr) {
            const auto* sysex_event = std::get_if<SysexEvent>(&event.event);
            if (sysex_event != nullptr) {
                add_sysex_event(event_track, *sysex_event, event.time, rank);
                continue;
            }
            if constexpr (std::is_same_v<T, DrumNoteColour>) {
                const auto* meta_event = std::get_if<MetaEvent>(&event.event);
                if (meta_event != nullptr) {
                    append_disco_flip(event_track, *meta_event, event.time,
                                      rank);
                }
            }

            continue;
        }
        switch (midi_event->status & UPPER_NIBBLE_MASK) {
        case NOTE_OFF_ID:
            add_note_off_event(event_track, midi_event->data, event.time, rank,
                               from_five_lane);
            break;
        case NOTE_ON_ID:
            add_note_on_event(event_track, midi_event->data, event.time, rank,
                              from_five_lane);
            break;
        }
    }

    event_track.disco_flip_off_events.at(Difficulty::Easy)
        .push_back({std::numeric_limits<int>::max(), ++rank});
    event_track.disco_flip_off_events.at(Difficulty::Medium)
        .push_back({std::numeric_limits<int>::max(), ++rank});
    event_track.disco_flip_off_events.at(Difficulty::Hard)
        .push_back({std::numeric_limits<int>::max(), ++rank});
    event_track.disco_flip_off_events.at(Difficulty::Expert)
        .push_back({std::numeric_limits<int>::max(), ++rank});

    if (event_track.sp_on_events.empty()
        && event_track.solo_on_events.size() > 1) {
        std::swap(event_track.sp_off_events, event_track.solo_off_events);
        std::swap(event_track.sp_on_events, event_track.solo_on_events);
    }

    return event_track;
}

static std::optional<BigRockEnding> read_bre(const MidiTrack& midi_track)
{
    constexpr int BRE_KEY = 120;
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    bool has_bre = false;
    int bre_start = 0;
    int bre_end = 0;

    for (const auto& event : midi_track.events) {
        const auto* midi_event = std::get_if<MidiEvent>(&event.event);
        if (midi_event == nullptr) {
            continue;
        }
        if (midi_event->data[0] != BRE_KEY) {
            continue;
        }
        const auto event_type = midi_event->status & UPPER_NIBBLE_MASK;
        if (event_type == NOTE_OFF_ID
            || (event_type == NOTE_ON_ID && midi_event->data[1] == 0)) {
            bre_end = event.time;
            has_bre = true;
            break;
        }
        if (event_type == NOTE_ON_ID) {
            bre_start = event.time;
        }
    }

    if (!has_bre) {
        return std::nullopt;
    }
    return {{bre_start, bre_end}};
}

static std::map<Difficulty, NoteTrack<NoteColour>>
note_tracks_from_midi(const MidiTrack& midi_track, int resolution)
{
    const auto event_track = read_instrument_midi_track<NoteColour>(midi_track);
    const auto bre = read_bre(midi_track);

    std::map<Difficulty, std::vector<std::tuple<int, int>>> open_events;
    for (const auto& [diff, open_ons] : event_track.open_on_events) {
        if (event_track.open_off_events.count(diff) == 0) {
            throw ParseError("No open Note Off events");
        }
        const auto& open_offs = event_track.open_off_events.at(diff);
        open_events[diff] = combine_note_on_off_events(open_ons, open_offs);
    }

    std::map<Difficulty, std::vector<Note<NoteColour>>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour] = key;
        if (event_track.note_off_events.count(key) == 0) {
            throw ParseError("No corresponding Note Off events");
        }
        const auto& note_offs = event_track.note_off_events.at(key);
        for (const auto& [pos, end] :
             combine_note_on_off_events(note_ons, note_offs)) {
            const auto note_length = end - pos;
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
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({start, end - start});
    }

    std::map<Difficulty, NoteTrack<NoteColour>> note_tracks;
    for (const auto& [diff, note_set] : notes) {
        std::vector<int> solo_ons;
        std::vector<int> solo_offs;
        solo_ons.reserve(event_track.solo_on_events.size());
        for (const auto& [pos, rank] : event_track.solo_on_events) {
            solo_ons.push_back(pos);
        }
        solo_offs.reserve(event_track.solo_off_events.size());
        for (const auto& [pos, rank] : event_track.solo_off_events) {
            solo_offs.push_back(pos);
        }
        auto solos = form_solo_vector(solo_ons, solo_offs, note_set, true);
        note_tracks.emplace(diff,
                            NoteTrack<NoteColour> {note_set,
                                                   sp_phrases,
                                                   std::move(solos),
                                                   {},
                                                   {},
                                                   bre,
                                                   resolution});
    }

    return note_tracks;
}

static std::map<Difficulty, NoteTrack<GHLNoteColour>>
ghl_note_tracks_from_midi(const MidiTrack& midi_track, int resolution)
{
    const auto event_track
        = read_instrument_midi_track<GHLNoteColour>(midi_track);

    std::map<Difficulty, std::vector<Note<GHLNoteColour>>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour] = key;
        if (event_track.note_off_events.count(key) == 0) {
            throw ParseError("No corresponding Note Off events");
        }
        const auto& note_offs = event_track.note_off_events.at(key);
        for (const auto& [pos, end] :
             combine_note_on_off_events(note_ons, note_offs)) {
            const auto note_length = end - pos;
            notes[diff].push_back({pos, note_length, colour});
        }
    }

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({start, end - start});
    }

    std::map<Difficulty, NoteTrack<GHLNoteColour>> note_tracks;
    for (const auto& [diff, note_set] : notes) {
        std::vector<int> solo_ons;
        std::vector<int> solo_offs;
        solo_ons.reserve(event_track.solo_on_events.size());
        for (const auto& [pos, rank] : event_track.solo_on_events) {
            solo_ons.push_back(pos);
        }
        solo_offs.reserve(event_track.solo_off_events.size());
        for (const auto& [pos, rank] : event_track.solo_off_events) {
            solo_offs.push_back(pos);
        }
        auto solos = form_solo_vector(solo_ons, solo_offs, note_set, true);
        note_tracks.emplace(diff,
                            NoteTrack<GHLNoteColour> {note_set,
                                                      sp_phrases,
                                                      std::move(solos),
                                                      {},
                                                      {},
                                                      {},
                                                      resolution});
    }

    return note_tracks;
}

class TomEvents {
private:
    std::vector<std::tuple<int, int>> m_yellow_tom_events;
    std::vector<std::tuple<int, int>> m_blue_tom_events;
    std::vector<std::tuple<int, int>> m_green_tom_events;

public:
    TomEvents(const InstrumentMidiTrack<DrumNoteColour>& events)
    {
        m_yellow_tom_events = combine_note_on_off_events(
            events.yellow_tom_on_events, events.yellow_tom_off_events);
        m_blue_tom_events = combine_note_on_off_events(
            events.blue_tom_on_events, events.blue_tom_off_events);
        m_green_tom_events = combine_note_on_off_events(
            events.green_tom_on_events, events.green_tom_off_events);
    }

    [[nodiscard]] DrumNoteColour apply_tom_events(DrumNoteColour colour,
                                                  int pos) const
    {
        if (colour == DrumNoteColour::YellowCymbal) {
            for (const auto& [open_start, open_end] : m_yellow_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return DrumNoteColour::Yellow;
                }
            }
        } else if (colour == DrumNoteColour::BlueCymbal) {
            for (const auto& [open_start, open_end] : m_blue_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return DrumNoteColour::Blue;
                }
            }
        } else if (colour == DrumNoteColour::GreenCymbal) {
            for (const auto& [open_start, open_end] : m_green_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return DrumNoteColour::Green;
                }
            }
        }
        return colour;
    }
};

// This is to deal with G cymbal + G tom from five lane being turned into G
// cymbal + B tom. This combination cannot happen from a four lane chart.
static void fix_double_greens(std::vector<Note<DrumNoteColour>>& notes)
{
    std::set<int> green_cymbal_positions;

    for (const auto& note : notes) {
        if (note.colour == DrumNoteColour::GreenCymbal) {
            green_cymbal_positions.insert(note.position);
        }
    }

    for (auto& note : notes) {
        if (note.colour != DrumNoteColour::Green) {
            continue;
        }
        if (green_cymbal_positions.count(note.position) != 0) {
            note.colour = DrumNoteColour::Blue;
        }
    }
}

static std::map<Difficulty, NoteTrack<DrumNoteColour>>
drum_note_tracks_from_midi(const MidiTrack& midi_track, int resolution)
{
    const auto event_track
        = read_instrument_midi_track<DrumNoteColour>(midi_track);

    const TomEvents tom_events {event_track};

    std::map<Difficulty, std::vector<Note<DrumNoteColour>>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour] = key;
        if (event_track.note_off_events.count(key) == 0) {
            throw ParseError("No corresponding Note Off events");
        }
        const auto& note_offs = event_track.note_off_events.at(key);
        for (const auto& [pos, end] :
             combine_note_on_off_events(note_ons, note_offs)) {
            notes[diff].push_back(
                {pos, 0, tom_events.apply_tom_events(colour, pos)});
        }
        fix_double_greens(notes[diff]);
    }

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({start, end - start});
    }

    std::vector<DrumFill> drum_fills;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.fill_on_events, event_track.fill_off_events)) {
        drum_fills.push_back({start, end - start});
    }

    std::map<Difficulty, NoteTrack<DrumNoteColour>> note_tracks;
    for (const auto& [diff, note_set] : notes) {
        std::vector<int> solo_ons;
        std::vector<int> solo_offs;
        solo_ons.reserve(event_track.solo_on_events.size());
        for (const auto& [pos, rank] : event_track.solo_on_events) {
            solo_ons.push_back(pos);
        }
        solo_offs.reserve(event_track.solo_off_events.size());
        for (const auto& [pos, rank] : event_track.solo_off_events) {
            solo_offs.push_back(pos);
        }
        std::vector<DiscoFlip> disco_flips;
        for (const auto& [start, end] : combine_note_on_off_events(
                 event_track.disco_flip_on_events.at(diff),
                 event_track.disco_flip_off_events.at(diff))) {
            disco_flips.push_back({start, end - start});
        }
        auto solos = form_solo_vector(solo_ons, solo_offs, note_set, true);
        note_tracks.emplace(diff,
                            NoteTrack<DrumNoteColour> {note_set,
                                                       sp_phrases,
                                                       std::move(solos),
                                                       drum_fills,
                                                       std::move(disco_flips),
                                                       {},
                                                       resolution});
    }

    return note_tracks;
}

static std::optional<std::string> midi_track_name(const MidiTrack& track)
{
    if (track.events.empty()) {
        return std::nullopt;
    }
    for (const auto& event : track.events) {
        const auto* meta_event = std::get_if<MetaEvent>(&event.event);
        if (meta_event == nullptr) {
            continue;
        }
        if (meta_event->type != 3) {
            continue;
        }
        return std::string {meta_event->data.cbegin(), meta_event->data.cend()};
    }
    return std::nullopt;
}

static std::optional<Instrument>
midi_section_instrument(const std::string& track_name)
{
    const std::map<std::string, Instrument> INSTRUMENTS {
        {"PART GUITAR", Instrument::Guitar},
        {"T1 GEMS", Instrument::Guitar},
        {"PART GUITAR COOP", Instrument::GuitarCoop},
        {"PART BASS", Instrument::Bass},
        {"PART RHYTHM", Instrument::Rhythm},
        {"PART KEYS", Instrument::Keys},
        {"PART GUITAR GHL", Instrument::GHLGuitar},
        {"PART BASS GHL", Instrument::GHLBass},
        {"PART DRUMS", Instrument::Drums}};

    const auto iter = INSTRUMENTS.find(track_name);
    if (iter == INSTRUMENTS.end()) {
        return std::nullopt;
    }
    return iter->second;
}

static std::vector<int> od_beats_from_track(const MidiTrack& track)
{
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;
    constexpr int BEAT_LOW_KEY = 12;
    constexpr int BEAT_HIGH_KEY = 13;

    std::vector<int> od_beats;

    for (const auto& event : track.events) {
        const auto* midi_event = std::get_if<MidiEvent>(&event.event);
        if (midi_event == nullptr) {
            continue;
        }
        if ((midi_event->status & UPPER_NIBBLE_MASK) != NOTE_ON_ID) {
            continue;
        }
        if (midi_event->data[1] == 0) {
            continue;
        }
        const auto key = midi_event->data[0];
        if (key == BEAT_LOW_KEY || key == BEAT_HIGH_KEY) {
            od_beats.push_back(event.time);
        }
    }

    return od_beats;
}

Song Song::from_midi(const Midi& midi, const IniValues& ini)
{
    if (midi.ticks_per_quarter_note == 0) {
        throw ParseError("Resolution must be > 0");
    }

    Song song;
    song.m_is_from_midi = true;
    song.m_resolution = midi.ticks_per_quarter_note;
    song.m_name = ini.name;
    song.m_artist = ini.artist;
    song.m_charter = ini.charter;

    if (midi.tracks.empty()) {
        return song;
    }

    song.m_sync_track = read_first_midi_track(midi.tracks[0]);

    for (const auto& track : midi.tracks) {
        const auto track_name = midi_track_name(track);
        if (!track_name.has_value()) {
            continue;
        }
        if (*track_name == "BEAT") {
            song.m_od_beats = od_beats_from_track(track);
        }
        const auto inst = midi_section_instrument(*track_name);
        if (!inst.has_value()) {
            continue;
        }
        if (is_six_fret_instrument(*inst)) {
            auto tracks = ghl_note_tracks_from_midi(track, song.m_resolution);
            for (auto& [diff, note_track] : tracks) {
                song.m_six_fret_tracks.emplace(std::tuple {*inst, diff},
                                               std::move(note_track));
            }
        } else if (*inst == Instrument::Drums) {
            song.m_drum_note_tracks
                = drum_note_tracks_from_midi(track, song.m_resolution);
        } else {
            auto tracks = note_tracks_from_midi(track, song.m_resolution);
            for (auto& [diff, note_track] : tracks) {
                song.m_five_fret_tracks.emplace(std::tuple {*inst, diff},
                                                std::move(note_track));
            }
        }
    }

    return song;
}

std::vector<int> Song::unison_phrase_positions() const
{
    std::map<int, std::set<Instrument>> phrase_by_instrument;
    for (const auto& [key, value] : m_five_fret_tracks) {
        const auto instrument = std::get<0>(key);
        for (const auto& phrase : value.sp_phrases()) {
            phrase_by_instrument[phrase.position].insert(instrument);
        }
    }
    for (const auto& [key, value] : m_drum_note_tracks) {
        for (const auto& phrase : value.sp_phrases()) {
            phrase_by_instrument[phrase.position].insert(Instrument::Drums);
        }
    }
    std::vector<int> unison_starts;
    for (const auto& [key, value] : phrase_by_instrument) {
        if (value.size() > 1) {
            unison_starts.push_back(key);
        }
    }
    return unison_starts;
}
