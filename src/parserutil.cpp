/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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

#include <set>

#include "parserutil.hpp"

bool is_six_fret_instrument(Instrument instrument)
{
    return instrument == Instrument::GHLGuitar
        || instrument == Instrument::GHLBass;
}

std::vector<std::tuple<Tick, Tick>>
combine_solo_events(const std::vector<int>& on_events,
                    const std::vector<int>& off_events)
{
    std::vector<std::tuple<Tick, Tick>> ranges;

    auto on_iter = on_events.cbegin();
    auto off_iter = off_events.cbegin();

    while (on_iter < on_events.cend() && off_iter < off_events.cend()) {
        if (*on_iter >= *off_iter) {
            ++off_iter;
            continue;
        }
        ranges.emplace_back(*on_iter, *off_iter);
        while (on_iter < on_events.cend() && *on_iter < *off_iter) {
            ++on_iter;
        }
    }

    return ranges;
}

std::vector<Solo> form_solo_vector(const std::vector<int>& solo_on_events,
                                   const std::vector<int>& solo_off_events,
                                   const std::vector<Note>& notes,
                                   TrackType track_type, bool is_midi)
{
    constexpr int SOLO_NOTE_VALUE = 100;

    std::vector<Solo> solos;

    for (auto [start, end] :
         combine_solo_events(solo_on_events, solo_off_events)) {
        std::set<Tick> positions_in_solo;
        auto note_count = 0;
        for (const auto& note : notes) {
            if ((note.position >= start && note.position < end)
                || (note.position == end && !is_midi)) {
                positions_in_solo.insert(note.position);
                ++note_count;
            }
        }
        if (positions_in_solo.empty()) {
            continue;
        }
        if (track_type != TrackType::Drums) {
            note_count = static_cast<int>(positions_in_solo.size());
        }
        solos.push_back({start, end, SOLO_NOTE_VALUE * note_count});
    }

    return solos;
}
