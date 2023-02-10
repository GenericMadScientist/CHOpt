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

#include <cmath>
#include <ostream>
#include <tuple>

#include "settings.hpp"
#include "songparts.hpp"
#include "time.hpp"

inline bool operator==(const Beat& lhs, const Beat& rhs)
{
    return std::abs(lhs.value() - rhs.value()) < 0.01;
}

inline bool operator!=(const Beat& lhs, const Beat& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& stream, Beat beat)
{
    stream << beat.value() << 'b';
    return stream;
}

inline bool operator!=(const BPM& lhs, const BPM& rhs)
{
    return std::tie(lhs.position, lhs.bpm) != std::tie(rhs.position, rhs.bpm);
}

inline std::ostream& operator<<(std::ostream& stream, const BPM& bpm)
{
    stream << "{Pos " << bpm.position << ", BPM " << bpm.bpm << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Difficulty difficulty)
{
    stream << static_cast<int>(difficulty);
    return stream;
}

inline bool operator==(const DrumFill& lhs, const DrumFill& rhs)
{
    return std::tie(lhs.position, lhs.length)
        == std::tie(rhs.position, rhs.length);
}

inline bool operator!=(const DrumFill& lhs, const DrumFill& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& stream, const DrumFill& fill)
{
    stream << "{Pos " << fill.position << ", Length " << fill.length << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Measure measure)
{
    stream << measure.value() << 'm';
    return stream;
}

inline bool operator!=(const Solo& lhs, const Solo& rhs)
{
    return std::tie(lhs.start, lhs.end, lhs.value)
        != std::tie(rhs.start, rhs.end, rhs.value);
}

inline std::ostream& operator<<(std::ostream& stream, const Solo& solo)
{
    stream << "{Start " << solo.start << ", End " << solo.end << ", Value "
           << solo.value << '}';
    return stream;
}

inline bool operator!=(const StarPower& lhs, const StarPower& rhs)
{
    return std::tie(lhs.position, lhs.length)
        != std::tie(rhs.position, rhs.length);
}

inline std::ostream& operator<<(std::ostream& stream, const StarPower& sp)
{
    stream << "{Pos " << sp.position << ", Length " << sp.length << '}';
    return stream;
}

inline bool operator!=(const TimeSignature& lhs, const TimeSignature& rhs)
{
    return std::tie(lhs.position, lhs.numerator, lhs.denominator)
        != std::tie(rhs.position, rhs.numerator, rhs.denominator);
}

inline std::ostream& operator<<(std::ostream& stream, const TimeSignature& ts)
{
    stream << "{Pos " << ts.position << ", " << ts.numerator << '/'
           << ts.denominator << '}';
    return stream;
}
