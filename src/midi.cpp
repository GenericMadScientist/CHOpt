/*
 * chopt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020 Raymond Wright
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
#include <array>
#include <climits>
#include <cstddef>
#include <stdexcept>
#include <utility>

#include "midi.hpp"

// This is a version of string_view for uint8_t. Once C++20 is ratified we can
// replace this with span<uint8_t>.
class ByteSpan {
private:
    using Iter = std::vector<std::uint8_t>::const_iterator;
    static constexpr std::size_t MAX_SIZE = static_cast<std::size_t>(-1);
    Iter m_begin;
    Iter m_end;
    std::size_t m_size;
    ByteSpan(Iter begin, Iter end)
        : m_begin {begin}
        , m_end {end}
        , m_size {static_cast<std::size_t>(m_end - m_begin)}
    {
    }

public:
    explicit ByteSpan(const std::vector<std::uint8_t>& data)
        : m_begin {data.cbegin()}
        , m_end {data.cend()}
        , m_size {data.size()}
    {
    }

    [[nodiscard]] std::vector<std::uint8_t>::const_iterator begin() const
    {
        return m_begin;
    }
    [[nodiscard]] std::vector<std::uint8_t>::const_iterator end() const
    {
        return m_end;
    }
    [[nodiscard]] std::size_t size() const { return m_size; }

    std::uint8_t operator[](std::size_t index) const
    {
        if (index >= m_size) {
            throw std::invalid_argument("index too large");
        }
        return *(m_begin + static_cast<std::ptrdiff_t>(index));
    }

    // Returns the subspan starting at offset of size count. Throws an exception
    // if offset is OOB. If count is -1, then returns the rest of the span, else
    // if count would go OOB then an exception is thrown.
    [[nodiscard]] ByteSpan subspan(std::size_t offset,
                                   std::size_t count = MAX_SIZE) const
    {
        if (offset > m_size) {
            throw std::invalid_argument("offset would be OOB");
        }
        if (count == MAX_SIZE) {
            return ByteSpan {m_begin + static_cast<std::ptrdiff_t>(offset),
                             m_end};
        }
        if (count > m_size - offset) {
            throw std::invalid_argument("count is too large");
        }
        return ByteSpan {m_begin + static_cast<std::ptrdiff_t>(offset),
                         m_begin + static_cast<std::ptrdiff_t>(offset + count)};
    }
};

// Read a two byte big endian number from the specified offset.
static int read_two_byte_be(ByteSpan span, std::size_t offset)
{
    return span[offset] << CHAR_BIT | span[offset + 1];
}

// Read a four byte big endian number from the specified offset.
static int read_four_byte_be(ByteSpan span, std::size_t offset)
{
    return span[offset] << (3 * CHAR_BIT) | span[offset + 1] << (2 * CHAR_BIT)
        | span[offset + 2] << CHAR_BIT | span[offset + 3];
}

struct MidiHeader {
    int ticks_per_quarter_note;
    int num_of_tracks;
};

static ByteSpan read_midi_header(ByteSpan span, MidiHeader& header)
{
    constexpr std::array<std::uint8_t, 10> MAGIC_NUMBER {
        0x4D, 0x54, 0x68, 0x64, 0, 0, 0, 6, 0, 1};
    constexpr int FIRST_TRACK_OFFSET = 14;
    constexpr int TICKS_OFFSET = 12;
    constexpr int TRACK_COUNT_OFFSET = 10;

    const auto first_ten_bytes = span.subspan(0, MAGIC_NUMBER.size());
    if (!std::equal(first_ten_bytes.begin(), first_ten_bytes.end(),
                    MAGIC_NUMBER.cbegin())) {
        throw std::invalid_argument("Invalid MIDI file");
    }
    header.num_of_tracks = read_two_byte_be(span, TRACK_COUNT_OFFSET);
    header.ticks_per_quarter_note = read_two_byte_be(span, TICKS_OFFSET);
    return span.subspan(FIRST_TRACK_OFFSET);
}

static ByteSpan read_meta_event(ByteSpan span, MetaEvent& event)
{
    event.type = span[0];
    auto data_length = span[1];
    span = span.subspan(2);
    event.data
        = std::vector<std::uint8_t> {span.begin(), span.begin() + data_length};
    return span.subspan(data_length);
}

static ByteSpan read_midi_track(ByteSpan span, MidiTrack& track)
{
    constexpr int IS_STATUS_BYTE_MASK = 0x80;
    constexpr int META_EVENT_ID = 0xFF;
    constexpr int TRACK_HEADER_MAGIC_NUMBER = 0x4D54726B;
    constexpr int TRACK_HEADER_SIZE = 8;
    constexpr int VARIABLE_LENGTH_DATA_MASK = 0x7F;
    constexpr int VARIABLE_LENGTH_DATA_SIZE = 7;
    constexpr int VARIABLE_LENGTH_HIGH_MASK = 0x80;

    if (read_four_byte_be(span, 0) != TRACK_HEADER_MAGIC_NUMBER) {
        throw std::invalid_argument("Invalid MIDI file");
    }
    track.size = read_four_byte_be(span, 4);
    span = span.subspan(TRACK_HEADER_SIZE);
    auto absolute_time = 0;
    const auto final_span_size
        = span.size() - static_cast<std::size_t>(track.size);
    auto prev_status_byte = -1;
    while (span.size() != final_span_size) {
        TimedEvent event {};
        auto delta_time = 0;
        while ((span[0] & VARIABLE_LENGTH_HIGH_MASK) != 0) {
            delta_time <<= VARIABLE_LENGTH_DATA_SIZE;
            delta_time |= span[0] & VARIABLE_LENGTH_DATA_MASK;
            span = span.subspan(1);
        }
        delta_time <<= VARIABLE_LENGTH_DATA_SIZE;
        delta_time |= span[0] & VARIABLE_LENGTH_DATA_MASK;
        absolute_time += delta_time;
        event.time = absolute_time;
        auto event_type = span[1];
        if (event_type == META_EVENT_ID) {
            prev_status_byte = -1;
            span = span.subspan(2);
            MetaEvent meta_event {};
            span = read_meta_event(span, meta_event);
            event.event = meta_event;
        } else {
            if ((event_type & IS_STATUS_BYTE_MASK) != 0) {
                prev_status_byte = event_type;
                span = span.subspan(2);
            } else if (prev_status_byte != -1) {
                event_type = static_cast<std::uint8_t>(prev_status_byte);
                span = span.subspan(1);
            } else {
                throw std::invalid_argument("MIDI Event has no status byte and "
                                            "there is no running status");
            }
            MidiEvent midi_event {event_type, {span[0], span[1]}};
            event.event = midi_event;
            span = span.subspan(2);
        }
        track.events.push_back(event);
    }
    return span;
}

Midi parse_midi(const std::vector<std::uint8_t>& data)
{
    ByteSpan span {data};
    MidiHeader header {};
    span = read_midi_header(span, header);
    std::vector<MidiTrack> tracks;
    for (auto i = 0; i < header.num_of_tracks; ++i) {
        MidiTrack track {};
        span = read_midi_track(span, track);
        tracks.push_back(track);
    }
    return Midi {header.ticks_per_quarter_note, std::move(tracks)};
}
