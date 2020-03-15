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

struct BPM {
    uint32_t position;
    uint32_t bpm;

    friend bool operator==(const BPM& lhs, const BPM& rhs)
    {
        return std::tie(lhs.position, lhs.bpm)
            == std::tie(rhs.position, rhs.bpm);
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
    bool is_forced = false;
    bool is_tap = false;

    friend bool operator==(const Note& lhs, const Note& rhs)
    {
        return std::tie(lhs.position, lhs.length, lhs.colour, lhs.is_forced,
                        lhs.is_tap)
            == std::tie(rhs.position, rhs.length, rhs.colour, rhs.is_forced,
                        rhs.is_tap);
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

    friend bool operator==(const NoteTrack& lhs, const NoteTrack& rhs)
    {
        return std::tie(lhs.m_notes, lhs.m_sp_phrases, lhs.m_events)
            == std::tie(rhs.m_notes, rhs.m_sp_phrases, rhs.m_events);
    }
};

class Chart {
private:
    static constexpr float DEFAULT_RESOLUTION = 192.F;
    float m_offset = 0;
    float m_resolution = DEFAULT_RESOLUTION;
    std::vector<TimeSignature> m_time_sigs;
    std::vector<BPM> m_bpms;
    std::vector<Section> m_sections;
    std::map<Difficulty, NoteTrack> m_note_tracks;

    std::string_view read_song_header(std::string_view input);
    std::string_view read_sync_track(std::string_view input);
    std::string_view read_events(std::string_view input);

public:
    explicit Chart(std::string_view input);
    [[nodiscard]] float offset() const { return m_offset; }
    [[nodiscard]] float resolution() const { return m_resolution; }
    [[nodiscard]] const std::vector<TimeSignature>& time_sigs() const
    {
        return m_time_sigs;
    }
    [[nodiscard]] const std::vector<BPM>& bpms() const { return m_bpms; }
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
