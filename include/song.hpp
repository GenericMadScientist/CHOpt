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
#include "timeconverter.hpp"

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
    SyncTrack m_sync_track;

public:
    SongGlobalData() = default;

    [[nodiscard]] bool is_from_midi() const { return m_is_from_midi; }
    [[nodiscard]] int resolution() const { return m_resolution; }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::string& artist() const { return m_artist; }
    [[nodiscard]] const std::string& charter() const { return m_charter; }
    [[nodiscard]] const SyncTrack& sync_track() const { return m_sync_track; }

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
    void sync_track(SyncTrack value) { m_sync_track = std::move(value); }
};

// Invariants:
// resolution() > 0.
class Song {
private:
    SongGlobalData m_global_data;
    std::vector<int> m_od_beats;
    std::map<std::tuple<Instrument, Difficulty>, NoteTrack> m_tracks;
    Song() = default;
    void append_instrument_track(Instrument inst, Difficulty diff,
                                 const ChartSection& section);

public:
    static Song from_filename(const std::string& filename);
    static Song from_chart(const Chart& chart, const IniValues& ini);
    static Song from_midi(const Midi& midi, const IniValues& ini);
    [[nodiscard]] const SongGlobalData& global_data() const
    {
        return m_global_data;
    }
    [[nodiscard]] const std::vector<int> od_beats() const { return m_od_beats; }
    [[nodiscard]] std::vector<Instrument> instruments() const;
    [[nodiscard]] std::vector<Difficulty>
    difficulties(Instrument instrument) const;
    [[nodiscard]] const NoteTrack& track(Instrument instrument,
                                         Difficulty difficulty) const;
    [[nodiscard]] std::vector<int> unison_phrase_positions() const;
};

#endif
