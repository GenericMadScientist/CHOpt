#include <climits>
#include <cstddef>
#include <utility>

#include <sightread/songparts.hpp>

#include "midi.hpp"

namespace {
void throw_on_insufficient_bytes()
{
    throw SightRead::ParseError("insufficient bytes");
}

std::uint8_t pop_front(std::span<const std::uint8_t>& data)
{
    if (data.empty()) {
        throw_on_insufficient_bytes();
    }
    const auto value = data.front();
    data = data.subspan(1);
    return value;
}

// Read a two byte big endian number from the specified offset.
int read_two_byte_be(std::span<const std::uint8_t> span, std::size_t offset)
{
    if (span.size() < offset + 2) {
        throw_on_insufficient_bytes();
    }
    return span[offset] << CHAR_BIT | span[offset + 1];
}

// Read a four byte big endian number from the specified offset.
int read_four_byte_be(std::span<const std::uint8_t> span, std::size_t offset)
{
    if (span.size() < offset + 4) {
        throw_on_insufficient_bytes();
    }
    return span[offset] << (3 * CHAR_BIT) | span[offset + 1] << (2 * CHAR_BIT)
        | span[offset + 2] << CHAR_BIT | span[offset + 3];
}

struct MidiHeader {
    int ticks_per_quarter_note;
    int num_of_tracks;
};

MidiHeader read_midi_header(std::span<const std::uint8_t>& data)
{
    constexpr std::array<std::uint8_t, 10> MAGIC_NUMBER {
        0x4D, 0x54, 0x68, 0x64, 0, 0, 0, 6, 0, 1};
    constexpr int DIVISION_NEGATIVE_SMPTE_MASK = 0x8000;
    constexpr int FIRST_TRACK_OFFSET = 14;
    constexpr int TICKS_OFFSET = 12;
    constexpr int TRACK_COUNT_OFFSET = 10;

    if (data.size() < FIRST_TRACK_OFFSET) {
        throw_on_insufficient_bytes();
    }

    const auto first_ten_bytes = data.subspan(0, MAGIC_NUMBER.size());
    if (!std::equal(first_ten_bytes.begin(), first_ten_bytes.end(),
                    MAGIC_NUMBER.cbegin())) {
        throw SightRead::ParseError("Invalid MIDI file");
    }
    const auto num_of_tracks = read_two_byte_be(data, TRACK_COUNT_OFFSET);
    const auto division = read_two_byte_be(data, TICKS_OFFSET);
    if ((division & DIVISION_NEGATIVE_SMPTE_MASK) != 0) {
        throw SightRead::ParseError("Only ticks per quarter-note is supported");
    }
    data = data.subspan(FIRST_TRACK_OFFSET);
    return {division, num_of_tracks};
}

int read_variable_length_num(std::span<const std::uint8_t>& data)
{
    constexpr int VARIABLE_LENGTH_DATA_MASK = 0x7F;
    constexpr int VARIABLE_LENGTH_DATA_SIZE = 7;
    constexpr int VARIABLE_LENGTH_HIGH_MASK = 0x80;

    int number = 0;
    int bytes_read = 0;
    while (!data.empty() && ((data.front() & VARIABLE_LENGTH_HIGH_MASK) != 0)) {
        ++bytes_read;
        if (bytes_read >= 4) {
            throw SightRead::ParseError("Too long variable length number");
        }
        number <<= VARIABLE_LENGTH_DATA_SIZE;
        number |= pop_front(data) & VARIABLE_LENGTH_DATA_MASK;
    }
    number <<= VARIABLE_LENGTH_DATA_SIZE;
    number |= pop_front(data) & VARIABLE_LENGTH_DATA_MASK;
    return number;
}

SightRead::Detail::MetaEvent
read_meta_event(std::span<const std::uint8_t>& data)
{
    if (data.empty()) {
        throw_on_insufficient_bytes();
    }
    SightRead::Detail::MetaEvent event;
    event.type = pop_front(data);
    const auto data_length = read_variable_length_num(data);
    if (static_cast<std::size_t>(data_length) > data.size()) {
        throw SightRead::ParseError("Meta Event too long");
    }
    event.data
        = std::vector<std::uint8_t> {data.begin(), data.begin() + data_length};
    data = data.subspan(static_cast<std::size_t>(data_length));
    return event;
}

SightRead::Detail::MidiEvent
read_midi_event(std::span<const std::uint8_t>& data, int prev_status_byte)
{
    constexpr int CHANNEL_PRESSURE_ID = 0xD0;
    constexpr int IS_STATUS_BYTE_MASK = 0x80;
    constexpr int PROGRAM_CHANGE_ID = 0xC0;
    constexpr int SYSTEM_COMMON_MSG_ID = 0xF0;
    constexpr int UPPER_NIBBLE_MASK = 0xF0;

    if (data.empty()) {
        throw_on_insufficient_bytes();
    }
    auto event_type = data.front();
    if ((event_type & IS_STATUS_BYTE_MASK) != 0) {
        data = data.subspan(1);
    } else if (prev_status_byte != -1) {
        event_type = static_cast<std::uint8_t>(prev_status_byte);
    } else {
        throw SightRead::ParseError(
            "MIDI Event has no status byte and there is no running status");
    }

    if ((event_type & UPPER_NIBBLE_MASK) == SYSTEM_COMMON_MSG_ID) {
        throw SightRead::ParseError(
            "MIDI Events with high nibble 0xF are not supported");
    }
    std::array<std::uint8_t, 2> event_data {pop_front(data), 0};
    if ((event_type & UPPER_NIBBLE_MASK) != PROGRAM_CHANGE_ID
        && (event_type & UPPER_NIBBLE_MASK) != CHANNEL_PRESSURE_ID) {
        event_data[1] = pop_front(data);
    }

    return {event_type, event_data};
}

SightRead::Detail::SysexEvent
read_sysex_event(std::span<const std::uint8_t>& data)
{
    const auto data_length = read_variable_length_num(data);
    if (static_cast<std::size_t>(data_length) > data.size()) {
        throw SightRead::ParseError("Sysex Event too long");
    }
    SightRead::Detail::SysexEvent event;
    event.data
        = std::vector<std::uint8_t> {data.begin(), data.begin() + data_length};
    data = data.subspan(static_cast<std::size_t>(data_length));
    return event;
}

SightRead::Detail::MidiTrack
read_midi_track(std::span<const std::uint8_t>& data)
{
    constexpr int META_EVENT_ID = 0xFF;
    constexpr int SYSEX_EVENT_ID = 0xF0;
    constexpr int TRACK_HEADER_MAGIC_NUMBER = 0x4D54726B;
    constexpr int TRACK_HEADER_SIZE = 8;

    if (read_four_byte_be(data, 0) != TRACK_HEADER_MAGIC_NUMBER) {
        throw SightRead::ParseError("Invalid MIDI file");
    }
    const auto track_size = read_four_byte_be(data, 4);
    data = data.subspan(TRACK_HEADER_SIZE);
    auto absolute_time = 0;
    const auto final_span_size
        = data.size() - static_cast<std::size_t>(track_size);
    auto prev_status_byte = -1;
    SightRead::Detail::MidiTrack track;
    while (data.size() != final_span_size) {
        const auto delta_time = read_variable_length_num(data);
        absolute_time += delta_time;
        SightRead::Detail::TimedEvent event {absolute_time, {}};
        if (data.empty()) {
            throw_on_insufficient_bytes();
        }
        const auto event_type = data.front();
        if (event_type == META_EVENT_ID) {
            data = data.subspan(1);
            event.event = read_meta_event(data);
        } else if (event_type == SYSEX_EVENT_ID) {
            data = data.subspan(1);
            event.event = read_sysex_event(data);
        } else {
            const auto midi_event = read_midi_event(data, prev_status_byte);
            prev_status_byte = midi_event.status;
            event.event = midi_event;
        }
        track.events.push_back(std::move(event));
    }
    return track;
}
}

SightRead::Detail::Midi
SightRead::Detail::parse_midi(std::span<const std::uint8_t> data)
{
    const auto header = read_midi_header(data);
    std::vector<SightRead::Detail::MidiTrack> tracks;
    for (auto i = 0; i < header.num_of_tracks && !data.empty(); ++i) {
        tracks.push_back(read_midi_track(data));
    }
    return SightRead::Detail::Midi {header.ticks_per_quarter_note,
                                    std::move(tracks)};
}
