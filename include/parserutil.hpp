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

#ifndef CHOPT_PARSERUTIL_HPP
#define CHOPT_PARSERUTIL_HPP

#include <tuple>
#include <vector>

#include "songparts.hpp"
#include "time.hpp"

bool is_six_fret_instrument(Instrument instrument);

// Takes a sequence of points where some note type/event is turned on, and a
// sequence where said type is turned off, and returns a tuple of intervals
// where the event is on.
std::vector<std::tuple<Tick, Tick>>
combine_solo_events(const std::vector<int>& on_events,
                    const std::vector<int>& off_events);

std::vector<Solo> form_solo_vector(const std::vector<int>& solo_on_events,
                                   const std::vector<int>& solo_off_events,
                                   const std::vector<Note>& notes,
                                   TrackType track_type, bool is_midi);

#endif
