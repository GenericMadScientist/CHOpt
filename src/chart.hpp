/*
 *  chopt - Star Power optimiser for Clone Hero
 *  Copyright (C) 2020  Raymond Wright
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
};

struct Note {
    uint32_t position = 0;
    uint32_t length = 0;
    NoteColour colour = NoteColour::Green;
    bool is_forced = false;
    bool is_tap = false;
};

struct StarPower {
    uint32_t position;
    uint32_t length;
};

struct ChartEvent {
    uint32_t position;
    std::string name;
};

struct NoteTrack {
    std::vector<Note> notes;
    std::vector<StarPower> sp_phrases;
    std::vector<ChartEvent> events;
};

class Chart {
private:
    static constexpr float DEFAULT_RESOLUTION = 192;
    float offset = 0;
    float resolution = DEFAULT_RESOLUTION;
    std::vector<TimeSignature> time_sigs;
    std::vector<BPM> bpms;
    std::vector<Section> sections;
    std::map<Difficulty, NoteTrack> note_tracks;

    std::string_view read_song_header(std::string_view input);
    std::string_view read_sync_track(std::string_view input);
    std::string_view read_events(std::string_view input);
    std::string_view read_single_track(std::string_view input, Difficulty diff);

public:
    explicit Chart(std::string_view input);
    [[nodiscard]] float get_offset() const { return offset; }
    [[nodiscard]] float get_resolution() const { return resolution; }
    [[nodiscard]] const std::vector<TimeSignature>& get_time_sigs() const
    {
        return time_sigs;
    }
    [[nodiscard]] const std::vector<BPM>& get_bpms() const { return bpms; }
};

#endif
