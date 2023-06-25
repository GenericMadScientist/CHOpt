/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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
#include <limits>
#include <set>

#include <sightread/detail/parserutil.hpp>

#include "midiparser.hpp"

namespace {
SightRead::TempoMap
read_first_midi_track(const SightRead::Detail::MidiTrack& track, int resolution)
{
    constexpr int SET_TEMPO_ID = 0x51;
    constexpr int TIME_SIG_ID = 0x58;

    std::vector<SightRead::BPM> tempos;
    std::vector<SightRead::TimeSignature> time_sigs;
    for (const auto& event : track.events) {
        const auto* meta_event
            = std::get_if<SightRead::Detail::MetaEvent>(&event.event);
        if (meta_event == nullptr) {
            continue;
        }
        switch (meta_event->type) {
        case SET_TEMPO_ID: {
            if (meta_event->data.size() < 3) {
                throw SightRead::ParseError("Tempo meta event too short");
            }
            const auto us_per_quarter = meta_event->data[0] << 16
                | meta_event->data[1] << 8 | meta_event->data[2];
            const auto bpm = 60000000000 / us_per_quarter;
            tempos.push_back(
                {SightRead::Tick {event.time}, static_cast<int>(bpm)});
            break;
        }
        case TIME_SIG_ID:
            if (meta_event->data.size() < 2) {
                throw SightRead::ParseError("Tempo meta event too short");
            }
            if (meta_event->data[1] >= (CHAR_BIT * sizeof(int))) {
                throw SightRead::ParseError("Time sig denominator too large");
            }
            time_sigs.push_back({SightRead::Tick {event.time},
                                 meta_event->data[0],
                                 1 << meta_event->data[1]});
            break;
        }
    }

    return {std::move(time_sigs), std::move(tempos), {}, resolution};
}

std::optional<std::string>
midi_track_name(const SightRead::Detail::MidiTrack& track)
{
    if (track.events.empty()) {
        return std::nullopt;
    }
    for (const auto& event : track.events) {
        const auto* meta_event
            = std::get_if<SightRead::Detail::MetaEvent>(&event.event);
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

std::vector<SightRead::Tick>
od_beats_from_track(const SightRead::Detail::MidiTrack& track)
{
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;
    constexpr int BEAT_LOW_KEY = 12;
    constexpr int BEAT_HIGH_KEY = 13;

    std::vector<SightRead::Tick> od_beats;

    for (const auto& event : track.events) {
        const auto* midi_event
            = std::get_if<SightRead::Detail::MidiEvent>(&event.event);
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

std::optional<SightRead::Instrument>
midi_section_instrument(const std::string& track_name)
{
    const std::map<std::string, SightRead::Instrument> INSTRUMENTS {
        {"PART GUITAR", SightRead::Instrument::Guitar},
        {"T1 GEMS", SightRead::Instrument::Guitar},
        {"PART GUITAR COOP", SightRead::Instrument::GuitarCoop},
        {"PART BASS", SightRead::Instrument::Bass},
        {"PART RHYTHM", SightRead::Instrument::Rhythm},
        {"PART KEYS", SightRead::Instrument::Keys},
        {"PART GUITAR GHL", SightRead::Instrument::GHLGuitar},
        {"PART BASS GHL", SightRead::Instrument::GHLBass},
        {"PART RHYTHM GHL", SightRead::Instrument::GHLRhythm},
        {"PART GUITAR COOP GHL", SightRead::Instrument::GHLGuitarCoop},
        {"PART DRUMS", SightRead::Instrument::Drums}};

    const auto iter = INSTRUMENTS.find(track_name);
    if (iter == INSTRUMENTS.end()) {
        return std::nullopt;
    }
    return iter->second;
}

bool is_five_lane_green_note(const SightRead::Detail::TimedEvent& event)
{
    constexpr std::array<std::uint8_t, 4> GREEN_LANE_KEYS {65, 77, 89, 101};
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    const auto* midi_event
        = std::get_if<SightRead::Detail::MidiEvent>(&event.event);
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

bool has_five_lane_green_notes(const SightRead::Detail::MidiTrack& midi_track)
{
    return std::find_if(midi_track.events.cbegin(), midi_track.events.cend(),
                        is_five_lane_green_note)
        != midi_track.events.cend();
}

bool is_enable_chart_dynamics(const SightRead::Detail::TimedEvent& event)
{
    using namespace std::literals;
    constexpr auto ENABLE_DYNAMICS = "[ENABLE_CHART_DYNAMICS]"sv;

    const auto* meta_event
        = std::get_if<SightRead::Detail::MetaEvent>(&event.event);
    if (meta_event == nullptr) {
        return false;
    }
    if (meta_event->type != 1) {
        return false;
    }
    return std::equal(meta_event->data.cbegin(), meta_event->data.cend(),
                      ENABLE_DYNAMICS.cbegin(), ENABLE_DYNAMICS.cend());
}

bool has_enable_chart_dynamics(const SightRead::Detail::MidiTrack& midi_track)
{
    return std::find_if(midi_track.events.cbegin(), midi_track.events.cend(),
                        is_enable_chart_dynamics)
        != midi_track.events.cend();
}

bool is_open_event_sysex(const SightRead::Detail::SysexEvent& event)
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

std::optional<SightRead::Difficulty>
look_up_difficulty(const std::array<std::tuple<int, int, SightRead::Difficulty>,
                                    4>& diff_ranges,
                   std::uint8_t key)
{
    for (const auto& [min, max, diff] : diff_ranges) {
        if (key >= min && key <= max) {
            return {diff};
        }
    }

    return std::nullopt;
}

std::optional<SightRead::Difficulty>
difficulty_from_key(std::uint8_t key, SightRead::TrackType track_type)
{
    std::array<std::tuple<int, int, SightRead::Difficulty>, 4> diff_ranges;
    switch (track_type) {
    case SightRead::TrackType::FiveFret:
        diff_ranges = {{{96, 102, SightRead::Difficulty::Expert}, // NOLINT
                        {84, 90, SightRead::Difficulty::Hard}, // NOLINT
                        {72, 78, SightRead::Difficulty::Medium}, // NOLINT
                        {60, 66, SightRead::Difficulty::Easy}}}; // NOLINT
        break;
    case SightRead::TrackType::SixFret:
        diff_ranges = {{{94, 102, SightRead::Difficulty::Expert}, // NOLINT
                        {82, 90, SightRead::Difficulty::Hard}, // NOLINT
                        {70, 78, SightRead::Difficulty::Medium}, // NOLINT
                        {58, 66, SightRead::Difficulty::Easy}}}; // NOLINT
        break;
    case SightRead::TrackType::Drums:
        diff_ranges = {{{95, 101, SightRead::Difficulty::Expert}, // NOLINT
                        {83, 89, SightRead::Difficulty::Hard}, // NOLINT
                        {71, 77, SightRead::Difficulty::Medium}, // NOLINT
                        {59, 65, SightRead::Difficulty::Easy}}}; // NOLINT
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

    throw SightRead::ParseError("Invalid key for note");
}

int colour_from_key(std::uint8_t key, SightRead::TrackType track_type,
                    bool from_five_lane)
{
    std::array<unsigned int, 4> diff_ranges {};
    switch (track_type) {
    case SightRead::TrackType::FiveFret: {
        diff_ranges = {96, 84, 72, 60}; // NOLINT
        constexpr std::array NOTE_COLOURS {
            SightRead::FIVE_FRET_GREEN, SightRead::FIVE_FRET_RED,
            SightRead::FIVE_FRET_YELLOW, SightRead::FIVE_FRET_BLUE,
            SightRead::FIVE_FRET_ORANGE};
        return colour_from_key_and_bounds(key, diff_ranges, NOTE_COLOURS);
    }
    case SightRead::TrackType::SixFret: {
        diff_ranges = {94, 82, 70, 58}; // NOLINT
        constexpr std::array GHL_NOTE_COLOURS {
            SightRead::SIX_FRET_OPEN,      SightRead::SIX_FRET_WHITE_LOW,
            SightRead::SIX_FRET_WHITE_MID, SightRead::SIX_FRET_WHITE_HIGH,
            SightRead::SIX_FRET_BLACK_LOW, SightRead::SIX_FRET_BLACK_MID,
            SightRead::SIX_FRET_BLACK_HIGH};
        return colour_from_key_and_bounds(key, diff_ranges, GHL_NOTE_COLOURS);
    }
    case SightRead::TrackType::Drums: {
        diff_ranges = {95, 83, 71, 59}; // NOLINT
        constexpr std::array DRUM_NOTE_COLOURS {
            SightRead::DRUM_DOUBLE_KICK, SightRead::DRUM_KICK,
            SightRead::DRUM_RED,         SightRead::DRUM_YELLOW,
            SightRead::DRUM_BLUE,        SightRead::DRUM_GREEN};
        constexpr std::array FIVE_LANE_COLOURS {
            SightRead::DRUM_DOUBLE_KICK, SightRead::DRUM_KICK,
            SightRead::DRUM_RED,         SightRead::DRUM_YELLOW,
            SightRead::DRUM_BLUE,        SightRead::DRUM_GREEN,
            SightRead::DRUM_GREEN};
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

SightRead::NoteFlags flags_from_track_type(SightRead::TrackType track_type)
{
    switch (track_type) {
    case SightRead::TrackType::FiveFret:
        return SightRead::FLAGS_FIVE_FRET_GUITAR;
    case SightRead::TrackType::SixFret:
        return SightRead::FLAGS_SIX_FRET_GUITAR;
    case SightRead::TrackType::Drums:
        return SightRead::FLAGS_DRUMS;
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

SightRead::NoteFlags dynamics_flags_from_velocity(std::uint8_t velocity)
{
    constexpr std::uint8_t MIN_ACCENT_VELOCITY = 127;

    if (velocity == 1) {
        return SightRead::FLAGS_GHOST;
    }
    if (velocity >= MIN_ACCENT_VELOCITY) {
        return SightRead::FLAGS_ACCENT;
    }
    return static_cast<SightRead::NoteFlags>(0);
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
        throw SightRead::ParseError("on event has no corresponding off event");
    }

    return ranges;
}

struct InstrumentMidiTrack {
public:
    std::map<std::tuple<SightRead::Difficulty, int, SightRead::NoteFlags>,
             std::vector<std::tuple<int, int>>>
        note_on_events;
    std::map<std::tuple<SightRead::Difficulty, int>,
             std::vector<std::tuple<int, int>>>
        note_off_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        open_on_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        open_off_events;
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
    std::vector<std::tuple<int, int>> tap_on_events;
    std::vector<std::tuple<int, int>> tap_off_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        force_hopo_on_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        force_hopo_off_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        force_strum_on_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        force_strum_off_events;
    std::vector<std::tuple<int, int>> fill_on_events;
    std::vector<std::tuple<int, int>> fill_off_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        disco_flip_on_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        disco_flip_off_events;

    InstrumentMidiTrack() = default;
};

void add_sysex_event(InstrumentMidiTrack& track,
                     const SightRead::Detail::SysexEvent& event, int time,
                     int rank)
{
    constexpr std::array<SightRead::Difficulty, 4> OPEN_EVENT_DIFFS {
        SightRead::Difficulty::Easy, SightRead::Difficulty::Medium,
        SightRead::Difficulty::Hard, SightRead::Difficulty::Expert};
    constexpr int SYSEX_ON_INDEX = 6;

    if (!is_open_event_sysex(event)) {
        return;
    }
    const auto diff = OPEN_EVENT_DIFFS.at(event.data[4]);
    if (event.data[SYSEX_ON_INDEX] == 0) {
        track.open_off_events[diff].emplace_back(time, rank);
    } else {
        track.open_on_events[diff].emplace_back(time, rank);
    }
}

void append_disco_flip(InstrumentMidiTrack& event_track,
                       const SightRead::Detail::MetaEvent& meta_event, int time,
                       int rank)
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
        = static_cast<SightRead::Difficulty>(meta_event.data[MIX.size()] - '0');
    if (meta_event.data.size() == FLIP_END_SIZE
        && meta_event.data[FLIP_END_SIZE - 1] == ']') {
        event_track.disco_flip_off_events[diff].emplace_back(time, rank);
    } else if (meta_event.data.size() == FLIP_START_SIZE
               && meta_event.data[FLIP_START_SIZE - 2] == 'd'
               && meta_event.data[FLIP_START_SIZE - 1] == ']') {
        event_track.disco_flip_on_events[diff].emplace_back(time, rank);
    }
}

bool force_hopo_key(std::uint8_t key, SightRead::TrackType track_type)
{
    constexpr std::array FORCE_HOPO_KEYS {65, 77, 89, 101};
    if (track_type == SightRead::TrackType::Drums) {
        return false;
    }
    return std::find(FORCE_HOPO_KEYS.cbegin(), FORCE_HOPO_KEYS.cend(), key)
        != FORCE_HOPO_KEYS.cend();
}

bool force_strum_key(std::uint8_t key, SightRead::TrackType track_type)
{
    constexpr std::array FORCE_STRUM_KEYS {66, 78, 90, 102};
    if (track_type == SightRead::TrackType::Drums) {
        return false;
    }
    return std::find(FORCE_STRUM_KEYS.cbegin(), FORCE_STRUM_KEYS.cend(), key)
        != FORCE_STRUM_KEYS.cend();
}

void add_note_off_event(InstrumentMidiTrack& track,
                        const std::array<std::uint8_t, 2>& data, int time,
                        int rank, bool from_five_lane,
                        SightRead::TrackType track_type)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;
    constexpr int TAP_NOTE_ID = 104;
    constexpr int DRUM_FILL_ID = 120;

    const auto diff = difficulty_from_key(data[0], track_type);
    if (diff.has_value()) {
        if (force_hopo_key(data[0], track_type)) {
            track.force_hopo_off_events[*diff].emplace_back(time, rank);
        } else if (force_strum_key(data[0], track_type)) {
            track.force_strum_off_events[*diff].emplace_back(time, rank);
        } else {
            const auto colour
                = colour_from_key(data[0], track_type, from_five_lane);
            track.note_off_events[{*diff, colour}].emplace_back(time, rank);
        }
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
    } else if (data[0] == TAP_NOTE_ID) {
        track.tap_off_events.emplace_back(time, rank);
    } else if (data[0] == DRUM_FILL_ID) {
        track.fill_off_events.emplace_back(time, rank);
    }
}

void add_note_on_event(InstrumentMidiTrack& track,
                       const std::array<std::uint8_t, 2>& data, int time,
                       int rank, bool from_five_lane, bool parse_dynamics,
                       SightRead::TrackType track_type)
{
    constexpr int YELLOW_TOM_ID = 110;
    constexpr int BLUE_TOM_ID = 111;
    constexpr int GREEN_TOM_ID = 112;
    constexpr int SOLO_NOTE_ID = 103;
    constexpr int SP_NOTE_ID = 116;
    constexpr int TAP_NOTE_ID = 104;
    constexpr int DRUM_FILL_ID = 120;

    // Velocity 0 Note On events are counted as Note Off events.
    if (data[1] == 0) {
        add_note_off_event(track, data, time, rank, from_five_lane, track_type);
        return;
    }

    const auto diff = difficulty_from_key(data[0], track_type);
    if (diff.has_value()) {
        if (force_hopo_key(data[0], track_type)) {
            track.force_hopo_on_events[*diff].emplace_back(time, rank);
        } else if (force_strum_key(data[0], track_type)) {
            track.force_strum_on_events[*diff].emplace_back(time, rank);
        } else {
            auto colour = colour_from_key(data[0], track_type, from_five_lane);
            auto flags = flags_from_track_type(track_type);
            if (track_type == SightRead::TrackType::Drums) {
                if (is_cymbal_key(data[0], from_five_lane)) {
                    flags = static_cast<SightRead::NoteFlags>(
                        flags | SightRead::FLAGS_CYMBAL);
                }
                if (parse_dynamics) {
                    flags = static_cast<SightRead::NoteFlags>(
                        flags | dynamics_flags_from_velocity(data[1]));
                }
            }
            track.note_on_events[{*diff, colour, flags}].emplace_back(time,
                                                                      rank);
        }
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
    } else if (data[0] == TAP_NOTE_ID) {
        track.tap_on_events.emplace_back(time, rank);
    } else if (data[0] == DRUM_FILL_ID) {
        track.fill_on_events.emplace_back(time, rank);
    }
}

InstrumentMidiTrack
read_instrument_midi_track(const SightRead::Detail::MidiTrack& midi_track,
                           SightRead::TrackType track_type)
{
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;
    constexpr std::array DIFFICULTIES {
        SightRead::Difficulty::Easy, SightRead::Difficulty::Medium,
        SightRead::Difficulty::Hard, SightRead::Difficulty::Expert};

    const bool from_five_lane = track_type == SightRead::TrackType::Drums
        && has_five_lane_green_notes(midi_track);
    const bool parse_dynamics = track_type == SightRead::TrackType::Drums
        && has_enable_chart_dynamics(midi_track);

    InstrumentMidiTrack event_track;
    for (auto d : DIFFICULTIES) {
        event_track.disco_flip_on_events[d] = {};
        event_track.disco_flip_off_events[d] = {};
        event_track.force_hopo_on_events[d] = {};
        event_track.force_hopo_off_events[d] = {};
        event_track.force_strum_on_events[d] = {};
        event_track.force_strum_off_events[d] = {};
    }

    int rank = 0;
    for (const auto& event : midi_track.events) {
        ++rank;
        const auto* midi_event
            = std::get_if<SightRead::Detail::MidiEvent>(&event.event);
        if (midi_event == nullptr) {
            const auto* sysex_event
                = std::get_if<SightRead::Detail::SysexEvent>(&event.event);
            if (sysex_event != nullptr) {
                add_sysex_event(event_track, *sysex_event, event.time, rank);
                continue;
            }
            if (track_type == SightRead::TrackType::Drums) {
                const auto* meta_event
                    = std::get_if<SightRead::Detail::MetaEvent>(&event.event);
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

    event_track.disco_flip_off_events.at(SightRead::Difficulty::Easy)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);
    event_track.disco_flip_off_events.at(SightRead::Difficulty::Medium)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);
    event_track.disco_flip_off_events.at(SightRead::Difficulty::Hard)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);
    event_track.disco_flip_off_events.at(SightRead::Difficulty::Expert)
        .emplace_back(std::numeric_limits<int>::max(), ++rank);

    if (event_track.sp_on_events.empty()
        && event_track.solo_on_events.size() > 1) {
        std::swap(event_track.sp_off_events, event_track.solo_off_events);
        std::swap(event_track.sp_on_events, event_track.solo_on_events);
    }

    return event_track;
}

bool position_in_event_spans(
    const std::vector<std::tuple<int, int>>& event_spans, int position)
{
    return std::any_of(event_spans.cbegin(), event_spans.cend(),
                       [&](const auto& span) {
                           return position >= std::get<0>(span)
                               && position < std::get<1>(span);
                       });
}

std::map<SightRead::Difficulty, std::vector<SightRead::Note>>
notes_from_event_track(
    const InstrumentMidiTrack& event_track,
    const std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>&
        open_events,
    SightRead::TrackType track_type)
{
    constexpr std::array DIFFICULTIES {
        SightRead::Difficulty::Easy, SightRead::Difficulty::Medium,
        SightRead::Difficulty::Hard, SightRead::Difficulty::Expert};

    const auto tap_events = combine_note_on_off_events(
        event_track.tap_on_events, event_track.tap_off_events);
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        force_hopo_events;
    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        force_strum_events;
    for (auto d : DIFFICULTIES) {
        force_hopo_events[d] = combine_note_on_off_events(
            event_track.force_hopo_on_events.at(d),
            event_track.force_hopo_off_events.at(d));
        force_strum_events[d] = combine_note_on_off_events(
            event_track.force_strum_on_events.at(d),
            event_track.force_strum_off_events.at(d));
    }

    std::map<SightRead::Difficulty, std::vector<SightRead::Note>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour, flags] = key;
        if (!event_track.note_off_events.contains({diff, colour})) {
            throw SightRead::ParseError("No corresponding Note Off events");
        }
        const auto& note_offs = event_track.note_off_events.at({diff, colour});
        for (const auto& [pos, end] :
             combine_note_on_off_events(note_ons, note_offs)) {
            const auto note_length = end - pos;
            auto note_colour = colour;
            if (track_type == SightRead::TrackType::FiveFret) {
                const auto open_events_iter = open_events.find(diff);
                if (open_events_iter != open_events.cend()
                    && position_in_event_spans(open_events_iter->second, pos)) {
                    note_colour = SightRead::FIVE_FRET_OPEN;
                }
            }
            SightRead::Note note;
            note.position = SightRead::Tick {pos};
            note.lengths.at(note_colour) = SightRead::Tick {note_length};
            note.flags = flags_from_track_type(track_type);
            if (position_in_event_spans(tap_events, pos)
                && track_type != SightRead::TrackType::Drums) {
                note.flags = static_cast<SightRead::NoteFlags>(
                    note.flags | SightRead::FLAGS_TAP);
            }
            if (position_in_event_spans(force_hopo_events[diff], pos)) {
                note.flags = static_cast<SightRead::NoteFlags>(
                    note.flags | SightRead::FLAGS_FORCE_HOPO);
            }
            if (position_in_event_spans(force_strum_events[diff], pos)) {
                note.flags = static_cast<SightRead::NoteFlags>(
                    note.flags | SightRead::FLAGS_FORCE_STRUM);
            }
            notes[diff].push_back(note);
        }
    }

    return notes;
}

std::map<SightRead::Difficulty, SightRead::NoteTrack> ghl_note_tracks_from_midi(
    const SightRead::Detail::MidiTrack& midi_track,
    const std::shared_ptr<SightRead::SongGlobalData>& global_data,
    const SightRead::HopoThreshold& hopo_threshold, bool permit_solos)
{
    const auto event_track
        = read_instrument_midi_track(midi_track, SightRead::TrackType::SixFret);

    const auto notes = notes_from_event_track(event_track, {},
                                              SightRead::TrackType::SixFret);

    std::vector<SightRead::StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back(
            {SightRead::Tick {start}, SightRead::Tick {end - start}});
    }

    std::map<SightRead::Difficulty, SightRead::NoteTrack> note_tracks;
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
        auto solos = SightRead::Detail::form_solo_vector(
            solo_ons, solo_offs, note_set, SightRead::TrackType::SixFret, true);
        if (!permit_solos) {
            solos.clear();
        }
        SightRead::NoteTrack note_track {
            note_set, sp_phrases, SightRead::TrackType::SixFret, global_data,
            hopo_threshold.midi_max_hopo_gap(global_data->resolution())};
        note_track.solos(std::move(solos));
        note_tracks.emplace(diff, std::move(note_track));
    }

    return note_tracks;
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
        if (colour == SightRead::DRUM_YELLOW) {
            for (const auto& [open_start, open_end] : m_yellow_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return true;
                }
            }
        } else if (colour == SightRead::DRUM_BLUE) {
            for (const auto& [open_start, open_end] : m_blue_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return true;
                }
            }
        } else if (colour == SightRead::DRUM_GREEN) {
            for (const auto& [open_start, open_end] : m_green_tom_events) {
                if (pos >= open_start && pos < open_end) {
                    return true;
                }
            }
        }
        return false;
    }
};

// This is to deal with G cymbal + G tom from five lane being turned into G
// cymbal + B tom. This combination cannot happen from a four lane chart.
void fix_double_greens(std::vector<SightRead::Note>& notes)
{
    std::set<SightRead::Tick> green_cymbal_positions;

    for (const auto& note : notes) {
        if ((note.lengths.at(SightRead::DRUM_GREEN) != SightRead::Tick {-1})
            && ((note.flags & SightRead::FLAGS_CYMBAL) != 0U)) {
            green_cymbal_positions.insert(note.position);
        }
    }

    for (auto& note : notes) {
        if ((note.lengths.at(SightRead::DRUM_GREEN) == SightRead::Tick {-1})
            || ((note.flags & SightRead::FLAGS_CYMBAL) != 0U)) {
            continue;
        }
        if (green_cymbal_positions.contains(note.position)) {
            std::swap(note.lengths[SightRead::DRUM_BLUE],
                      note.lengths[SightRead::DRUM_GREEN]);
        }
    }
}

std::map<SightRead::Difficulty, SightRead::NoteTrack>
drum_note_tracks_from_midi(
    const SightRead::Detail::MidiTrack& midi_track,
    const std::shared_ptr<SightRead::SongGlobalData>& global_data,
    bool permit_solos)
{
    const auto event_track
        = read_instrument_midi_track(midi_track, SightRead::TrackType::Drums);

    const TomEvents tom_events {event_track};

    std::map<SightRead::Difficulty, std::vector<SightRead::Note>> notes;
    for (const auto& [key, note_ons] : event_track.note_on_events) {
        const auto& [diff, colour, flags] = key;
        const std::tuple<SightRead::Difficulty, int> no_flags_key {diff,
                                                                   colour};
        if (!event_track.note_off_events.contains(no_flags_key)) {
            throw SightRead::ParseError("No corresponding Note Off events");
        }
        const auto& note_offs = event_track.note_off_events.at(no_flags_key);
        for (const auto& [pos, end] :
             combine_note_on_off_events(note_ons, note_offs)) {
            SightRead::Note note;
            note.position = SightRead::Tick {pos};
            note.lengths.at(colour) = SightRead::Tick {0};
            note.flags = flags;
            if (tom_events.force_tom(colour, pos)) {
                note.flags = static_cast<SightRead::NoteFlags>(
                    note.flags & ~SightRead::FLAGS_CYMBAL);
            }
            notes[diff].push_back(note);
        }
        fix_double_greens(notes[diff]);
    }

    std::vector<SightRead::StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back(
            {SightRead::Tick {start}, SightRead::Tick {end - start}});
    }

    std::vector<SightRead::DrumFill> drum_fills;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.fill_on_events, event_track.fill_off_events)) {
        drum_fills.push_back(
            {SightRead::Tick {start}, SightRead::Tick {end - start}});
    }

    std::map<SightRead::Difficulty, SightRead::NoteTrack> note_tracks;
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
        std::vector<SightRead::DiscoFlip> disco_flips;
        for (const auto& [start, end] : combine_note_on_off_events(
                 event_track.disco_flip_on_events.at(diff),
                 event_track.disco_flip_off_events.at(diff))) {
            disco_flips.push_back(
                {SightRead::Tick {start}, SightRead::Tick {end - start}});
        }
        auto solos = SightRead::Detail::form_solo_vector(
            solo_ons, solo_offs, note_set, SightRead::TrackType::Drums, true);
        if (!permit_solos) {
            solos.clear();
        }
        SightRead::NoteTrack note_track {
            note_set, sp_phrases, SightRead::TrackType::Drums, global_data};
        note_track.solos(std::move(solos));
        note_track.drum_fills(drum_fills);
        note_track.disco_flips(std::move(disco_flips));
        note_tracks.emplace(diff, std::move(note_track));
    }

    return note_tracks;
}

std::optional<SightRead::BigRockEnding>
read_bre(const SightRead::Detail::MidiTrack& midi_track)
{
    constexpr int BRE_KEY = 120;
    constexpr int NOTE_OFF_ID = 0x80;
    constexpr int NOTE_ON_ID = 0x90;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    SightRead::Tick bre_start {0};

    for (const auto& event : midi_track.events) {
        const auto* midi_event
            = std::get_if<SightRead::Detail::MidiEvent>(&event.event);
        if (midi_event == nullptr) {
            continue;
        }
        if (midi_event->data[0] != BRE_KEY) {
            continue;
        }
        const auto event_type = midi_event->status & UPPER_NIBBLE_MASK;
        if (event_type == NOTE_OFF_ID
            || (event_type == NOTE_ON_ID && midi_event->data[1] == 0)) {
            const SightRead::Tick bre_end {event.time};
            return {{bre_start, bre_end}};
        }
        if (event_type == NOTE_ON_ID) {
            bre_start = SightRead::Tick {event.time};
        }
    }

    return std::nullopt;
}

std::map<SightRead::Difficulty, SightRead::NoteTrack> note_tracks_from_midi(
    const SightRead::Detail::MidiTrack& midi_track,
    const std::shared_ptr<SightRead::SongGlobalData>& global_data,
    const SightRead::HopoThreshold& hopo_threshold, bool permit_solos)
{
    const auto event_track = read_instrument_midi_track(
        midi_track, SightRead::TrackType::FiveFret);
    const auto bre = read_bre(midi_track);

    std::map<SightRead::Difficulty, std::vector<std::tuple<int, int>>>
        open_events;
    for (const auto& [diff, open_ons] : event_track.open_on_events) {
        if (!event_track.open_off_events.contains(diff)) {
            throw SightRead::ParseError("No open Note Off events");
        }
        const auto& open_offs = event_track.open_off_events.at(diff);
        open_events[diff] = combine_note_on_off_events(open_ons, open_offs);
    }

    const auto notes = notes_from_event_track(event_track, open_events,
                                              SightRead::TrackType::FiveFret);

    std::vector<SightRead::StarPower> sp_phrases;
    for (const auto& [start, end] : combine_note_on_off_events(
             event_track.sp_on_events, event_track.sp_off_events)) {
        sp_phrases.push_back(
            {SightRead::Tick {start}, SightRead::Tick {end - start}});
    }

    std::map<SightRead::Difficulty, SightRead::NoteTrack> note_tracks;
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
        auto solos = SightRead::Detail::form_solo_vector(
            solo_ons, solo_offs, note_set, SightRead::TrackType::FiveFret,
            true);
        if (!permit_solos) {
            solos.clear();
        }
        SightRead::NoteTrack note_track {
            note_set, sp_phrases, SightRead::TrackType::FiveFret, global_data,
            hopo_threshold.midi_max_hopo_gap(global_data->resolution())};
        note_track.solos(std::move(solos));
        note_track.bre(bre);
        note_tracks.emplace(diff, std::move(note_track));
    }

    return note_tracks;
}
}

MidiParser::MidiParser(const IniValues& ini)
    : m_song_name {ini.name}
    , m_artist {ini.artist}
    , m_charter {ini.charter}
    , m_hopo_threshold {SightRead::HopoThresholdType::Resolution,
                        SightRead::Tick {0}}
    , m_permitted_instruments {SightRead::all_instruments()}
    , m_permit_solos {true}
{
}

MidiParser& MidiParser::hopo_threshold(SightRead::HopoThreshold hopo_threshold)
{
    m_hopo_threshold = hopo_threshold;
    return *this;
}

MidiParser& MidiParser::permit_instruments(
    std::set<SightRead::Instrument> permitted_instruments)
{
    m_permitted_instruments = std::move(permitted_instruments);
    return *this;
}

MidiParser& MidiParser::parse_solos(bool permit_solos)
{
    m_permit_solos = permit_solos;
    return *this;
}

SightRead::Song MidiParser::from_midi(const SightRead::Detail::Midi& midi) const
{
    if (midi.ticks_per_quarter_note == 0) {
        throw SightRead::ParseError("Resolution must be > 0");
    }

    SightRead::Song song;

    song.global_data().is_from_midi(true);
    song.global_data().resolution(midi.ticks_per_quarter_note);
    song.global_data().name(m_song_name);
    song.global_data().artist(m_artist);
    song.global_data().charter(m_charter);

    if (midi.tracks.empty()) {
        return song;
    }

    song.global_data().tempo_map(
        read_first_midi_track(midi.tracks[0], midi.ticks_per_quarter_note));

    for (const auto& track : midi.tracks) {
        const auto track_name = midi_track_name(track);
        if (!track_name.has_value()) {
            continue;
        }
        if (*track_name == "BEAT") {
            song.global_data().od_beats(od_beats_from_track(track));
        }
        const auto inst = midi_section_instrument(*track_name);
        if (!inst.has_value() || !m_permitted_instruments.contains(*inst)) {
            continue;
        }
        if (SightRead::Detail::is_six_fret_instrument(*inst)) {
            auto tracks
                = ghl_note_tracks_from_midi(track, song.global_data_ptr(),
                                            m_hopo_threshold, m_permit_solos);
            for (auto& [diff, note_track] : tracks) {
                song.add_note_track(*inst, diff, std::move(note_track));
            }
        } else if (*inst == SightRead::Instrument::Drums) {
            auto tracks = drum_note_tracks_from_midi(
                track, song.global_data_ptr(), m_permit_solos);
            for (auto& [diff, note_track] : tracks) {
                song.add_note_track(SightRead::Instrument::Drums, diff,
                                    std::move(note_track));
            }
        } else {
            auto tracks
                = note_tracks_from_midi(track, song.global_data_ptr(),
                                        m_hopo_threshold, m_permit_solos);
            for (auto& [diff, note_track] : tracks) {
                song.add_note_track(*inst, diff, std::move(note_track));
            }
        }
    }

    const auto& od_beats = song.global_data().od_beats();
    if (!od_beats.empty()) {
        auto old_tempo_map = song.global_data().tempo_map();
        SightRead::TempoMap new_tempo_map {old_tempo_map.time_sigs(),
                                           old_tempo_map.bpms(), od_beats,
                                           midi.ticks_per_quarter_note};
        song.global_data().tempo_map(new_tempo_map);
    }

    return song;
}

SightRead::Song MidiParser::parse(std::span<const std::uint8_t> data) const
{
    return from_midi(SightRead::Detail::parse_midi(data));
}