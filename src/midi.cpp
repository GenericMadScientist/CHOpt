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
#include <cstddef>
#include <stdexcept>

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

static bool is_header_valid(ByteSpan span)
{
    constexpr std::array<std::uint8_t, 10> MAGIC_NUMBER {
        0x4D, 0x54, 0x68, 0x64, 0, 0, 0, 6, 0, 1};
    const auto first_ten_bytes = span.subspan(0, 10);
    return std::equal(first_ten_bytes.begin(), first_ten_bytes.end(),
                      MAGIC_NUMBER.cbegin());
}

Midi parse_midi(const std::vector<std::uint8_t>& data)
{
    ByteSpan span {data};
    if (!is_header_valid(span)) {
        throw std::invalid_argument("Invalid MIDI file");
    }
    const auto num_of_tracks = (span[10] << 8U) + span[11];
    const auto ticks_per_quarter_note = (span[12] << 8U) + span[13];
    return Midi {ticks_per_quarter_note, num_of_tracks};
}
