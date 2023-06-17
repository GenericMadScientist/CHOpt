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

#include <climits>
#include <set>

#include "chartparser.hpp"
#include "parserutil.hpp"

namespace {
std::string get_with_default(const std::map<std::string, std::string>& map,
                             const std::string& key, std::string default_value)
{
    const auto iter = map.find(key);
    if (iter == map.end()) {
        return default_value;
    }
    return iter->second;
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
    return {std::move(tses), std::move(bpms), {}, resolution};
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
            return header.starts_with(std::get<0>(pair));
        });
    if (diff_iter == DIFFICULTIES.cend()) {
        return std::nullopt;
    }
    auto inst_iter = std::find_if( // NOLINT
        INSTRUMENTS.cbegin(), INSTRUMENTS.cend(),
        [&](const auto& pair) { return header.ends_with(std::get<0>(pair)); });
    if (inst_iter == INSTRUMENTS.cend()) {
        return std::nullopt;
    }
    return std::tuple {std::get<1>(*diff_iter), std::get<1>(*inst_iter)};
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
        if (green_positions.contains(Tick {note_event.position})) {
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
        if (!deletion_spots.contains(i)) {
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
        if (accent_events.contains({note.position, lane})) {
            note.flags = static_cast<NoteFlags>(note.flags | FLAGS_ACCENT);
        } else if (ghost_events.contains({note.position, lane})) {
            note.flags = static_cast<NoteFlags>(note.flags | FLAGS_GHOST);
        }
    }
    return notes;
}

class ForcingEvents {
private:
    std::set<int> m_forcing_positions;
    std::set<int> m_tap_positions;

    static bool is_forcing_key(int fret_type, TrackType track_type)
    {
        constexpr int HOPO_FORCE_KEY = 5;

        switch (track_type) {
        case TrackType::FiveFret:
        case TrackType::SixFret:
            return fret_type == HOPO_FORCE_KEY;
        default:
            return false;
        }
    }

    static bool is_tap_key(int fret_type, TrackType track_type)
    {
        constexpr int TAP_FORCE_KEY = 6;

        switch (track_type) {
        case TrackType::FiveFret:
        case TrackType::SixFret:
            return fret_type == TAP_FORCE_KEY;
        default:
            return false;
        }
    }

public:
    void apply_forcing(std::vector<Note>& notes) const
    {
        for (auto& note : notes) {
            if (m_tap_positions.contains(note.position.value())) {
                note.flags = static_cast<NoteFlags>(note.flags | FLAGS_TAP);
            } else if (m_forcing_positions.contains(note.position.value())) {
                note.flags
                    = static_cast<NoteFlags>(note.flags | FLAGS_FORCE_FLIP);
            }
        }
    }

    void add_force_event(const NoteEvent& event, TrackType track_type)
    {
        if (is_forcing_key(event.fret, track_type)) {
            m_forcing_positions.insert(event.position);
        } else if (is_tap_key(event.fret, track_type)) {
            m_tap_positions.insert(event.position);
        }
    }
};

std::vector<Note> apply_drum_events(std::vector<Note> notes,
                                    const std::vector<NoteEvent>& note_events,
                                    TrackType track_type)
{
    if (track_type != TrackType::Drums) {
        return notes;
    }
    notes = add_fifth_lane_greens(std::move(notes), note_events);
    notes = apply_cymbal_events(notes);
    return apply_dynamics_events(notes, note_events);
}

NoteTrack note_track_from_section(const ChartSection& section,
                                  std::shared_ptr<SongGlobalData> global_data,
                                  TrackType track_type, bool permit_solos,
                                  Tick max_hopo_gap)
{
    constexpr int DISCO_FLIP_START_SIZE = 13;
    constexpr int DISCO_FLIP_END_SIZE = 12;
    constexpr int DRUM_FILL_KEY = 64;
    constexpr std::array<std::uint8_t, 4> MIX {{'m', 'i', 'x', '_'}};
    constexpr std::array<std::uint8_t, 6> DRUMS {
        {'_', 'd', 'r', 'u', 'm', 's'}};

    ForcingEvents forcing_events;
    std::vector<Note> notes;
    for (const auto& note_event : section.note_events) {
        const auto note
            = note_from_note_colour(note_event.position, note_event.length,
                                    note_event.fret, track_type);
        if (note.has_value()) {
            notes.push_back(*note);
        } else {
            forcing_events.add_force_event(note_event, track_type);
        }
    }
    forcing_events.apply_forcing(notes);
    notes = apply_drum_events(notes, section.note_events, track_type);

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
    if (!permit_solos) {
        solos.clear();
    }
    std::sort(disco_flip_on_events.begin(), disco_flip_on_events.end());
    std::sort(disco_flip_off_events.begin(), disco_flip_off_events.end());
    std::vector<DiscoFlip> disco_flips;
    for (auto [start, end] :
         combine_solo_events(disco_flip_on_events, disco_flip_off_events)) {
        disco_flips.push_back({start, end - start});
    }

    NoteTrack note_track {std::move(notes), sp, track_type,
                          std::move(global_data), max_hopo_gap};
    note_track.solos(std::move(solos));
    note_track.drum_fills(std::move(fills));
    note_track.disco_flips(std::move(disco_flips));
    return note_track;
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
}

ChartParser::ChartParser(const IniValues& ini)
    : m_song_name {ini.name}
    , m_artist {ini.artist}
    , m_charter {ini.charter}
    , m_hopo_threshold {HopoThresholdType::Resolution, Tick {0}}
    , m_permitted_instruments {all_instruments()}
    , m_permit_solos {true}
{
}

ChartParser& ChartParser::hopo_threshold(HopoThreshold hopo_threshold)
{
    m_hopo_threshold = hopo_threshold;
    return *this;
}

ChartParser&
ChartParser::permit_instruments(std::set<Instrument> permitted_instruments)
{
    m_permitted_instruments = std::move(permitted_instruments);
    return *this;
}

ChartParser& ChartParser::parse_solos(bool permit_solos)
{
    m_permit_solos = permit_solos;
    return *this;
}

Tick ChartParser::max_hopo_gap(int resolution) const
{
    constexpr int DEFAULT_HOPO_GAP = 65;
    constexpr int DEFAULT_RESOLUTION = 192;

    switch (m_hopo_threshold.threshold_type) {
    case HopoThresholdType::HopoFrequency:
        return m_hopo_threshold.hopo_frequency;
    case HopoThresholdType::EighthNote:
        return Tick {(resolution + 3) / 2};
    default:
        return Tick {(DEFAULT_HOPO_GAP * resolution) / DEFAULT_RESOLUTION};
    }
}

Song ChartParser::parse(std::string_view data) const
{
    return from_chart(parse_chart(data));
}

Song ChartParser::from_chart(const Chart& chart) const
{
    Song song;

    song.global_data().is_from_midi(false);
    song.global_data().name(m_song_name);
    song.global_data().artist(m_artist);
    song.global_data().charter(m_charter);

    for (const auto& section : chart.sections) {
        if (section.name == "Song") {
            try {
                const auto resolution = std::stoi(get_with_default(
                    section.key_value_pairs, "Resolution", "192"));
                song.global_data().resolution(resolution);
            } catch (const std::invalid_argument&) {
                // CH just ignores this kind of parsing mistake.
                // TODO: Use from_chars instead to avoid having to use
                // exceptions as control flow.
            }
        } else if (section.name == "SyncTrack") {
            song.global_data().tempo_map(tempo_map_from_section(
                section, song.global_data().resolution()));
        } else {
            auto pair = diff_inst_from_header(section.name);
            if (!pair.has_value()) {
                continue;
            }
            auto [diff, inst] = *pair;
            if (!m_permitted_instruments.contains(inst)) {
                continue;
            }
            const auto resolution = song.global_data().resolution();
            auto note_track = note_track_from_section(
                section, song.global_data_ptr(),
                track_type_from_instrument(inst), m_permit_solos,
                max_hopo_gap(resolution));
            song.add_note_track(inst, diff, std::move(note_track));
        }
    }

    if (song.instruments().empty()) {
        throw ParseError("Chart has no notes");
    }

    return song;
}
