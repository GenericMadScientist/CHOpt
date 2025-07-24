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

#include <stdexcept>

#include "sptimemap.hpp"

SightRead::Beat SpTimeMap::to_beats(SightRead::Fretbar fretbars) const
{
    return m_tempo_map.to_beats(fretbars);
}

SightRead::Beat SpTimeMap::to_beats(SightRead::Second seconds) const
{
    return m_tempo_map.to_beats(seconds);
}

SightRead::Beat SpTimeMap::to_beats(SpMeasure measures) const
{
    switch (m_sp_mode) {
    case SpMode::Measure:
        return m_tempo_map.to_beats(SightRead::Measure {measures.value()});
    case SpMode::OdBeat:
        return m_tempo_map.to_beats(SightRead::OdBeat {measures.value()});
    default:
        throw std::runtime_error("Invalid SpMode value");
    }
}

SightRead::Beat SpTimeMap::to_beats(SightRead::Tick ticks) const
{
    return m_tempo_map.to_beats(ticks);
}

SightRead::Fretbar SpTimeMap::to_fretbars(SightRead::Beat beats) const
{
    return m_tempo_map.to_fretbars(beats);
}

SightRead::Fretbar SpTimeMap::to_fretbars(SightRead::Second seconds) const
{
    return m_tempo_map.to_fretbars(m_tempo_map.to_beats(seconds));
}

SightRead::Fretbar SpTimeMap::to_fretbars(SightRead::Tick ticks) const
{
    return m_tempo_map.to_fretbars(ticks);
}

SightRead::Second SpTimeMap::to_seconds(SightRead::Beat beats) const
{
    return m_tempo_map.to_seconds(beats);
}

SightRead::Second SpTimeMap::to_seconds(SightRead::Fretbar fretbars) const
{
    return m_tempo_map.to_seconds(m_tempo_map.to_beats(fretbars));
}

SightRead::Second SpTimeMap::to_seconds(SpMeasure sp_measures) const
{
    return to_seconds(to_beats(sp_measures));
}

SpMeasure SpTimeMap::to_sp_measures(SightRead::Beat beats) const
{
    switch (m_sp_mode) {
    case SpMode::Measure:
        return SpMeasure {m_tempo_map.to_measures(beats).value()};
    case SpMode::OdBeat:
        return SpMeasure {m_tempo_map.to_od_beats(beats).value()};
    default:
        throw std::runtime_error("Invalid SpMode value");
    }
}

SpMeasure SpTimeMap::to_sp_measures(SightRead::Second seconds) const
{
    return to_sp_measures(m_tempo_map.to_beats(seconds));
}
