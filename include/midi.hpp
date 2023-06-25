#ifndef SIGHTREAD_DETAIL_MIDI_HPP
#define SIGHTREAD_DETAIL_MIDI_HPP

#include <array>
#include <cstdint>
#include <span>
#include <variant>
#include <vector>

namespace SightRead::Detail {
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
}

#endif
