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

#ifndef CHOPT_SONG_HPP
#define CHOPT_SONG_HPP

#include <algorithm>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "midi.hpp"

enum class Difficulty { Easy, Medium, Hard, Expert };

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

    friend bool operator==(const TimeSignature& lhs, const TimeSignature& rhs)
    {
        return std::tie(lhs.position, lhs.numerator, lhs.denominator)
            == std::tie(rhs.position, rhs.numerator, rhs.denominator);
    }
};

struct BPM {
    int position;
    int bpm;

    friend bool operator==(const BPM& lhs, const BPM& rhs)
    {
        return std::tie(lhs.position, lhs.bpm)
            == std::tie(rhs.position, rhs.bpm);
    }
};

template <typename T> struct Note {
    int position {0};
    int length {0};
    T colour = {};

    friend bool operator==(const Note& lhs, const Note& rhs)
    {
        return std::tie(lhs.position, lhs.length, lhs.colour)
            == std::tie(rhs.position, rhs.length, rhs.colour);
    }
};

struct StarPower {
    int position;
    int length;

    friend bool operator==(const StarPower& lhs, const StarPower& rhs)
    {
        return std::tie(lhs.position, lhs.length)
            == std::tie(rhs.position, rhs.length);
    }
};

struct Solo {
    int start;
    int end;
    int value;

    friend bool operator==(const Solo& lhs, const Solo& rhs)
    {
        return std::tie(lhs.start, lhs.end, lhs.value)
            == std::tie(rhs.start, rhs.end, rhs.value);
    }
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

public:
    NoteTrack() = default;
    NoteTrack(std::vector<Note<T>> notes, std::vector<StarPower> sp_phrases,
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

    friend bool operator==(const NoteTrack& lhs, const NoteTrack& rhs)
    {
        return std::tie(lhs.m_notes, lhs.m_sp_phrases, lhs.m_solos)
            == std::tie(rhs.m_notes, rhs.m_sp_phrases, rhs.m_solos);
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
    std::map<Difficulty, NoteTrack<NoteColour>> m_bass_note_tracks;
    std::map<Difficulty, NoteTrack<NoteColour>> m_guitar_note_tracks;
    std::map<Difficulty, NoteTrack<NoteColour>> m_guitar_coop_note_tracks;
    std::map<Difficulty, NoteTrack<NoteColour>> m_keys_note_tracks;
    std::map<Difficulty, NoteTrack<NoteColour>> m_rhythm_note_tracks;
    std::map<Difficulty, NoteTrack<GHLNoteColour>> m_ghl_guitar_note_tracks;
    std::map<Difficulty, NoteTrack<GHLNoteColour>> m_ghl_bass_note_tracks;
    std::map<Difficulty, NoteTrack<DrumNoteColour>> m_drum_note_tracks;
    Song() = default;

public:
    static Song from_filename(const std::string& filename);
    static Song from_midi(const Midi& midi);
    static Song parse_chart(std::string_view input);
    [[nodiscard]] int resolution() const { return m_resolution; }
    [[nodiscard]] const SongHeader& song_header() const
    {
        return m_song_header;
    }
    [[nodiscard]] const SyncTrack& sync_track() const { return m_sync_track; }
    [[nodiscard]] const NoteTrack<NoteColour>&
    guitar_note_track(Difficulty diff) const
    {
        return m_guitar_note_tracks.at(diff);
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    guitar_coop_note_track(Difficulty diff) const
    {
        return m_guitar_coop_note_tracks.at(diff);
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    bass_note_track(Difficulty diff) const
    {
        return m_bass_note_tracks.at(diff);
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    rhythm_note_track(Difficulty diff) const
    {
        return m_rhythm_note_tracks.at(diff);
    }
    [[nodiscard]] const NoteTrack<NoteColour>&
    keys_note_track(Difficulty diff) const
    {
        return m_keys_note_tracks.at(diff);
    }
    [[nodiscard]] const NoteTrack<GHLNoteColour>&
    ghl_guitar_note_track(Difficulty diff) const
    {
        return m_ghl_guitar_note_tracks.at(diff);
    }
    [[nodiscard]] const NoteTrack<GHLNoteColour>&
    ghl_bass_note_track(Difficulty diff) const
    {
        return m_ghl_bass_note_tracks.at(diff);
    }
    [[nodiscard]] const NoteTrack<DrumNoteColour>&
    drum_note_track(Difficulty diff) const
    {
        return m_drum_note_tracks.at(diff);
    }
};

#endif
