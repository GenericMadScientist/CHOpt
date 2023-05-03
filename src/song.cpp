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

#include <algorithm>
#include <climits>
#include <filesystem>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <type_traits>

#include <boost/nowide/fstream.hpp>

#include "song.hpp"

namespace {
bool starts_with_prefix(const std::string& string, std::string_view prefix)
{
    return string.substr(0, prefix.size()) == prefix;
}

bool ends_with_suffix(const std::string& string, std::string_view suffix)
{
    if (string.size() < suffix.size()) {
        return false;
    }
    return string.substr(string.size() - suffix.size()) == suffix;
}

// Takes a sequence of points where some note type/event is turned on, and a
// sequence where said type is turned off, and returns a tuple of intervals
// where the event is on.
std::vector<std::tuple<Tick, Tick>>
combine_solo_events(const std::vector<int>& on_events,
                    const std::vector<int>& off_events)
{
    std::vector<std::tuple<Tick, Tick>> ranges;

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

std::vector<Solo> form_solo_vector(const std::vector<int>& solo_on_events,
                                   const std::vector<int>& solo_off_events,
                                   const std::vector<Note>& notes,
                                   TrackType track_type, bool is_midi)
{
    constexpr int SOLO_NOTE_VALUE = 100;

    std::vector<Solo> solos;

    for (auto [start, end] :
         combine_solo_events(solo_on_events, solo_off_events)) {
        std::set<Tick> positions_in_solo;
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
        if (track_type != TrackType::Drums) {
            note_count = static_cast<int>(positions_in_solo.size());
        }
        solos.push_back({start, end, SOLO_NOTE_VALUE * note_count});
    }

    return solos;
}

std::optional<Note>
note_from_colour_key_map(const std::map<int, int>& colour_map, int position,
                         int length, int fret_type, NoteFlags flags)
{
    const auto colour_iter = colour_map.find(fret_type);
    if (colour_iter == colour_map.end()) {
        return std::nullopt;
    }
    Note note;
    note.position = Tick {position};
    note.lengths.at(colour_iter->second) = Tick {length};
    note.flags = flags;
    return note;
}

std::optional<Note> note_from_note_colour(int position, int length,
                                          int fret_type, TrackType track_type)
{
    std::map<int, int> colours;
    switch (track_type) {
    case TrackType::FiveFret:
        colours = {{0, FIVE_FRET_GREEN},  {1, FIVE_FRET_RED},
                   {2, FIVE_FRET_YELLOW}, {3, FIVE_FRET_BLUE},
                   {4, FIVE_FRET_ORANGE}, {7, FIVE_FRET_OPEN}}; // NOLINT
        return note_from_colour_key_map(colours, position, length, fret_type,
                                        FLAGS_FIVE_FRET_GUITAR);
    case TrackType::SixFret:
        colours = {{0, SIX_FRET_WHITE_LOW},  {1, SIX_FRET_WHITE_MID},
                   {2, SIX_FRET_WHITE_HIGH}, {3, SIX_FRET_BLACK_LOW},
                   {4, SIX_FRET_BLACK_MID},  {7, SIX_FRET_OPEN}, // NOLINT
                   {8, SIX_FRET_BLACK_HIGH}}; // NOLINT
        return note_from_colour_key_map(colours, position, length, fret_type,
                                        FLAGS_SIX_FRET_GUITAR);
    case TrackType::Drums: {
        constexpr int CYMBAL_THRESHOLD = 64;
        colours = {{0, DRUM_KICK},    {1, DRUM_RED},
                   {2, DRUM_YELLOW},  {3, DRUM_BLUE},
                   {4, DRUM_GREEN},   {32, DRUM_DOUBLE_KICK}, // NOLINT
                   {66, DRUM_YELLOW}, {67, DRUM_BLUE}, // NOLINT
                   {68, DRUM_GREEN}}; // NOLINT
        auto note = note_from_colour_key_map(colours, position, length,
                                             fret_type, FLAGS_DRUMS);
        if (note.has_value() && fret_type >= CYMBAL_THRESHOLD) {
            note->flags = static_cast<NoteFlags>(note->flags | FLAGS_CYMBAL);
        }
        return note;
    }
    default:
        throw std::invalid_argument("Invalid track type");
    }
}

std::string get_with_default(const std::map<std::string, std::string>& map,
                             const std::string& key, std::string default_value)
{
    const auto iter = map.find(key);
    if (iter == map.end()) {
        return default_value;
    }
    return iter->second;
}

std::optional<std::tuple<Difficulty, Instrument>>
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

std::vector<Note>
add_fifth_lane_greens(std::vector<Note> notes,
                      const std::vector<NoteEvent>& note_events)
{
    constexpr int FIVE_LANE_GREEN = 5;

    std::set<Tick> green_positions;
    for (const auto& note : notes) {
        if (note.lengths[3] != Tick {-1}) {
            green_positions.insert(note.position);
        }
    }
    for (const auto& note_event : note_events) {
        if (note_event.fret != FIVE_LANE_GREEN) {
            continue;
        }
        Note note;
        note.position = Tick {note_event.position};
        note.flags = FLAGS_DRUMS;
        if (green_positions.count(Tick {note_event.position}) != 0) {
            note.lengths[DRUM_BLUE] = Tick {0};
        } else {
            note.lengths[DRUM_GREEN] = Tick {0};
        }
        notes.push_back(note);
    }
    return notes;
}

std::vector<Note> apply_cymbal_events(const std::vector<Note>& notes)
{
    std::set<unsigned int> deletion_spots;
    for (auto i = 0U; i < notes.size(); ++i) {
        const auto& cymbal_note = notes[i];
        if ((cymbal_note.flags & FLAGS_CYMBAL) == 0U) {
            continue;
        }
        bool delete_cymbal = true;
        for (auto j = 0U; j < notes.size(); ++j) {
            if (i == j) {
                continue;
            }
            const auto& non_cymbal_note = notes[j];
            if (non_cymbal_note.position != cymbal_note.position) {
                continue;
            }
            if (cymbal_note.colours() != non_cymbal_note.colours()) {
                continue;
            }
            delete_cymbal = false;
            deletion_spots.insert(j);
        }
        if (delete_cymbal) {
            deletion_spots.insert(i);
        }
    }

    std::vector<Note> new_notes;
    for (auto i = 0U; i < notes.size(); ++i) {
        if (deletion_spots.count(i) == 0) {
            new_notes.push_back(notes[i]);
        }
    }
    return new_notes;
}

int no_dynamics_lane_colour(const Note& note)
{
    if (note.lengths[DRUM_RED] != Tick {-1}) {
        return 0;
    }
    if (note.lengths[DRUM_YELLOW] != Tick {-1}) {
        return 1;
    }
    if (note.lengths[DRUM_BLUE] != Tick {-1}) {
        return 2;
    }
    if (note.lengths[DRUM_GREEN] != Tick {-1}) {
        return 3;
    }
    return -1;
}

std::vector<Note>
apply_dynamics_events(std::vector<Note> notes,
                      const std::vector<NoteEvent>& note_events)
{
    constexpr int GHOST_BASE = 34;
    constexpr int ACCENT_BASE = 40;
    constexpr int LANE_COUNT = 4;

    std::set<std::tuple<Tick, int>> accent_events;
    std::set<std::tuple<Tick, int>> ghost_events;

    for (const auto& event : note_events) {
        if (event.fret > ACCENT_BASE + LANE_COUNT || event.fret < GHOST_BASE) {
            continue;
        }
        if (event.fret < GHOST_BASE + LANE_COUNT) {
            accent_events.emplace(Tick {event.position},
                                  event.fret - GHOST_BASE);
        }
        if (event.fret >= ACCENT_BASE) {
            ghost_events.emplace(Tick {event.position},
                                 event.fret - ACCENT_BASE);
        }
    }
    for (auto& note : notes) {
        if (note.is_kick_note()) {
            continue;
        }
        const auto lane = no_dynamics_lane_colour(note);
        if (accent_events.count({note.position, lane}) > 0) {
            note.flags = static_cast<NoteFlags>(note.flags | FLAGS_ACCENT);
        } else if (ghost_events.count({note.position, lane}) > 0) {
            note.flags = static_cast<NoteFlags>(note.flags | FLAGS_GHOST);
        }
    }
    return notes;
}

NoteTrack note_track_from_section(const ChartSection& section,
                                  std::shared_ptr<SongGlobalData> global_data,
                                  TrackType track_type)
{
    constexpr int DISCO_FLIP_START_SIZE = 13;
    constexpr int DISCO_FLIP_END_SIZE = 12;
    constexpr int DRUM_FILL_KEY = 64;
    constexpr std::array<std::uint8_t, 4> MIX {{'m', 'i', 'x', '_'}};
    constexpr std::array<std::uint8_t, 6> DRUMS {
        {'_', 'd', 'r', 'u', 'm', 's'}};

    std::vector<Note> notes;
    for (const auto& note_event : section.note_events) {
        const auto note
            = note_from_note_colour(note_event.position, note_event.length,
                                    note_event.fret, track_type);
        if (note.has_value()) {
            notes.push_back(*note);
        }
    }
    if (track_type == TrackType::Drums) {
        notes = add_fifth_lane_greens(std::move(notes), section.note_events);
        notes = apply_cymbal_events(notes);
        notes = apply_dynamics_events(notes, section.note_events);
    }

    std::vector<DrumFill> fills;
    std::vector<StarPower> sp;
    for (const auto& phrase : section.special_events) {
        if (phrase.key == 2) {
            sp.push_back(
                StarPower {Tick {phrase.position}, Tick {phrase.length}});
        } else if (phrase.key == DRUM_FILL_KEY) {
            fills.push_back(
                DrumFill {Tick {phrase.position}, Tick {phrase.length}});
        }
    }
    if (track_type != TrackType::Drums) {
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
    auto solos = form_solo_vector(solo_on_events, solo_off_events, notes,
                                  track_type, false);
    std::sort(disco_flip_on_events.begin(), disco_flip_on_events.end());
    std::sort(disco_flip_off_events.begin(), disco_flip_off_events.end());
    std::vector<DiscoFlip> disco_flips;
    for (auto [start, end] :
         combine_solo_events(disco_flip_on_events, disco_flip_off_events)) {
        disco_flips.push_back({start, end - start});
    }

    return {
        std::move(notes),       sp, std::move(solos), std::move(fills),
        std::move(disco_flips), {}, track_type,       std::move(global_data)};
}

bool is_six_fret_instrument(Instrument instrument)
{
    return instrument == Instrument::GHLGuitar
        || instrument == Instrument::GHLBass;
}

TrackType track_type_from_instrument(Instrument instrument)
{
    switch (instrument) {
    case Instrument::Guitar:
    case Instrument::GuitarCoop:
    case Instrument::Bass:
    case Instrument::Rhythm:
    case Instrument::Keys:
        return TrackType::FiveFret;
    case Instrument::GHLGuitar:
    case Instrument::GHLBass:
        return TrackType::SixFret;
    case Instrument::Drums:
        return TrackType::Drums;
    default:
        throw std::invalid_argument("Invalid instrument");
    }
}

TempoMap tempo_map_from_section(const ChartSection& section, int resolution)
{
    std::vector<BPM> bpms;
    bpms.reserve(section.bpm_events.size());
    for (const auto& bpm : section.bpm_events) {
        bpms.push_back({Tick {bpm.position}, bpm.bpm});
    }
    std::vector<TimeSignature> tses;
    for (const auto& ts : section.ts_events) {
        if (static_cast<std::size_t>(ts.denominator)
            >= (CHAR_BIT * sizeof(int))) {
            throw ParseError("Invalid Time Signature denominator");
        }
        tses.push_back({Tick {ts.position}, ts.numerator, 1 << ts.denominator});
    }
    return {std::move(tses), std::move(bpms), resolution};
}

// Like combine_solo_events, but never skips on events to suit Midi parsing and
// checks if there is an unmatched on event.
// The tuples are a pair of the form (position, rank), where events later in the
// file have a higher rank. This is in case of the Note Off event being right
// after the corresponding Note On event in the file, but at the same tick.
std::vector<std::tuple<int, int>>
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

std::optional<Difficulty> look_up_difficulty(
    const std::array<std::tuple<int, int, Difficulty>, 4>& diff_ranges,
    std::uint8_t key)
{
    for (const auto& [min, max, diff] : diff_ranges) {
        if (key >= min && key <= max) {
            return {diff};
        }
    }

    return std::nullopt;
}

std::optional<Difficulty> difficulty_from_key(std::uint8_t key,
                                              TrackType track_type)
{
    std::array<std::tuple<int, int, Difficulty>, 4> diff_ranges;
    switch (track_type) {
    case TrackType::FiveFret:
        diff_ranges = {{{96, 100, Difficulty::Expert}, // NOLINT
                        {84, 88, Difficulty::Hard}, // NOLINT
                        {72, 76, Difficulty::Medium}, // NOLINT
                        {60, 64, Difficulty::Easy}}}; // NOLINT
        break;
    case TrackType::SixFret:
        diff_ranges = {{{94, 100, Difficulty::Expert}, // NOLINT
                        {82, 88, Difficulty::Hard}, // NOLINT
                        {70, 76, Difficulty::Medium}, // NOLINT
                        {58, 64, Difficulty::Easy}}}; // NOLINT
        break;
    case TrackType::Drums:
        diff_ranges = {{{95, 101, Difficulty::Expert}, // NOLINT
                        {83, 89, Difficulty::Hard}, // NOLINT
                        {71, 77, Difficulty::Medium}, // NOLINT
                        {59, 65, Difficulty::Easy}}}; // NOLINT
        break;
    }
    return look_up_difficulty(diff_ranges, key);
}

template <typename T, std::size_t N>
T colour_from_key_and_bounds(std::uint8_t key,
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

int colour_from_key(std::uint8_t key, TrackType track_type, bool from_five_lane)
{
    std::array<unsigned int, 4> diff_ranges {};
    switch (track_type) {
    case TrackType::FiveFret: {
        diff_ranges = {96, 84, 72, 60}; // NOLINT
        constexpr std::array NOTE_COLOURS {FIVE_FRET_GREEN, FIVE_FRET_RED,
                                           FIVE_FRET_YELLOW, FIVE_FRET_BLUE,
                                           FIVE_FRET_ORANGE};
        return colour_from_key_and_bounds(key, diff_ranges, NOTE_COLOURS);
    }
    case TrackType::SixFret: {
        diff_ranges = {94, 82, 70, 58}; // NOLINT
        constexpr std::array GHL_NOTE_COLOURS {
            SIX_FRET_OPEN,       SIX_FRET_WHITE_LOW, SIX_FRET_WHITE_MID,
            SIX_FRET_WHITE_HIGH, SIX_FRET_BLACK_LOW, SIX_FRET_BLACK_MID,
            SIX_FRET_BLACK_HIGH};
        return colour_from_key_and_bounds(key, diff_ranges, GHL_NOTE_COLOURS);
    }
    case TrackType::Drums: {
        diff_ranges = {95, 83, 71, 59}; // NOLINT
        constexpr std::array DRUM_NOTE_COLOURS {DRUM_DOUBLE_KICK, DRUM_KICK,
                                                DRUM_RED,         DRUM_YELLOW,
                                                DRUM_BLUE,        DRUM_GREEN};
        constexpr std::array FIVE_LANE_COLOURS {
            DRUM_DOUBLE_KICK, DRUM_KICK,  DRUM_RED,  DRUM_YELLOW,
            DRUM_BLUE,        DRUM_GREEN, DRUM_GREEN};
        if (from_five_lane) {
            return colour_from_key_and_bounds(key, diff_ranges,
                                              FIVE_LANE_COLOURS);
        }
        return colour_from_key_and_bounds(key, diff_ranges, DRUM_NOTE_COLOURS);
    }
    default:
        throw std::invalid_argument("Invalid track type");
    }
}

bool is_cymbal_key(std::uint8_t key, bool from_five_lane)
{
    const auto index = (key + 1) % 12;
    if (from_five_lane) {
        return index == 3 || index == 5; // NOLINT
    }
    return index == 3 || index == 4 || index == 5; // NOLINT
}

bool is_open_event_sysex(const SysexEvent& event)
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

TempoMap read_first_midi_track(const MidiTrack& track, int resolution)
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
            tempos.push_back({Tick {event.time}, static_cast<int>(bpm)});
            break;
        }
        case TIME_SIG_ID:
            if (meta_event->data.size() < 2) {
                throw ParseError("Tempo meta event too short");
            }
            if (meta_event->data[1] >= (CHAR_BIT * sizeof(int))) {
                throw ParseError("Time sig denominator too large");
            }
            time_sigs.push_back({Tick {event.time}, meta_event->data[0],
                                 1 << meta_event->data[1]});
            break;
        }
    }

    return {std::move(time_sigs), std::move(tempos), resolution};
}

struct InstrumentMidiTrack {
public:
    std::map<std::tuple<Difficulty, int, NoteFlags>,
             std::vector<std::tuple<int, int>>>
        note_on_events;
    std::map<std::tuple<Difficulty, int>, std::vector<std::tuple<int, int>>>
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

    InstrumentMidiTrack() = default;
};

void add_sysex_event(InstrumentMidiTrack& track, const SysexEvent& event,
                     int time, int rank)
{
    constexpr std::array<Difficulty, 4> OPEN_EVENT_DIFFS {
        Difficulty::Easy, Difficulty::Medium, Difficulty::Hard,
        Difficulty::Expert};
    constexpr int SYSEX_ON_INDEX = 6;

    if (!is_open_event_sysex(event)) {
        return;
    }
    const Difficulty diff = OPEN_EVENT_DIFFS.at(event.data[4]);
    if (event.data[SYSEX_ON_INDEX] == 0) {
        track.open_off_events[diff].emplace_back(time, rank);
    } else {
        track.open_on_events[diff].emplace_back(time, rank);
    }
}

void add_note_off_event(InstrumentMidiTrack& track,
                        const std::array<std::uint8_t, 2>& data, int time,
                        int rank, bool from_five_lane, TrackType track_type)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;
    constexpr int DRUM_FILL_ID = 120;

    const auto diff = difficulty_from_key(data[0], track_type);
    if (diff.has_value()) {
        const auto colour
            = colour_from_key(data[0], track_type, from_five_lane);
        track.note_off_events[{*diff, colour}].emplace_back(time, rank);
    } else if (data[0] == YELLOW_TOM_ID) {
        track.yellow_tom_off_events.emplace_back(time, rank);
    } else if (data[0] == BLUE_TOM_ID) {
        track.blue_tom_off_events.emplace_back(time, rank);
    } else if (data[0] == GREEN_TOM_ID) {
        track.green_tom_off_events.emplace_back(time, rank);
    } else if (data[0] == SOLO_NOTE_ID) {
        track.solo_off_events.emplace_back(time, rank);
    } else if (data[0] == SP_NOTE_ID) {
        track.sp_off_events.emplace_back(time, rank);
    } else if (data[0] == DRUM_FILL_ID) {
        track.fill_off_events.emplace_back(time, rank);
    }
}

NoteFlags flags_from_track_type(TrackType track_type)
{
    switch (track_type) {
    case TrackType::FiveFret:
        return FLAGS_FIVE_FRET_GUITAR;
    case TrackType::SixFret:
        return FLAGS_SIX_FRET_GUITAR;
    case TrackType::Drums:
        return FLAGS_DRUMS;
    default:
        throw std::invalid_argument("Invalid track type");
    }
}

NoteFlags dynamics_flags_from_velocity(std::uint8_t velocity)
{
    constexpr std::uint8_t MIN_ACCENT_VELOCITY = 127;

    if (velocity == 1) {
        return FLAGS_GHOST;
    }
    if (velocity >= MIN_ACCENT_VELOCITY) {
        return FLAGS_ACCENT;
    }
    return static_cast<NoteFlags>(0);
}

void add_note_on_event(InstrumentMidiTrack& track,
                       const std::array<std::uint8_t, 2>& data, int time,
                       int rank, bool from_five_lane, bool parse_dynamics,
                       TrackType track_type)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;
    constexpr int DRUM_FILL_ID = 120;

    // Velocity 0 Note On events are counted as Note Off events.
    if (data[1] == 0) {
        add_note_off_event(track, data, time, rank, from_five_lane, track_type);
        return;
    }

    const auto diff = difficulty_from_key(data[0], track_type);
    if (diff.has_value()) {
        auto colour = colour_from_key(data[0], track_type, from_five_lane);
        auto flags = flags_from_track_type(track_type);
        if (track_type == TrackType::Drums) {
            if (is_cymbal_key(data[0], from_five_lane)) {
                flags = static_cast<NoteFlags>(flags | FLAGS_CYMBAL);
            }
            if (parse_dynamics) {
                flags = static_cast<NoteFlags>(
                    flags | dynamics_flags_from_velocity(data[1]));
            }
        }
        track.note_on_events[{*diff, colour, flags}].emplace_back(time, rank);
    } else if (data[0] == YELLOW_TOM_ID) {
        track.yellow_tom_on_events.emplace_back(time, rank);
    } else if (data[0] == BLUE_TOM_ID) {
        track.blue_tom_on_events.emplace_back(time, rank);
    } else if (data[0] == GREEN_TOM_ID) {
        track.green_tom_on_events.emplace_back(time, rank);
    } else if (data[0] == SOLO_NOTE_ID) {
        track.solo_on_events.emplace_back(time, rank);
    } else if (data[0] == SP_NOTE_ID) {
        track.sp_on_events.emplace_back(time, rank);
    } else if (data[0] == DRUM_FILL_ID) {
        track.fill_on_events.emplace_back(time, rank);
    }
}

void append_disco_flip(InstrumentMidiTrack& event_track,
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
        event_track.disco_flip_off_events[diff].emplace_back(time, rank);
    } else if (meta_event.data.size() == FLIP_START_SIZE
               && meta_event.data[FLIP_START_SIZE - 2] == 'd'
               && meta_event.data[FLIP_START_SIZE - 1] == ']') {
        event_track.disco_flip_on_events[diff].emplace_back(time, rank);
    }
}

bool is_five_lane_green_note(const TimedEvent& event)
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

bool is_enable_chart_dynamics(const TimedEvent& event)
{
    using namespace std::literals;
    constexpr auto ENABLE_DYNAMICS = "[ENABLE_CHART_DYNAMICS]"sv;

    const auto* meta_event = std::get_if<MetaEvent>(&event.event);
    if (meta_event == nullptr) {
        return false;
    }
    if (meta_event->type != 1) {
        return false;
    }
    return std::equal(meta_event->data.cbegin(), meta_event->data.cend(),
                      ENABLE_DYNAMICS.cbegin(), ENABLE_DYNAMICS.cend());
}

bool has_five_lane_green_notes(const MidiTrack& midi_track)
{
    return std::find_if(midi_track.events.cbegin(), midi_track.events.cend(),
                        is_five_lane_green_note)
        != midi_track.events.cend();
}

bool has_enable_chart_dynamics(const MidiTrack& midi_track)
{
    return std::find_if(midi_track.events.cbegin(), midi_track.events.cend(),
                        is_enable_chart_dynamics)
        != midi_track.events.cend();
}

InstrumentMidiTrack read_instrument_midi_track(const MidiTrack& midi_track,
                                               TrackType track_type)
{
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    const bool from_five_lane = track_type == TrackType::Drums
        && has_five_lane_green_notes(midi_track);
    const bool parse_dynamics = track_type == TrackType::Drums
        && has_enable_chart_dynamics(midi_track);

    InstrumentMidiTrack event_track;
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
            if (track_type == TrackType::Drums) {
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
                               from_five_lane, track_type);
            break;
        case NOTE_ON_ID:
            add_note_on_event(event_track, midi_event->data, event.time, rank,
                              from_five_lane, parse_dynamics, track_type);
            break;
        }
    }

    event_track.disco_flip_off_events.at(Difficulty::Easy)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);
    event_track.disco_flip_off_events.at(Difficulty::Medium)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);
    event_track.disco_flip_off_events.at(Difficulty::Hard)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);
    event_track.disco_flip_off_events.at(Difficulty::Expert)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);

    if (event_track.sp_on_events.empty()
        && event_track.solo_on_events.size() > 1) {
        std::swap(event_track.sp_off_events, event_track.solo_off_events);
        std::swap(event_track.sp_on_events, event_track.solo_on_events);
    }

    return event_track;
}

std::optional<BigRockEnding> read_bre(const MidiTrack& midi_track)
{
    constexpr int BRE_KEY = 120;
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    Tick bre_start {0};

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
            const Tick bre_end {event.time};
            return {{bre_start, bre_end}};
        }
        if (event_type == NOTE_ON_ID) {
            bre_start = Tick {event.time};
        }
    }

    return std::nullopt;
}

std::map<Difficulty, std::vector<Note>> notes_from_event_track(
    const InstrumentMidiTrack& event_track,
    const std::map<Difficulty, std::vector<std::tuple<int, int>>>& open_events,
    TrackType track_type)
{
    std::map<Difficulty, std::vector<Note>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour, flags] = key;
        if (event_track.note_off_events.count({diff, colour}) == 0) {
            throw ParseError("No corresponding Note Off events");
        }
        const auto& note_offs = event_track.note_off_events.at({diff, colour});
        for (const auto& [pos, end] :
             combine_note_on_off_events(note_ons, note_offs)) {
            const auto note_length = end - pos;
            auto note_colour = colour;
            if (track_type == TrackType::FiveFret) {
                const auto open_events_iter = open_events.find(diff);
                if (open_events_iter != open_events.cend()) {
                    for (const auto& [open_start, open_end] :
                         open_events_iter->second) {
                        if (pos >= open_start && pos < open_end) {
                            note_colour = FIVE_FRET_OPEN;
                        }
                    }
                }
            }
            Note note;
            note.position = Tick {pos};
            note.lengths.at(note_colour) = Tick {note_length};
            note.flags = flags_from_track_type(track_type);
            notes[diff].push_back(note);
        }
    }

    return notes;
}

std::map<Difficulty, NoteTrack>
note_tracks_from_midi(const MidiTrack& midi_track,
                      const std::shared_ptr<SongGlobalData>& global_data)
{
    const auto event_track
        = read_instrument_midi_track(midi_track, TrackType::FiveFret);
    const auto bre = read_bre(midi_track);

    std::map<Difficulty, std::vector<std::tuple<int, int>>> open_events;
    for (const auto& [diff, open_ons] : event_track.open_on_events) {
        if (event_track.open_off_events.count(diff) == 0) {
            throw ParseError("No open Note Off events");
        }
        const auto& open_offs = event_track.open_off_events.at(diff);
        open_events[diff] = combine_note_on_off_events(open_ons, open_offs);
    }

    const auto notes
        = notes_from_event_track(event_track, open_events, TrackType::FiveFret);

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({Tick {start}, Tick {end - start}});
    }

    std::map<Difficulty, NoteTrack> note_tracks;
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
        auto solos = form_solo_vector(solo_ons, solo_offs, note_set,
                                      TrackType::FiveFret, true);
        note_tracks.emplace(diff,
                            NoteTrack {note_set,
                                       sp_phrases,
                                       std::move(solos),
                                       {},
                                       {},
                                       bre,
                                       TrackType::FiveFret,
                                       global_data});
    }

    return note_tracks;
}

std::map<Difficulty, NoteTrack>
ghl_note_tracks_from_midi(const MidiTrack& midi_track,
                          const std::shared_ptr<SongGlobalData>& global_data)
{
    const auto event_track
        = read_instrument_midi_track(midi_track, TrackType::SixFret);

    const auto notes
        = notes_from_event_track(event_track, {}, TrackType::SixFret);

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({Tick {start}, Tick {end - start}});
    }

    std::map<Difficulty, NoteTrack> note_tracks;
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
        auto solos = form_solo_vector(solo_ons, solo_offs, note_set,
                                      TrackType::SixFret, true);
        note_tracks.emplace(diff,
                            NoteTrack {note_set,
                                       sp_phrases,
                                       std::move(solos),
                                       {},
                                       {},
                                       {},
                                       TrackType::SixFret,
                                       global_data});
    }

    return note_tracks;
}

// This is to deal with G cymbal + G tom from five lane being turned into G
// cymbal + B tom. This combination cannot happen from a four lane chart.
void fix_double_greens(std::vector<Note>& notes)
{
    std::set<Tick> green_cymbal_positions;

    for (const auto& note : notes) {
        if ((note.lengths.at(DRUM_GREEN) != Tick {-1})
            && ((note.flags & FLAGS_CYMBAL) != 0U)) {
            green_cymbal_positions.insert(note.position);
        }
    }

    for (auto& note : notes) {
        if ((note.lengths.at(DRUM_GREEN) == Tick {-1})
            || ((note.flags & FLAGS_CYMBAL) != 0U)) {
            continue;
        }
        if (green_cymbal_positions.count(note.position) != 0) {
            std::swap(note.lengths[DRUM_BLUE], note.lengths[DRUM_GREEN]);
        }
    }
}

class TomEvents {
private:
    std::vector<std::tuple<int, int>> m_yellow_tom_events;
    std::vector<std::tuple<int, int>> m_blue_tom_events;
    std::vector<std::tuple<int, int>> m_green_tom_events;

public:
    explicit TomEvents(const InstrumentMidiTrack& events)
        : m_yellow_tom_events {combine_note_on_off_events(
            events.yellow_tom_on_events, events.yellow_tom_off_events)}
        , m_blue_tom_events {combine_note_on_off_events(
              events.blue_tom_on_events, events.blue_tom_off_events)}
        , m_green_tom_events {combine_note_on_off_events(
              events.green_tom_on_events, events.green_tom_off_events)}
    {
    }

    [[nodiscard]] bool force_tom(int colour, int pos) const
    {
        if (colour == DRUM_YELLOW) {
            for (const auto& [open_start, open_end] : m_yellow_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return true;
                }
            }
        } else if (colour == DRUM_BLUE) {
            for (const auto& [open_start, open_end] : m_blue_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return true;
                }
            }
        } else if (colour == DRUM_GREEN) {
            for (const auto& [open_start, open_end] : m_green_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return true;
                }
            }
        }
        return false;
    }
};

std::map<Difficulty, NoteTrack>
drum_note_tracks_from_midi(const MidiTrack& midi_track,
                           const std::shared_ptr<SongGlobalData>& global_data)
{
    const auto event_track
        = read_instrument_midi_track(midi_track, TrackType::Drums);

    const TomEvents tom_events {event_track};

    std::map<Difficulty, std::vector<Note>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour, flags] = key;
        const std::tuple<Difficulty, int> no_flags_key {diff, colour};
        if (event_track.note_off_events.count(no_flags_key) == 0) {
            throw ParseError("No corresponding Note Off events");
        }
        const auto& note_offs = event_track.note_off_events.at(no_flags_key);
        for (const auto& [pos, end] :
             combine_note_on_off_events(note_ons, note_offs)) {
            Note note;
            note.position = Tick {pos};
            note.lengths.at(colour) = Tick {0};
            note.flags = flags;
            if (tom_events.force_tom(colour, pos)) {
                note.flags = static_cast<NoteFlags>(note.flags & ~FLAGS_CYMBAL);
            }
            notes[diff].push_back(note);
        }
        fix_double_greens(notes[diff]);
    }

    std::vector<StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back({Tick {start}, Tick {end - start}});
    }

    std::vector<DrumFill> drum_fills;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.fill_on_events, event_track.fill_off_events)) {
        drum_fills.push_back({Tick {start}, Tick {end - start}});
    }

    std::map<Difficulty, NoteTrack> note_tracks;
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
            disco_flips.push_back({Tick {start}, Tick {end - start}});
        }
        auto solos = form_solo_vector(solo_ons, solo_offs, note_set,
                                      TrackType::Drums, true);
        note_tracks.emplace(diff,
                            NoteTrack {note_set,
                                       sp_phrases,
                                       std::move(solos),
                                       drum_fills,
                                       std::move(disco_flips),
                                       {},
                                       TrackType::Drums,
                                       global_data});
    }

    return note_tracks;
}

std::optional<std::string> midi_track_name(const MidiTrack& track)
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

std::optional<Instrument> midi_section_instrument(const std::string& track_name)
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

std::vector<Tick> od_beats_from_track(const MidiTrack& track)
{
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;
    constexpr int BEAT_LOW_KEY = 12;
    constexpr int BEAT_HIGH_KEY = 13;

    std::vector<Tick> od_beats;

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
            od_beats.emplace_back(event.time);
        }
    }

    return od_beats;
}
}

Song song_from_filename(const std::string& filename)
{
    std::string ini_file;
    const std::filesystem::path song_path {filename};
    const auto song_directory = song_path.parent_path();
    const auto ini_path = song_directory / "song.ini";
    boost::nowide::ifstream ini_in {ini_path.string()};
    if (ini_in.is_open()) {
        ini_file = std::string {std::istreambuf_iterator<char>(ini_in),
                                std::istreambuf_iterator<char>()};
    }
    const auto ini = parse_ini(ini_file);

    if (ends_with_suffix(filename, ".chart")) {
        boost::nowide::ifstream in {filename};
        if (!in.is_open()) {
            throw std::invalid_argument("File did not open");
        }
        const std::string contents {std::istreambuf_iterator<char>(in),
                                    std::istreambuf_iterator<char>()};
        return Song::from_chart(parse_chart(contents), ini);
    }
    if (ends_with_suffix(filename, ".mid")) {
        boost::nowide::ifstream in {filename, std::ios::binary};
        if (!in.is_open()) {
            throw std::invalid_argument("File did not open");
        }
        const std::vector<std::uint8_t> buffer {
            std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>()};
        return Song::from_midi(parse_midi(buffer), ini);
    }
    throw std::invalid_argument("file should be .chart or .mid");
}

std::vector<Instrument> Song::instruments() const
{
    std::set<Instrument> instrument_set;
    for (const auto& [key, val] : m_tracks) {
        instrument_set.insert(std::get<0>(key));
    }

    std::vector<Instrument> instruments {instrument_set.cbegin(),
                                         instrument_set.cend()};
    std::sort(instruments.begin(), instruments.end());
    return instruments;
}

std::vector<Difficulty> Song::difficulties(Instrument instrument) const
{
    std::vector<Difficulty> difficulties;
    for (const auto& [key, val] : m_tracks) {
        if (std::get<0>(key) == instrument) {
            difficulties.push_back(std::get<1>(key));
        }
    }
    std::sort(difficulties.begin(), difficulties.end());
    return difficulties;
}

void Song::append_instrument_track(Instrument inst, Difficulty diff,
                                   const ChartSection& section)
{
    auto note_track = note_track_from_section(section, m_global_data,
                                              track_type_from_instrument(inst));
    if (!note_track.notes().empty()) {
        m_tracks.insert({{inst, diff}, note_track});
    }
}

Song Song::from_chart(const Chart& chart, const IniValues& ini)
{
    Song song;

    song.m_global_data->is_from_midi(false);
    song.m_global_data->name(ini.name);
    song.m_global_data->artist(ini.artist);
    song.m_global_data->charter(ini.charter);

    for (const auto& section : chart.sections) {
        if (section.name == "Song") {
            try {
                const auto resolution = std::stoi(get_with_default(
                    section.key_value_pairs, "Resolution", "192"));
                song.m_global_data->resolution(resolution);
            } catch (const std::invalid_argument&) {
                // CH just ignores this kind of parsing mistake.
                // TODO: Use from_chars instead to avoid having to use
                // exceptions as control flow.
            }
        } else if (section.name == "SyncTrack") {
            song.m_global_data->tempo_map(tempo_map_from_section(
                section, song.m_global_data->resolution()));
        } else {
            auto pair = diff_inst_from_header(section.name);
            if (!pair.has_value()) {
                continue;
            }
            auto [diff, inst] = *pair;
            song.append_instrument_track(inst, diff, section);
        }
    }

    if (song.m_tracks.empty()) {
        throw ParseError("Chart has no notes");
    }

    return song;
}

Song Song::from_midi(const Midi& midi, const IniValues& ini)
{
    if (midi.ticks_per_quarter_note == 0) {
        throw ParseError("Resolution must be > 0");
    }

    Song song;

    song.m_global_data->is_from_midi(true);
    song.m_global_data->resolution(midi.ticks_per_quarter_note);
    song.m_global_data->name(ini.name);
    song.m_global_data->artist(ini.artist);
    song.m_global_data->charter(ini.charter);

    if (midi.tracks.empty()) {
        return song;
    }

    song.m_global_data->tempo_map(
        read_first_midi_track(midi.tracks[0], midi.ticks_per_quarter_note));

    for (const auto& track : midi.tracks) {
        const auto track_name = midi_track_name(track);
        if (!track_name.has_value()) {
            continue;
        }
        if (*track_name == "BEAT") {
            song.m_global_data->od_beats(od_beats_from_track(track));
        }
        const auto inst = midi_section_instrument(*track_name);
        if (!inst.has_value()) {
            continue;
        }
        if (is_six_fret_instrument(*inst)) {
            auto tracks = ghl_note_tracks_from_midi(track, song.m_global_data);
            for (auto& [diff, note_track] : tracks) {
                song.m_tracks.emplace(std::tuple {*inst, diff},
                                      std::move(note_track));
            }
        } else if (*inst == Instrument::Drums) {
            auto tracks = drum_note_tracks_from_midi(track, song.m_global_data);
            for (auto& [diff, note_track] : tracks) {
                song.m_tracks.emplace(std::tuple {Instrument::Drums, diff},
                                      std::move(note_track));
            }
        } else {
            auto tracks = note_tracks_from_midi(track, song.m_global_data);
            for (auto& [diff, note_track] : tracks) {
                song.m_tracks.emplace(std::tuple {*inst, diff},
                                      std::move(note_track));
            }
        }
    }

    return song;
}

const NoteTrack& Song::track(Instrument instrument, Difficulty difficulty) const
{
    const auto insts = instruments();
    if (std::find(insts.cbegin(), insts.cend(), instrument) == insts.cend()) {
        throw std::invalid_argument("Chosen instrument not present in song");
    }
    const auto diffs = difficulties(instrument);
    if (std::find(diffs.cbegin(), diffs.cend(), difficulty) == diffs.cend()) {
        throw std::invalid_argument(
            "Difficulty not available for chosen instrument");
    }
    return m_tracks.at({instrument, difficulty});
}

std::vector<Tick> Song::unison_phrase_positions() const
{
    std::map<Tick, std::set<Instrument>> phrase_by_instrument;
    for (const auto& [key, value] : m_tracks) {
        const auto instrument = std::get<0>(key);
        if (is_six_fret_instrument(instrument)) {
            continue;
        }
        for (const auto& phrase : value.sp_phrases()) {
            phrase_by_instrument[phrase.position].insert(instrument);
        }
    }

    std::vector<Tick> unison_starts;
    for (const auto& [key, value] : phrase_by_instrument) {
        if (value.size() > 1) {
            unison_starts.push_back(key);
        }
    }
    return unison_starts;
}
