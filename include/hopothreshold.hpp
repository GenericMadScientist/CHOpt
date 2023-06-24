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

#ifndef CHOPT_HOPOTHRESHOLD_HPP
#define CHOPT_HOPOTHRESHOLD_HPP

#include <sightread/time.hpp>

enum class HopoThresholdType { Resolution, HopoFrequency, EighthNote };

struct HopoThreshold {
    HopoThresholdType threshold_type;
    SightRead::Tick hopo_frequency;

    SightRead::Tick chart_max_hopo_gap(int resolution) const
    {
        constexpr int DEFAULT_HOPO_GAP = 65;
        constexpr int DEFAULT_RESOLUTION = 192;

        switch (threshold_type) {
        case HopoThresholdType::HopoFrequency:
            return hopo_frequency;
        case HopoThresholdType::EighthNote:
            return SightRead::Tick {(resolution + 3) / 2};
        default:
            return SightRead::Tick {(DEFAULT_HOPO_GAP * resolution)
                                    / DEFAULT_RESOLUTION};
        }
    }

    SightRead::Tick midi_max_hopo_gap(int resolution) const
    {
        switch (threshold_type) {
        case HopoThresholdType::HopoFrequency:
            return hopo_frequency;
        case HopoThresholdType::EighthNote:
            return SightRead::Tick {(resolution + 3) / 2};
        default:
            return SightRead::Tick {resolution / 3 + 1};
        }
    }
};

#endif
