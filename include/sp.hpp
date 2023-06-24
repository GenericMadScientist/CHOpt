/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023 Raymond Wright
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

#include <sightread/time.hpp>

#include "engine.hpp"
#include "settings.hpp"
#include "songparts.hpp"
#include "sptimemap.hpp"
#include "tempomap.hpp"

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
        SightRead::Beat position;
        double net_sp_gain_rate;
    };

    struct WhammyRange {
        SpPosition start;
        SpPosition end;
        SightRead::Beat note;
    };

    struct WhammyPropagationState {
        std::vector<BeatRate>::const_iterator current_beat_rate;
        SightRead::Beat current_position;
        double current_sp;
    };

    static constexpr double DEFAULT_BEATS_PER_BAR = 32.0;
    static constexpr double MEASURES_PER_BAR = 8.0;

    SpTimeMap m_time_map;
    std::vector<BeatRate> m_beat_rates;
    std::vector<WhammyRange> m_whammy_ranges;
    SightRead::Beat m_last_whammy_point {
        -std::numeric_limits<double>::infinity()};
    std::vector<std::vector<WhammyRange>::const_iterator> m_initial_guesses;
    const double m_sp_gain_rate;
    const double m_default_net_sp_gain_rate;

    static std::vector<BeatRate>
    form_beat_rates(const SightRead::TempoMap& tempo_map,
                    const std::vector<SightRead::Tick>& od_beats,
                    const Engine& engine);

    [[nodiscard]] double
    propagate_over_whammy_range(SightRead::Beat start, SightRead::Beat end,
                                double sp_bar_amount) const;
    [[nodiscard]] SightRead::Beat
    whammy_propagation_endpoint(SightRead::Beat start, SightRead::Beat end,
                                double sp_bar_amount) const;
    [[nodiscard]] std::vector<WhammyRange>::const_iterator
    first_whammy_range_after(SightRead::Beat pos) const;
    [[nodiscard]] WhammyPropagationState
    initial_whammy_prop_state(SightRead::Beat start, SightRead::Beat end,
                              double sp_bar_amount) const;
    SpPosition sp_drain_end_point(SpPosition start, double sp_bar_amount) const;

public:
    SpData(const NoteTrack& track, SpTimeMap time_map,
           const std::vector<SightRead::Tick>& od_beats,
           const SqueezeSettings& squeeze_settings, const Engine& engine);
    // Return the maximum amount of SP available at the end after propagating
    // over a range, or -1 if SP runs out at any point. Only includes SP gain
    // from whammy.
    [[nodiscard]] double propagate_sp_over_whammy_max(SpPosition start,
                                                      SpPosition end,
                                                      double sp) const;
    // Return the minimum amount of SP is available at the end after propagating
    // over a range, returning 0.0 if the minimum would hypothetically be
    // negative.
    [[nodiscard]] double
    propagate_sp_over_whammy_min(SpPosition start, SpPosition end, double sp,
                                 SpPosition required_whammy_end) const;
    // Return if a beat is at a place that can be whammied.
    [[nodiscard]] bool is_in_whammy_ranges(SightRead::Beat beat) const;
    // Return the amount of whammy obtainable across a range.
    [[nodiscard]] double available_whammy(SightRead::Beat start,
                                          SightRead::Beat end) const;
    // Return the amount of whammy obtainable across a range, from notes before
    // note_pos.
    [[nodiscard]] double available_whammy(SightRead::Beat start,
                                          SightRead::Beat end,
                                          SightRead::Beat note_pos) const;
    // Return how far an activation can propagate based on whammy, returning the
    // end of the range if it can be reached.
    [[nodiscard]] SpPosition activation_end_point(SpPosition start,
                                                  SpPosition end,
                                                  double sp_bar_amount) const;
};

#endif
