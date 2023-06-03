/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2023 Raymond Wright
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

#ifndef CHOPT_MIDI_HPP
#define CHOPT_MIDI_HPP

#include <array>
#include <cstdint>
#include <span>
#include <variant>
#include <vector>

struct MetaEvent {
    int type;
    std::vector<std::uint8_t> data;
};

struct MidiEvent {
    int status;
    std::array<std::uint8_t, 2> data;
};

struct SysexEvent {
    std::vector<std::uint8_t> data;
};

struct TimedEvent {
    int time {0};
    std::variant<MetaEvent, MidiEvent, SysexEvent> event;
};

struct MidiTrack {
    std::vector<TimedEvent> events;
};

struct Midi {
    int ticks_per_quarter_note;
    std::vector<MidiTrack> tracks;
};

Midi parse_midi(std::span<const std::uint8_t> data);

#endif
