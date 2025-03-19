/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023, 2025 Raymond Wright
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

#ifndef CHOPT_TESTHELPERS_HPP
#define CHOPT_TESTHELPERS_HPP

#include <cmath>
#include <iomanip>
#include <ostream>
#include <tuple>
#include <vector>

#include <boost/test/unit_test.hpp>

#include <sightread/songparts.hpp>
#include <sightread/time.hpp>

#include "imagebuilder.hpp"
#include "points.hpp"
#include "processed.hpp"
#include "sp.hpp"

inline std::ostream& operator<<(std::ostream& stream, ActValidity validity)
{
    stream << static_cast<int>(validity);
    return stream;
}

inline bool operator==(const Activation& lhs, const Activation& rhs)
{
    return std::tie(lhs.act_start, lhs.act_end)
        == std::tie(rhs.act_start, rhs.act_end)
        && std::abs(lhs.sp_start.value() - rhs.sp_start.value()) < 0.01
        && std::abs(lhs.sp_end.value() - rhs.sp_end.value()) < 0.01;
}

inline std::ostream& operator<<(std::ostream& stream, const Activation& act)
{
    stream << "{Start " << &(*act.act_start) << ", End " << &(*act.act_end)
           << ", SPStart " << act.sp_start.value() << "b, SPEnd "
           << act.sp_end.value() << "b}";
    return stream;
}

inline bool operator==(const DrawnNote& lhs, const DrawnNote& rhs)
{
    for (auto i = 0; i < 7; ++i) {
        if (std::abs(lhs.lengths[i] - rhs.lengths[i]) >= 0.000001) {
            return false;
        }
    }
    return std::abs(lhs.beat - rhs.beat) < 0.000001
        && std::tie(lhs.note_flags, lhs.is_sp_note)
        == std::tie(rhs.note_flags, rhs.is_sp_note);
}

inline std::ostream& operator<<(std::ostream& stream, const DrawnNote& note)
{
    stream << '{' << note.beat << "b, ";
    for (auto i = 0; i < 7; ++i) {
        if (note.lengths[i] != -1) {
            stream << "Colour " << i << " with Length " << note.lengths[i]
                   << ", ";
        }
    }
    stream << ", Is SP Note " << note.is_sp_note << '}';
    return stream;
}

inline bool operator==(const DrawnRow& lhs, const DrawnRow& rhs)
{
    return std::abs(lhs.start - rhs.start) < 0.000001
        && std::abs(lhs.end - rhs.end) < 0.000001;
}

inline std::ostream& operator<<(std::ostream& stream, const DrawnRow& row)
{
    stream << '{' << row.start << ", " << row.end << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, PointPtr addr)
{
    stream << "Point @ " << &(*addr);
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

inline bool operator==(const SpMeasure& lhs, const SpMeasure& rhs)
{
    return std::abs(lhs.value() - rhs.value()) < 0.000001;
}

inline std::ostream& operator<<(std::ostream& stream, SpMeasure measure)
{
    stream << measure.value() << 'm';
    return stream;
}

inline bool operator==(const SpPosition& lhs, const SpPosition& rhs)
{
    return std::abs(lhs.beat.value() - rhs.beat.value()) <= 0.01
        && lhs.sp_measure == rhs.sp_measure;
}

inline std::ostream& operator<<(std::ostream& stream,
                                const SpPosition& position)
{
    stream << "{Beat " << position.beat << ", SpMeasure" << position.sp_measure
           << '}';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream,
                                const std::tuple<SpPosition, int>& tuple)
{
    stream << '{' << std::get<0>(tuple) << ", " << std::get<1>(tuple) << '}';
    return stream;
}

inline SightRead::Note make_note(int position, int length = 0,
                                 SightRead::FiveFretNotes colour
                                 = SightRead::FIVE_FRET_GREEN)
{
    SightRead::Note note;
    note.position = SightRead::Tick {position};
    note.flags = SightRead::FLAGS_FIVE_FRET_GUITAR;
    note.lengths[colour] = SightRead::Tick {length};

    return note;
}

inline SightRead::Note make_chord(
    int position,
    const std::vector<std::tuple<SightRead::FiveFretNotes, int>>& lengths)
{
    SightRead::Note note;
    note.position = SightRead::Tick {position};
    note.flags = SightRead::FLAGS_FIVE_FRET_GUITAR;
    for (auto& [lane, length] : lengths) {
        note.lengths[lane] = SightRead::Tick {length};
    }

    return note;
}

inline SightRead::Note make_ghl_note(int position, int length = 0,
                                     SightRead::SixFretNotes colour
                                     = SightRead::SIX_FRET_WHITE_LOW)
{
    SightRead::Note note;
    note.position = SightRead::Tick {position};
    note.flags = SightRead::FLAGS_SIX_FRET_GUITAR;
    note.lengths[colour] = SightRead::Tick {length};

    return note;
}

inline SightRead::Note
make_drum_note(int position, SightRead::DrumNotes colour = SightRead::DRUM_RED,
               SightRead::NoteFlags flags = SightRead::FLAGS_NONE)
{
    SightRead::Note note;
    note.position = SightRead::Tick {position};
    note.flags
        = static_cast<SightRead::NoteFlags>(flags | SightRead::FLAGS_DRUMS);
    note.lengths[colour] = SightRead::Tick {0};

    return note;
}

inline PathingSettings default_drums_pathing_settings()
{
    return {std::make_unique<ChDrumEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

inline PathingSettings default_fortnite_guitar_pathing_settings()
{
    return {std::make_unique<FortniteGuitarEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

inline PathingSettings default_gh1_pathing_settings()
{
    return {std::make_unique<Gh1Engine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

inline PathingSettings default_guitar_pathing_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

inline PathingSettings default_pro_drums_pathing_settings()
{
    return {std::make_unique<ChDrumEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            {false, false, true, false}};
}

inline PathingSettings default_rb_pathing_settings()
{
    return {std::make_unique<RbEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

inline PathingSettings default_rb3_pathing_settings()
{
    return {std::make_unique<Rb3Engine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

inline PathingSettings positive_video_lag_settings()
{
    return {std::make_unique<ChGuitarEngine>(),
            1.0,
            1.0,
            SightRead::Second {0.0},
            SightRead::Second {0.1},
            SightRead::Second {0.0},
            SightRead::DrumSettings::default_settings()};
}

inline SpDurationData default_measure_mode_data()
{
    return {{{}, SpMode::Measure}, {}, {}};
}

inline SpDurationData default_od_beat_mode_data()
{
    return {{{}, SpMode::OdBeat}, {}, {}};
}

#endif
