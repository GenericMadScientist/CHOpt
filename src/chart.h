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

#ifndef CHOPT_CHART_H
#define CHOPT_CHART_H

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

enum class Difficulty { Easy, Medium, Hard, Expert };

enum class NoteColour { Green, Red, Yellow, Blue, Orange, Open };

struct TimeSignature {
    uint32_t position;
    uint32_t numerator;
    uint32_t denominator;
};

struct BPM {
    uint32_t position;
    uint32_t bpm;
};

struct Section {
    uint32_t position;
    std::string name;
};

struct Note {
    uint32_t position;
    uint32_t length;
    NoteColour colour;
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
    float offset = 0;
    float resolution = 192;
    std::vector<TimeSignature> time_sigs;
    std::vector<BPM> bpms;
    std::vector<Section> sections;
    std::map<Difficulty, NoteTrack> note_tracks;

    std::string_view read_song_header(std::string_view input);
    std::string_view read_sync_track(std::string_view input);
    std::string_view read_events(std::string_view input);
    std::string_view read_single_track(std::string_view input, Difficulty diff);
    std::string_view skip_unrecognised_section(std::string_view input) const;

public:
    Chart(std::string_view input);
};

#endif
