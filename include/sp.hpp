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

#ifndef CHOPT_SP_HPP
#define CHOPT_SP_HPP

#include <algorithm>
#include <limits>
#include <tuple>
#include <vector>

#include "time.hpp"

// Represents the minimum and maximum SP possible at a given time.
class SpBar {
private:
    double m_min;
    double m_max;

public:
    SpBar(double min, double max)
        : m_min {min}
        , m_max {max}
    {
    }

    double& min() { return m_min; }
    double& max() { return m_max; }
    [[nodiscard]] double min() const { return m_min; }
    [[nodiscard]] double max() const { return m_max; }

    friend bool operator==(const SpBar& lhs, const SpBar& rhs)
    {
        return std::tie(lhs.m_min, lhs.m_max) == std::tie(rhs.m_min, rhs.m_max);
    }

    void add_phrase()
    {
        constexpr double SP_PHRASE_AMOUNT = 0.25;

        m_min += SP_PHRASE_AMOUNT;
        m_max += SP_PHRASE_AMOUNT;
        m_min = std::min(m_min, 1.0);
        m_max = std::min(m_max, 1.0);
    }

    [[nodiscard]] bool full_enough_to_activate() const
    {
        constexpr double MINIMUM_SP_AMOUNT = 0.5;
        return m_max >= MINIMUM_SP_AMOUNT;
    }
};

// This is used by the optimiser to calculate SP drain.
class SpData {
private:
    struct BeatRate {
        Beat position;
        double net_sp_gain_rate;
    };

    struct WhammyRange {
        Position start;
        Position end;
    };

    static constexpr double DEFAULT_NET_SP_GAIN_RATE = 1 / 480.0;
    static constexpr double NEG_INF = -std::numeric_limits<double>::infinity();
    static constexpr double SP_GAIN_RATE = 1 / 30.0;

    TimeConverter m_converter;
    std::vector<BeatRate> m_beat_rates;
    std::vector<WhammyRange> m_whammy_ranges;

    [[nodiscard]] double
    propagate_over_whammy_range(Beat start, Beat end,
                                double sp_bar_amount) const;
    [[nodiscard]] Beat whammy_propagation_endpoint(Beat start, Beat end,
                                                   double sp_bar_amount) const;

    static std::vector<BeatRate> form_beat_rates(int resolution,
                                                 const SyncTrack& sync_track);

public:
    SpData(const NoteTrack<NoteColour>& track, int resolution,
           const SyncTrack& sync_track, double early_whammy,
           Second lazy_whammy);

    // Return how much SP is available at the end after propagating over a
    // range, or -1 if SP runs out at any point. Only includes SP gain from
    // whammy.
    [[nodiscard]] SpBar
    propagate_sp_over_whammy(Position start, Position end, SpBar sp_bar,
                             Position required_whammy_end
                             = {Beat {NEG_INF}, Measure {NEG_INF}}) const;
    // Return if a beat is at a place that can be whammied.
    [[nodiscard]] bool is_in_whammy_ranges(Beat beat) const;
    // Return the amount of whammy obtainable across a range.
    [[nodiscard]] double available_whammy(Beat start, Beat end) const;
    // Return how far an activation can propagate based on whammy, returning the
    // end of the range if it can be reached.
    [[nodiscard]] Position activation_end_point(Position start, Position end,
                                                double sp_bar_amount) const;
};

#endif
