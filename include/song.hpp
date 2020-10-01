/*
 * CHOpt - Star Power optimiser for Clone Hero
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

#ifndef CHOPT_SONG_HPP
#define CHOPT_SONG_HPP

#include <algorithm>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "chart.hpp"
#include "ini.hpp"
#include "midi.hpp"

enum class Difficulty { Easy, Medium, Hard, Expert };

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
    Kick
};

struct TimeSignature {
    int position;
    int numerator;
    int denominator;
};

struct BPM {
    int position;
    int bpm;
};

template <typename T> struct Note {
    int position {0};
    int length {0};
    T colour = {};
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

// Invariants:
// notes() will always return a vector of sorted notes.
// notes() will not return a vector with two notes of the same colour with the
// same position.
// sp_phrases() will always return a vector of sorted SP phrases.
// sp_phrases() will only return phrases with a note in their range.
// sp_phrases() will return non-overlapping phrases.
// solos() will always return a vector of sorted solos.
template <typename T> class NoteTrack {
private:
    std::vector<Note<T>> m_notes;
    std::vector<StarPower> m_sp_phrases;
    std::vector<Solo> m_solos;
    int m_resolution;

public:
    NoteTrack(std::vector<Note<T>> notes, std::vector<StarPower> sp_phrases,
              std::vector<Solo> solos, int resolution)
        : m_resolution {resolution}
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
            p->length
                = std::min(p->length, next_phrase->position - p->position);
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

        std::stable_sort(solos.begin(), solos.end(),
                         [](const auto& lhs, const auto& rhs) {
                             return lhs.start < rhs.start;
                         });

        m_solos = std::move(solos);
    }

    [[nodiscard]] const std::vector<Note<T>>& notes() const { return m_notes; }
    [[nodiscard]] const std::vector<StarPower>& sp_phrases() const
    {
        return m_sp_phrases;
    }
    [[nodiscard]] const std::vector<Solo>& solos() const { return m_solos; }
    [[nodiscard]] int base_score() const
    {
        constexpr auto BASE_NOTE_VALUE = 50;
        constexpr auto BASE_SUSTAIN_DENSITY = 25;

        auto base_score = static_cast<int>(BASE_NOTE_VALUE * m_notes.size());
        auto total_ticks = 0;
        auto current_pos = -1;
        auto first_note_length = 0;
        auto current_total_ticks = 0;
        auto is_disjoint_chord = false;
        for (const auto& note : m_notes) {
            if (note.position != current_pos) {
                if (is_disjoint_chord) {
                    total_ticks += current_total_ticks;
                } else {
                    total_ticks += first_note_length;
                }
                first_note_length = note.length;
                current_total_ticks = note.length;
                current_pos = note.position;
                is_disjoint_chord = false;
            } else {
                if (note.length != first_note_length) {
                    is_disjoint_chord = true;
                }
                current_total_ticks += note.length;
            }
        }
        if (is_disjoint_chord) {
            total_ticks += current_total_ticks;
        } else {
            total_ticks += first_note_length;
        }

        base_score += (total_ticks * BASE_SUSTAIN_DENSITY + m_resolution - 1)
            / m_resolution;
        return base_score;
    }
};

// Invariants:
// bpms() are sorted by position.
// bpms() never has two BPMs with the same position.
// bpms() is never empty.
// time_sigs() are sorted by position.
// time_sigs() never has two TimeSignatures with the same position.
// time_sigs() is never empty.
class SyncTrack {
private:
    std::vector<TimeSignature> m_time_sigs;
    std::vector<BPM> m_bpms;

public:
    SyncTrack()
        : SyncTrack({}, {})
    {
    }
    SyncTrack(std::vector<TimeSignature> time_sigs, std::vector<BPM> bpms);
    [[nodiscard]] const std::vector<TimeSignature>& time_sigs() const
    {
        return m_time_sigs;
    }
    [[nodiscard]] const std::vector<BPM>& bpms() const { return m_bpms; }
    // Return the SyncTrack for a speedup of speed% (normal speed is 100).
    [[nodiscard]] SyncTrack speedup(int speed) const;
};

struct SongHeader {
    std::string name {"Unknown Song"};
    std::string artist {"Unknown Artist"};
    std::string charter {"Unknown Charter"};
};

// Invariants:
// resolution() > 0.
class Song {
private:
    static constexpr int DEFAULT_RESOLUTION = 192;

    int m_resolution = DEFAULT_RESOLUTION;
    SongHeader m_song_header;
    SyncTrack m_sync_track;
    std::map<std::tuple<Instrument, Difficulty>, NoteTrack<NoteColour>>
        m_five_fret_tracks;
    std::map<std::tuple<Instrument, Difficulty>, NoteTrack<GHLNoteColour>>
        m_six_fret_tracks;
    std::map<Difficulty, NoteTrack<DrumNoteColour>> m_drum_note_tracks;
    Song() = default;

public:
    static Song from_filename(const std::string& filename);
    static Song from_chart(const Chart& chart, const IniValues& ini);
    static Song from_midi(const Midi& midi, const IniValues& ini);
    [[nodiscard]] int resolution() const { return m_resolution; }
    [[nodiscard]] const SongHeader& song_header() const
    {
        return m_song_header;
    }
    [[nodiscard]] const SyncTrack& sync_track() const { return m_sync_track; }
    [[nodiscard]] std::vector<Instrument> instruments() const;
    [[nodiscard]] std::vector<Difficulty>
    difficulties(Instrument instrument) const;
    [[nodiscard]] const NoteTrack<NoteColour>&
    guitar_note_track(Difficulty diff) const
    {
        return m_five_fret_tracks.at({Instrument::Guitar, diff});
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    guitar_coop_note_track(Difficulty diff) const
    {
        return m_five_fret_tracks.at({Instrument::GuitarCoop, diff});
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    bass_note_track(Difficulty diff) const
    {
        return m_five_fret_tracks.at({Instrument::Bass, diff});
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    rhythm_note_track(Difficulty diff) const
    {
        return m_five_fret_tracks.at({Instrument::Rhythm, diff});
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    keys_note_track(Difficulty diff) const
    {
        return m_five_fret_tracks.at({Instrument::Keys, diff});
    }
    [[nodiscard]] const NoteTrack<GHLNoteColour>&
    ghl_guitar_note_track(Difficulty diff) const
    {
        return m_six_fret_tracks.at({Instrument::GHLGuitar, diff});
    }
    [[nodiscard]] const NoteTrack<GHLNoteColour>&
    ghl_bass_note_track(Difficulty diff) const
    {
        return m_six_fret_tracks.at({Instrument::GHLBass, diff});
    }
    [[nodiscard]] const NoteTrack<DrumNoteColour>&
    drum_note_track(Difficulty diff) const
    {
        return m_drum_note_tracks.at(diff);
    }
};

#endif
