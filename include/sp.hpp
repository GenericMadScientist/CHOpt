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

#include "engine.hpp"
#include "settings.hpp"
#include "songparts.hpp"
#include "time.hpp"
#include "timeconverter.hpp"

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
        Beat position;
        double net_sp_gain_rate;
    };

    struct WhammyRange {
        Position start;
        Position end;
        Beat note;
    };

    static constexpr double DEFAULT_BEATS_PER_BAR = 32.0;
    static constexpr double MEASURES_PER_BAR = 8.0;

    TimeConverter m_converter;
    std::vector<BeatRate> m_beat_rates;
    std::vector<WhammyRange> m_whammy_ranges;
    Beat m_last_whammy_point {-std::numeric_limits<double>::infinity()};
    std::vector<std::vector<WhammyRange>::const_iterator> m_initial_guesses;
    const double m_sp_gain_rate;
    const double m_default_net_sp_gain_rate;

    [[nodiscard]] double
    propagate_over_whammy_range(Beat start, Beat end,
                                double sp_bar_amount) const;
    [[nodiscard]] Beat whammy_propagation_endpoint(Beat start, Beat end,
                                                   double sp_bar_amount) const;
    [[nodiscard]] std::vector<WhammyRange>::const_iterator
    first_whammy_range_after(Beat pos) const;

    static std::vector<BeatRate>
    form_beat_rates(int resolution, const SyncTrack& sync_track,
                    const std::vector<int>& od_beats, const Engine& engine);

    template <typename T>
    std::vector<std::tuple<int, int, Second>>
    note_spans(const NoteTrack<T>& track, double early_whammy,
               const Engine& engine)
    {
        std::vector<std::tuple<int, int, Second>> spans;
        for (auto note = track.notes().cbegin(); note < track.notes().cend();
             ++note) {
            auto early_gap = std::numeric_limits<double>::infinity();
            auto late_gap = std::numeric_limits<double>::infinity();
            const auto current_note_time
                = m_converter
                      .beats_to_seconds(
                          Beat {note->position
                                / static_cast<double>(track.resolution())})
                      .value();
            if (note != track.notes().cbegin()) {
                early_gap = current_note_time
                    - m_converter
                          .beats_to_seconds(
                              Beat {std::prev(note)->position
                                    / static_cast<double>(track.resolution())})
                          .value();
            }
            if (std::next(note) < track.notes().cend()) {
                late_gap = m_converter
                               .beats_to_seconds(Beat {
                                   std::next(note)->position
                                   / static_cast<double>(track.resolution())})
                               .value()
                    - current_note_time;
            }
            spans.push_back(
                {note->position, note->length,
                 Second {engine.early_timing_window(early_gap, late_gap)}
                     * early_whammy});
        }
        return spans;
    }

    Position sp_drain_end_point(Position start, double sp_bar_amount) const;

    void initialise(const std::vector<std::tuple<int, int, Second>>& note_spans,
                    const std::vector<StarPower>& phrases, int resolution,
                    const SqueezeSettings& squeeze_settings);

public:
    template <typename T>
    SpData(const NoteTrack<T>& track, const SyncTrack& sync_track,
           const std::vector<int>& od_beats,
           const SqueezeSettings& squeeze_settings, const Engine& engine)
        : m_converter {sync_track, track.resolution(), engine, od_beats}
        , m_beat_rates {form_beat_rates(track.resolution(), sync_track,
                                        od_beats, engine)}
        , m_sp_gain_rate {engine.sp_gain_rate()}
        , m_default_net_sp_gain_rate {m_sp_gain_rate
                                      - 1 / DEFAULT_BEATS_PER_BAR}
    {
        initialise(note_spans(track, squeeze_settings.early_whammy, engine),
                   track.sp_phrases(), track.resolution(), squeeze_settings);
    }

    // Return the maximum amount of SP available at the end after propagating
    // over a range, or -1 if SP runs out at any point. Only includes SP gain
    // from whammy.
    [[nodiscard]] double
    propagate_sp_over_whammy_max(Position start, Position end, double sp) const;
    // Return the minimum amount of SP is available at the end after propagating
    // over a range, returning 0.0 if the minimum would hypothetically be
    // negative.
    [[nodiscard]] double
    propagate_sp_over_whammy_min(Position start, Position end, double sp,
                                 Position required_whammy_end) const;
    // Return if a beat is at a place that can be whammied.
    [[nodiscard]] bool is_in_whammy_ranges(Beat beat) const;
    // Return the amount of whammy obtainable across a range.
    [[nodiscard]] double available_whammy(Beat start, Beat end) const;
    // Return the amount of whammy obtainable across a range, from notes before
    // note_pos.
    [[nodiscard]] double available_whammy(Beat start, Beat end,
                                          Beat note_pos) const;
    // Return how far an activation can propagate based on whammy, returning the
    // end of the range if it can be reached.
    [[nodiscard]] Position activation_end_point(Position start, Position end,
                                                double sp_bar_amount) const;
};

#endif
