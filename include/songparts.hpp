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
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "settings.hpp"
#include "timeconverter.hpp"

enum class NoteColour { Green, Red, Yellow, Blue, Orange, Open };

enum class GHLNoteColour {
    WhiteLow,
    WhiteMid,
    WhiteHigh,
    BlackLow,
    BlackMid,
    BlackHigh,
    Open
};

enum class DrumNoteColour {
    Red,
    Yellow,
    Blue,
    Green,
    YellowCymbal,
    BlueCymbal,
    GreenCymbal,
    RedGhost,
    YellowGhost,
    BlueGhost,
    GreenGhost,
    YellowCymbalGhost,
    BlueCymbalGhost,
    GreenCymbalGhost,
    RedAccent,
    YellowAccent,
    BlueAccent,
    GreenAccent,
    YellowCymbalAccent,
    BlueCymbalAccent,
    GreenCymbalAccent,
    Kick,
    DoubleKick
};

constexpr DrumNoteColour strip_dynamics(DrumNoteColour colour)
{
    switch (colour) {
    case DrumNoteColour::Red:
    case DrumNoteColour::RedGhost:
    case DrumNoteColour::RedAccent:
        return DrumNoteColour::Red;
    case DrumNoteColour::Yellow:
    case DrumNoteColour::YellowGhost:
    case DrumNoteColour::YellowAccent:
        return DrumNoteColour::Yellow;
    case DrumNoteColour::Blue:
    case DrumNoteColour::BlueGhost:
    case DrumNoteColour::BlueAccent:
        return DrumNoteColour::Blue;
    case DrumNoteColour::Green:
    case DrumNoteColour::GreenGhost:
    case DrumNoteColour::GreenAccent:
        return DrumNoteColour::Green;
    case DrumNoteColour::YellowCymbal:
    case DrumNoteColour::YellowCymbalGhost:
    case DrumNoteColour::YellowCymbalAccent:
        return DrumNoteColour::YellowCymbal;
    case DrumNoteColour::BlueCymbal:
    case DrumNoteColour::BlueCymbalGhost:
    case DrumNoteColour::BlueCymbalAccent:
        return DrumNoteColour::BlueCymbal;
    case DrumNoteColour::GreenCymbal:
    case DrumNoteColour::GreenCymbalGhost:
    case DrumNoteColour::GreenCymbalAccent:
        return DrumNoteColour::GreenCymbal;
    case DrumNoteColour::Kick:
        return DrumNoteColour::Kick;
    case DrumNoteColour::DoubleKick:
        return DrumNoteColour::DoubleKick;
    }

    throw std::invalid_argument("Invalid DrumNoteColour");
}

enum class TrackType { FiveFret, SixFret, Drums };

enum NoteFlags : std::uint32_t {
    FLAGS_NONE = 0,
    FLAGS_CYMBAL = 1U << 0,
    FLAGS_GHOST = 1U << 1,
    FLAGS_ACCENT = 1U << 2,
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
    int position {0};
    std::array<int, 7> lengths {{-1, -1, -1, -1, -1, -1, -1}};
    NoteFlags flags {0};

    [[nodiscard]] int colours() const;
    void merge_non_opens_into_open();
    void disable_dynamics();
    [[nodiscard]] bool is_kick_note() const;
    [[nodiscard]] bool is_skipped_kick(const DrumSettings& settings) const;
};

struct StarPower {
    int position;
    int length;
};

struct Solo {
    int start;
    int end;
    int value;
};

struct DrumFill {
    int position;
    int length;
};

struct DiscoFlip {
    int position;
    int length;
};

struct BigRockEnding {
    int start;
    int end;
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
    int m_resolution;
    int m_base_score_ticks;

    void compute_base_score_ticks();
    void merge_same_time_notes();

public:
    NoteTrack(std::vector<Note> notes, const std::vector<StarPower>& sp_phrases,
              std::vector<Solo> solos, std::vector<DrumFill> drum_fills,
              std::vector<DiscoFlip> disco_flips,
              std::optional<BigRockEnding> bre, TrackType track_type,
              int resolution);
    void generate_drum_fills(const TimeConverter& converter);
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
    [[nodiscard]] const std::vector<DiscoFlip>& disco_flips() const
    {
        return m_disco_flips;
    }
    [[nodiscard]] std::optional<BigRockEnding> bre() const { return m_bre; }
    [[nodiscard]] TrackType track_type() const { return m_track_type; }
    [[nodiscard]] int resolution() const { return m_resolution; }
    [[nodiscard]] int base_score(DrumSettings drum_settings
                                 = DrumSettings::default_settings()) const;
    [[nodiscard]] NoteTrack trim_sustains() const;
    [[nodiscard]] NoteTrack snap_chords(int snap_gap) const;
};

#endif
