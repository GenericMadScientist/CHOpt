/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include "song.hpp"

namespace {
bool is_six_fret_instrument(Instrument instrument)
{
    return instrument == Instrument::GHLGuitar
        || instrument == Instrument::GHLBass;
}
}

void Song::add_note_track(Instrument instrument, Difficulty difficulty,
                          NoteTrack note_track)
{
    if (!note_track.notes().empty()) {
        m_tracks.insert({{instrument, difficulty}, std::move(note_track)});
    }
}

std::vector<Instrument> Song::instruments() const
{
    std::set<Instrument> instrument_set;
    for (const auto& [key, val] : m_tracks) {
        instrument_set.insert(std::get<0>(key));
    }

    std::vector<Instrument> instruments {instrument_set.cbegin(),
                                         instrument_set.cend()};
    std::sort(instruments.begin(), instruments.end());
    return instruments;
}

std::vector<Difficulty> Song::difficulties(Instrument instrument) const
{
    std::vector<Difficulty> difficulties;
    for (const auto& [key, val] : m_tracks) {
        if (std::get<0>(key) == instrument) {
            difficulties.push_back(std::get<1>(key));
        }
    }
    std::sort(difficulties.begin(), difficulties.end());
    return difficulties;
}

const NoteTrack& Song::track(Instrument instrument, Difficulty difficulty) const
{
    const auto insts = instruments();
    if (std::find(insts.cbegin(), insts.cend(), instrument) == insts.cend()) {
        throw std::invalid_argument("Chosen instrument not present in song");
    }
    const auto diffs = difficulties(instrument);
    if (std::find(diffs.cbegin(), diffs.cend(), difficulty) == diffs.cend()) {
        throw std::invalid_argument(
            "Difficulty not available for chosen instrument");
    }
    return m_tracks.at({instrument, difficulty});
}

std::vector<Tick> Song::unison_phrase_positions() const
{
    std::map<Tick, std::set<Instrument>> phrase_by_instrument;
    for (const auto& [key, value] : m_tracks) {
        const auto instrument = std::get<0>(key);
        if (is_six_fret_instrument(instrument)) {
            continue;
        }
        for (const auto& phrase : value.sp_phrases()) {
            phrase_by_instrument[phrase.position].insert(instrument);
        }
    }

    std::vector<Tick> unison_starts;
    for (const auto& [key, value] : phrase_by_instrument) {
        if (value.size() > 1) {
            unison_starts.push_back(key);
        }
    }
    return unison_starts;
}

void Song::speedup(int speed)
{
    constexpr int DEFAULT_SPEED = 100;

    if (speed == DEFAULT_SPEED) {
        return;
    }
    if (speed <= 0) {
        throw std::invalid_argument("Speed must be positive");
    }

    m_global_data->name(m_global_data->name() + " (" + std::to_string(speed)
                        + "%)");
    m_global_data->tempo_map(m_global_data->tempo_map().speedup(speed));
}
