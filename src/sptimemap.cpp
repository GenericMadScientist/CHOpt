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

#include <stdexcept>

#include "sptimemap.hpp"

Beat SpTimeMap::to_beats(Second seconds) const
{
    return m_tempo_map.to_beats(seconds);
}

Beat SpTimeMap::to_beats(SpMeasure sp_measures) const
{
    return m_tempo_map.to_beats(sp_measures);
}

Beat SpTimeMap::to_beats(Tick ticks) const
{
    return m_tempo_map.to_beats(ticks);
}

Second SpTimeMap::to_seconds(Beat beats) const
{
    return m_tempo_map.to_seconds(beats);
}

SpMeasure SpTimeMap::to_sp_measures(Beat beats) const
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

SpMeasure SpTimeMap::to_sp_measures(Second seconds) const
{
    return to_sp_measures(m_tempo_map.to_beats(seconds));
}