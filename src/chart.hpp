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

#ifndef CHOPT_CHART_HPP
#define CHOPT_CHART_HPP

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

enum class Difficulty { Easy, Medium, Hard, Expert };

enum class NoteColour { Green, Red, Yellow, Blue, Orange, Open };

struct TimeSignature {
    uint32_t position;
    uint32_t numerator;
    uint32_t denominator;

    friend bool operator==(const TimeSignature& lhs, const TimeSignature& rhs)
    {
        return std::tie(lhs.position, lhs.numerator, lhs.denominator)
            == std::tie(rhs.position, rhs.numerator, rhs.denominator);
    }
};

struct Section {
    uint32_t position;
    std::string name;

    friend bool operator==(const Section& lhs, const Section& rhs)
    {
        return std::tie(lhs.position, lhs.name)
            == std::tie(rhs.position, rhs.name);
    }
};

struct Note {
    uint32_t position = 0;
    uint32_t length = 0;
    NoteColour colour = NoteColour::Green;

    friend bool operator==(const Note& lhs, const Note& rhs)
    {
        return std::tie(lhs.position, lhs.length, lhs.colour)
            == std::tie(rhs.position, rhs.length, rhs.colour);
    }
};

struct StarPower {
    uint32_t position;
    uint32_t length;

    friend bool operator==(const StarPower& lhs, const StarPower& rhs)
    {
        return std::tie(lhs.position, lhs.length)
            == std::tie(rhs.position, rhs.length);
    }
};

struct ChartEvent {
    uint32_t position;
    std::string name;

    friend bool operator==(const ChartEvent& lhs, const ChartEvent& rhs)
    {
        return std::tie(lhs.position, lhs.name)
            == std::tie(rhs.position, rhs.name);
    }
};

// Invariants:
// notes() will always return a vector of sorted notes.
// notes() will not return a vector with two notes of the same colour with the
// same position.
// sp_phrases() will always return a vector of sorted SP phrases.
// sp_phrases() will only return phrases with a note in their range.
// sp_phrases() will return non-overlapping phrases.
class NoteTrack {
private:
    std::vector<Note> m_notes;
    std::vector<StarPower> m_sp_phrases;
    std::vector<ChartEvent> m_events;

public:
    NoteTrack() = default;
    NoteTrack(std::vector<Note> notes, std::vector<StarPower> sp_phrases,
              std::vector<ChartEvent> events);

    [[nodiscard]] const std::vector<Note>& notes() const { return m_notes; }
    [[nodiscard]] const std::vector<StarPower>& sp_phrases() const
    {
        return m_sp_phrases;
    }

    friend bool operator==(const NoteTrack& lhs, const NoteTrack& rhs)
    {
        return std::tie(lhs.m_notes, lhs.m_sp_phrases, lhs.m_events)
            == std::tie(rhs.m_notes, rhs.m_sp_phrases, rhs.m_events);
    }
};

// Invariants:
// time_sigs() are sorted by position.
// time_sigs() never has two TimeSignatures with the same poisition.
// time_sigs() is never empty.
class SyncTrack {
private:
    std::vector<TimeSignature> m_time_sigs;

public:
    explicit SyncTrack(std::vector<TimeSignature> time_sigs = {});
    [[nodiscard]] const std::vector<TimeSignature>& time_sigs() const
    {
        return m_time_sigs;
    }
};

// Invariants:
// resolution() > 0.
class Chart {
private:
    static constexpr int32_t DEFAULT_RESOLUTION = 192;

    int32_t m_resolution = DEFAULT_RESOLUTION;
    SyncTrack m_sync_track;
    std::vector<Section> m_sections;
    std::map<Difficulty, NoteTrack> m_note_tracks;
    Chart() = default;

public:
    static Chart parse_chart(std::string_view input);
    [[nodiscard]] int32_t resolution() const { return m_resolution; }
    [[nodiscard]] const SyncTrack& sync_track() const { return m_sync_track; }
    [[nodiscard]] const std::vector<Section>& sections() const
    {
        return m_sections;
    }
    [[nodiscard]] const NoteTrack& note_track(Difficulty diff) const
    {
        return m_note_tracks.at(diff);
    }
};

#endif
