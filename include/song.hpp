/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021 Raymond Wright
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
#include "songparts.hpp"

// Invariants:
// resolution() > 0.
class SongGlobalData {
private:
    static constexpr int DEFAULT_RESOLUTION = 192;

    int m_resolution;

public:
    explicit SongGlobalData(int resolution = DEFAULT_RESOLUTION);

    int resolution() const { return m_resolution; }
};

// Invariants:
// resolution() > 0.
class Song {
private:
    bool m_is_from_midi = false;
    SongGlobalData m_global_data;
    std::string m_name;
    std::string m_artist;
    std::string m_charter;
    SyncTrack m_sync_track;
    std::vector<int> m_od_beats;
    std::map<std::tuple<Instrument, Difficulty>, NoteTrack> m_tracks;
    Song() = default;
    void append_instrument_track(Instrument inst, Difficulty diff,
                                 const ChartSection& section);

public:
    static Song from_filename(const std::string& filename);
    static Song from_chart(const Chart& chart, const IniValues& ini);
    static Song from_midi(const Midi& midi, const IniValues& ini);
    [[nodiscard]] bool is_from_midi() const { return m_is_from_midi; }
    [[nodiscard]] const SongGlobalData& global_data() const
    {
        return m_global_data;
    }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::string& artist() const { return m_artist; }
    [[nodiscard]] const std::string& charter() const { return m_charter; }
    [[nodiscard]] const SyncTrack& sync_track() const { return m_sync_track; }
    [[nodiscard]] const std::vector<int> od_beats() const { return m_od_beats; }
    [[nodiscard]] std::vector<Instrument> instruments() const;
    [[nodiscard]] std::vector<Difficulty>
    difficulties(Instrument instrument) const;
    [[nodiscard]] const NoteTrack& track(Instrument instrument,
                                         Difficulty difficulty) const;
    [[nodiscard]] std::vector<int> unison_phrase_positions() const;
};

#endif
