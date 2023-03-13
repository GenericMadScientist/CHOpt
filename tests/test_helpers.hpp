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

#include <array>
#include <cmath>
#include <ostream>
#include <tuple>
#include <vector>

#include "chart.hpp"
#include "imagebuilder.hpp"
#include "midi.hpp"
#include "points.hpp"
#include "processed.hpp"
#include "settings.hpp"
#include "songparts.hpp"
#include "sp.hpp"
#include "time.hpp"

template <typename Iter>
inline std::ostream& print_container(std::ostream& stream, Iter begin, Iter end)
{
    stream << '{';
    if (begin != end) {
        stream << *begin++;
    }
    while (begin != end) {
        stream << ", " << *begin++;
    }
    stream << '}';
    return stream;
}

template <typename T>
inline std::ostream& operator<<(std::ostream& stream,
                                const std::vector<T>& values)
{
    return print_container(stream, values.cbegin(), values.cend());
}

template <typename T, std::size_t N>
inline std::ostream& operator<<(std::ostream& stream,
                                const std::array<T, N>& values)
{
    return print_container(stream, values.cbegin(), values.cend());
}

inline std::ostream& operator<<(std::ostream& stream, ActValidity validity)
{
    stream << static_cast<int>(validity);
    return stream;
}

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

inline bool operator!=(const Activation& lhs, const Activation& rhs)
{
    return std::tie(lhs.act_start, lhs.act_end, lhs.sp_start, lhs.sp_end)
        != std::tie(rhs.act_start, rhs.act_end, rhs.sp_start, rhs.sp_end);
}

inline std::ostream& operator<<(std::ostream& stream, const Activation& act)
{
    stream << "{Start " << &(*act.act_start) << ", End " << &(*act.act_end)
           << ", SPStart " << act.sp_start.value() << "b, SPEnd "
           << act.sp_end.value() << "b}";
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

inline bool operator!=(const BpmEvent& lhs, const BpmEvent& rhs)
{
    return std::tie(lhs.position, lhs.bpm) != std::tie(rhs.position, rhs.bpm);
}

inline std::ostream& operator<<(std::ostream& stream, const BpmEvent& event)
{
    stream << "{Pos " << event.position << ", BPM " << event.bpm << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Difficulty difficulty)
{
    stream << static_cast<int>(difficulty);
    return stream;
}

inline bool operator==(const DiscoFlip& lhs, const DiscoFlip& rhs)
{
    return std::tie(lhs.position, lhs.length)
        == std::tie(rhs.position, rhs.length);
}

inline bool operator!=(const DiscoFlip& lhs, const DiscoFlip& rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& stream, const DiscoFlip& flip)
{
    stream << "{Pos " << flip.position << ", Length " << flip.length << '}';
    return stream;
}

template <typename T>
inline bool operator!=(const DrawnNote<T>& lhs, const DrawnNote<T>& rhs)
{
    return std::abs(lhs.beat - rhs.beat) >= 0.000001
        || std::abs(lhs.length - rhs.length) >= 0.000001
        || std::tie(lhs.colour, lhs.is_sp_note)
        != std::tie(rhs.colour, rhs.is_sp_note);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& stream, const DrawnNote<T>& note)
{
    stream << '{' << note.beat << "b, Length " << note.length << ", Colour "
           << static_cast<int>(note.colour) << ", Is SP Note "
           << note.is_sp_note << '}';
    return stream;
}

inline bool operator!=(const DrawnRow& lhs, const DrawnRow& rhs)
{
    return std::abs(lhs.start - rhs.start) >= 0.000001
        || std::abs(lhs.end - rhs.end) >= 0.000001;
}

inline std::ostream& operator<<(std::ostream& stream, const DrawnRow& row)
{
    stream << '{' << row.start << ", " << row.end << '}';
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

inline std::ostream& operator<<(std::ostream& stream, DrumNoteColour colour)
{
    stream << static_cast<int>(colour);
    return stream;
}

inline bool operator!=(const Event& lhs, const Event& rhs)
{
    return std::tie(lhs.position, lhs.data) != std::tie(rhs.position, rhs.data);
}

inline std::ostream& operator<<(std::ostream& stream, const Event& event)
{
    stream << "{Pos " << event.position << ", Data " << event.data << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Instrument instrument)
{
    stream << static_cast<int>(instrument);
    return stream;
}

inline bool operator==(const Measure& lhs, const Measure& rhs)
{
    return std::abs(lhs.value() - rhs.value()) < 0.000001;
}

inline std::ostream& operator<<(std::ostream& stream, Measure measure)
{
    stream << measure.value() << 'm';
    return stream;
}

inline bool operator==(const MetaEvent& lhs, const MetaEvent& rhs)
{
    return std::tie(lhs.type, lhs.data) == std::tie(rhs.type, rhs.data);
}

inline std::ostream& operator<<(std::ostream& stream, const MetaEvent& event)
{
    stream << "{Type " << event.type << ", Data " << event.data << '}';
    return stream;
}

inline bool operator==(const MidiEvent& lhs, const MidiEvent& rhs)
{
    return std::tie(lhs.status, lhs.data) == std::tie(rhs.status, rhs.data);
}

inline std::ostream& operator<<(std::ostream& stream, const MidiEvent& event)
{
    stream << "{Status " << event.status << ", Data " << event.data << '}';
    return stream;
}

template <typename T>
inline bool operator!=(const Note<T>& lhs, const Note<T>& rhs)
{
    return std::tie(lhs.position, lhs.length, lhs.colour)
        != std::tie(rhs.position, rhs.length, rhs.colour);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& stream, const Note<T>& note)
{
    stream << "{Pos " << note.position << ", Length " << note.length
           << ", Colour " << static_cast<int>(note.colour) << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, NoteColour colour)
{
    stream << static_cast<int>(colour);
    return stream;
}

inline bool operator!=(const NoteEvent& lhs, const NoteEvent& rhs)
{
    return std::tie(lhs.position, lhs.fret, lhs.length)
        != std::tie(rhs.position, rhs.fret, rhs.length);
}

inline std::ostream& operator<<(std::ostream& stream, const NoteEvent& event)
{
    stream << "{Pos " << event.position << ", Fret " << event.fret << ", Length"
           << event.length << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, PointPtr addr)
{
    stream << "Point @ " << &(*addr);
    return stream;
}

inline bool operator==(const Position& lhs, const Position& rhs)
{
    return std::tie(lhs.beat, lhs.measure) == std::tie(rhs.beat, rhs.measure);
}

inline std::ostream& operator<<(std::ostream& stream, const Position& position)
{
    stream << "{Beat " << position.beat << ", Measure" << position.measure
           << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream,
                                const std::tuple<Position, int>& tuple)
{
    stream << '{' << std::get<0>(tuple) << ", " << std::get<1>(tuple) << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Second second)
{
    stream << second.value() << 's';
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

inline bool operator==(const SpBar& lhs, const SpBar& rhs)
{
    return std::abs(lhs.min() - rhs.min()) < 0.000001
        && std::abs(lhs.max() - rhs.max()) < 0.000001;
}

inline std::ostream& operator<<(std::ostream& stream, const SpBar& sp)
{
    stream << "{Min " << sp.min() << ", Max " << sp.max() << '}';
    return stream;
}

inline bool operator!=(const SpecialEvent& lhs, const SpecialEvent& rhs)
{
    return std::tie(lhs.position, lhs.key, lhs.length)
        != std::tie(rhs.position, rhs.key, rhs.length);
}

inline std::ostream& operator<<(std::ostream& stream, const SpecialEvent& event)
{
    stream << "{Pos " << event.position << ", Key " << event.key << ", Length"
           << event.length << '}';
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

inline bool operator==(const SysexEvent& lhs, const SysexEvent& rhs)
{
    return lhs.data == rhs.data;
}

inline std::ostream& operator<<(std::ostream& stream, const SysexEvent& event)
{
    stream << "{Data " << event.data << '}';
    return stream;
}

inline bool operator!=(const TimedEvent& lhs, const TimedEvent& rhs)
{
    return std::tie(lhs.time, lhs.event) != std::tie(rhs.time, rhs.event);
}

inline std::ostream& operator<<(std::ostream& stream, const TimedEvent& event)
{
    stream << "{Time " << event.time << ", ";
    if (std::holds_alternative<MetaEvent>(event.event)) {
        stream << "MetaEvent " << std::get<MetaEvent>(event.event);
    } else if (std::holds_alternative<MidiEvent>(event.event)) {
        stream << "MidiEvent " << std::get<MidiEvent>(event.event);
    } else {
        stream << "SysexEvent " << std::get<SysexEvent>(event.event);
    }
    stream << '}';
    return stream;
}

inline bool operator!=(const TimeSigEvent& lhs, const TimeSigEvent& rhs)
{
    return std::tie(lhs.position, lhs.numerator, lhs.denominator)
        != std::tie(rhs.position, rhs.numerator, rhs.denominator);
}

inline std::ostream& operator<<(std::ostream& stream, const TimeSigEvent& ts)
{
    stream << "{Pos " << ts.position << ", " << ts.numerator << '/'
           << ts.denominator << '}';
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

inline std::ostream& operator<<(std::ostream& stream, TrackType track_type)
{
    stream << static_cast<int>(track_type);
    return stream;
}
