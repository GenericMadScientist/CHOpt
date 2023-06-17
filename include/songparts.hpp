/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023 Raymond Wright
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

#ifndef CHOPT_SONGPARTS_HPP
#define CHOPT_SONGPARTS_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "drumsettings.hpp"
#include "tempomap.hpp"
#include "time.hpp"

enum class Difficulty { Easy = 0, Medium = 1, Hard = 2, Expert = 3 };

enum class Instrument {
    Guitar,
    GuitarCoop,
    Bass,
    Rhythm,
    Keys,
    GHLGuitar,
    GHLBass,
    Drums
};

std::set<Instrument> all_instruments();

enum class TrackType { FiveFret, SixFret, Drums };

enum NoteFlags : std::uint32_t {
    FLAGS_NONE = 0,
    FLAGS_CYMBAL = 1U << 0,
    FLAGS_GHOST = 1U << 1,
    FLAGS_ACCENT = 1U << 2,
    FLAGS_HOPO = 1U << 3,
    FLAGS_TAP = 1U << 4,
    FLAGS_DRUMS = 1U << 29,
    FLAGS_SIX_FRET_GUITAR = 1U << 30,
    FLAGS_FIVE_FRET_GUITAR = 1U << 31
};

enum FiveFretNotes {
    FIVE_FRET_GREEN = 0,
    FIVE_FRET_RED = 1,
    FIVE_FRET_YELLOW = 2,
    FIVE_FRET_BLUE = 3,
    FIVE_FRET_ORANGE = 4,
    FIVE_FRET_OPEN = 5
};

enum SixFretNotes {
    SIX_FRET_WHITE_LOW = 0,
    SIX_FRET_WHITE_MID = 1,
    SIX_FRET_WHITE_HIGH = 2,
    SIX_FRET_BLACK_LOW = 3,
    SIX_FRET_BLACK_MID = 4,
    SIX_FRET_BLACK_HIGH = 5,
    SIX_FRET_OPEN = 6
};

enum DrumNotes {
    DRUM_RED = 0,
    DRUM_YELLOW = 1,
    DRUM_BLUE = 2,
    DRUM_GREEN = 3,
    DRUM_KICK = 4,
    DRUM_DOUBLE_KICK = 5
};

struct Note {
private:
    int open_index() const;

public:
    Tick position {0};
    std::array<Tick, 7> lengths {{Tick {-1}, Tick {-1}, Tick {-1}, Tick {-1},
                                  Tick {-1}, Tick {-1}, Tick {-1}}};
    NoteFlags flags {0};

    [[nodiscard]] int colours() const;
    void merge_non_opens_into_open();
    void disable_dynamics();
    [[nodiscard]] bool is_kick_note() const;
    [[nodiscard]] bool is_skipped_kick(const DrumSettings& settings) const;
};

struct StarPower {
    Tick position;
    Tick length;
};

struct Solo {
    Tick start;
    Tick end;
    int value;
};

struct DrumFill {
    Tick position;
    Tick length;
};

struct DiscoFlip {
    Tick position;
    Tick length;
};

struct BigRockEnding {
    Tick start;
    Tick end;
};

// Invariants:
// resolution() > 0.
class SongGlobalData {
private:
    static constexpr int DEFAULT_RESOLUTION = 192;

    bool m_is_from_midi = false;
    int m_resolution = DEFAULT_RESOLUTION;
    std::string m_name;
    std::string m_artist;
    std::string m_charter;
    TempoMap m_tempo_map;
    std::vector<Tick> m_od_beats;

public:
    SongGlobalData() = default;

    [[nodiscard]] bool is_from_midi() const { return m_is_from_midi; }
    [[nodiscard]] int resolution() const { return m_resolution; }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::string& artist() const { return m_artist; }
    [[nodiscard]] const std::string& charter() const { return m_charter; }
    [[nodiscard]] TempoMap& tempo_map() { return m_tempo_map; }
    [[nodiscard]] const TempoMap& tempo_map() const { return m_tempo_map; }
    [[nodiscard]] const std::vector<Tick>& od_beats() const
    {
        return m_od_beats;
    }

    void is_from_midi(bool value) { m_is_from_midi = value; }
    void resolution(int value)
    {
        if (value <= 0) {
            throw ParseError("Resolution non-positive");
        }
        m_resolution = value;
    }
    void name(std::string value) { m_name = std::move(value); }
    void artist(std::string value) { m_artist = std::move(value); }
    void charter(std::string value) { m_charter = std::move(value); }
    void tempo_map(TempoMap value) { m_tempo_map = std::move(value); }
    void od_beats(std::vector<Tick> value) { m_od_beats = std::move(value); }
};

// Invariants:
// notes() will always return a vector of sorted notes.
// notes() will not return a vector with two notes of the same colour with the
// same position.
// resolution() will be greater than zero.
// sp_phrases() will always return a vector of sorted SP phrases.
// sp_phrases() will only return phrases with a note in their range.
// sp_phrases() will return non-overlapping phrases.
// solos() will always return a vector of sorted solos.
class NoteTrack {
private:
    std::vector<Note> m_notes;
    std::vector<StarPower> m_sp_phrases;
    std::vector<Solo> m_solos;
    std::vector<DrumFill> m_drum_fills;
    std::vector<DiscoFlip> m_disco_flips;
    std::optional<BigRockEnding> m_bre;
    TrackType m_track_type;
    std::shared_ptr<SongGlobalData> m_global_data;
    int m_base_score_ticks;

    void compute_base_score_ticks();
    void merge_same_time_notes();
    void add_hopos(Tick max_hopo_gap);

public:
    NoteTrack(std::vector<Note> notes, const std::vector<StarPower>& sp_phrases,
              std::vector<Solo> solos, TrackType track_type,
              std::shared_ptr<SongGlobalData> global_data,
              Tick max_hopo_gap = Tick {65});
    void generate_drum_fills(const TempoMap& tempo_map);
    void disable_dynamics();
    [[nodiscard]] const std::vector<Note>& notes() const { return m_notes; }
    [[nodiscard]] const std::vector<StarPower>& sp_phrases() const
    {
        return m_sp_phrases;
    }
    [[nodiscard]] std::vector<Solo>
    solos(const DrumSettings& drum_settings) const;

    [[nodiscard]] const std::vector<DrumFill>& drum_fills() const
    {
        return m_drum_fills;
    }
    void drum_fills(std::vector<DrumFill> drum_fills)
    {
        m_drum_fills = std::move(drum_fills);
    }

    [[nodiscard]] const std::vector<DiscoFlip>& disco_flips() const
    {
        return m_disco_flips;
    }
    void disco_flips(std::vector<DiscoFlip> disco_flips)
    {
        m_disco_flips = std::move(disco_flips);
    }

    [[nodiscard]] std::optional<BigRockEnding> bre() const { return m_bre; }
    void bre(std::optional<BigRockEnding> bre) { m_bre = std::move(bre); }

    [[nodiscard]] TrackType track_type() const { return m_track_type; }
    [[nodiscard]] const SongGlobalData& global_data() const
    {
        return *m_global_data;
    }
    [[nodiscard]] int base_score(DrumSettings drum_settings
                                 = DrumSettings::default_settings()) const;
    [[nodiscard]] NoteTrack trim_sustains() const;
    [[nodiscard]] NoteTrack snap_chords(Tick snap_gap) const;
};

#endif
