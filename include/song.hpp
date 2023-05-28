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

#ifndef CHOPT_SONG_HPP
#define CHOPT_SONG_HPP

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "chart.hpp"
#include "ini.hpp"
#include "midi.hpp"
#include "songparts.hpp"

class Song {
private:
    std::shared_ptr<SongGlobalData> m_global_data
        = std::make_shared<SongGlobalData>();
    std::map<std::tuple<Instrument, Difficulty>, NoteTrack> m_tracks;

public:
    Song() = default;
    static Song from_midi(const Midi& midi, const IniValues& ini);
    void add_note_track(Instrument instrument, Difficulty difficulty,
                        NoteTrack note_track);
    [[nodiscard]] SongGlobalData& global_data() { return *m_global_data; }
    [[nodiscard]] const SongGlobalData& global_data() const
    {
        return *m_global_data;
    }
    [[nodiscard]] const std::shared_ptr<SongGlobalData>& global_data_ptr()
    {
        return m_global_data;
    }
    [[nodiscard]] std::vector<Instrument> instruments() const;
    [[nodiscard]] std::vector<Difficulty>
    difficulties(Instrument instrument) const;
    [[nodiscard]] const NoteTrack& track(Instrument instrument,
                                         Difficulty difficulty) const;
    [[nodiscard]] std::vector<Tick> unison_phrase_positions() const;
    void speedup(int speed);
};

Song from_chart(const Chart& chart, const IniValues& ini);

#endif
