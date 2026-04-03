/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2021, 2022, 2023, 2024, 2025, 2026 Raymond Wright
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

#ifndef CHOPT_ENGINE_HPP
#define CHOPT_ENGINE_HPP

#include <algorithm>
#include <cstdint>

#include <sightread/time.hpp>

#include "sptimemap.hpp"

enum class SustainRoundingPolicy : std::uint8_t { RoundUp, RoundToNearest };

enum class SustainTicksMetric : std::uint8_t { Beat, Fretbar, OdBeat };

struct SpEngineValues {
    double phrase_amount;
    double unison_phrase_amount;
    double minimum_to_activate;
};

class Engine {
public:
    [[nodiscard]] virtual int base_note_value() const = 0;
    [[nodiscard]] virtual int base_cymbal_value() const
    {
        return base_note_value();
    }
    [[nodiscard]] virtual double burst_size() const = 0;
    [[nodiscard]] virtual bool chords_multiply_sustains() const = 0;
    [[nodiscard]] virtual bool delayed_multiplier() const = 0;
    [[nodiscard]] virtual double early_timing_window(double early_gap,
                                                     double late_gap) const = 0;
    [[nodiscard]] virtual bool has_bres() const = 0;
    [[nodiscard]] virtual bool has_early_whammy() const = 0;
    [[nodiscard]] virtual bool has_unison_bonuses() const = 0;
    [[nodiscard]] virtual bool is_rock_band() const = 0;
    [[nodiscard]] virtual bool ignore_average_multiplier() const = 0;
    [[nodiscard]] virtual double late_timing_window(double early_gap,
                                                    double late_gap) const = 0;
    [[nodiscard]] virtual int max_multiplier() const = 0;
    [[nodiscard]] virtual bool merge_uneven_sustains() const = 0;
    [[nodiscard]] virtual bool overlaps() const = 0;
    [[nodiscard]] virtual bool round_tick_gap() const = 0;
    [[nodiscard]] virtual SightRead::Tick snap_gap() const = 0;
    [[nodiscard]] virtual SpEngineValues sp_engine_values() const = 0;
    [[nodiscard]] virtual SpGainMode sp_gain_mode() const = 0;
    [[nodiscard]] virtual double sp_gain_rate() const = 0;
    [[nodiscard]] virtual SpMode sp_mode() const = 0;
    [[nodiscard]] virtual int sust_points_per_beat() const = 0;
    [[nodiscard]] virtual SustainRoundingPolicy sustain_rounding() const = 0;
    [[nodiscard]] virtual SustainTicksMetric sustain_ticks_metric() const = 0;

    Engine() = default;
    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;
    virtual ~Engine() = default;
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
class BaseChEngine : public Engine {
protected:
    [[nodiscard]] virtual double timing_window(double early_gap,
                                               double late_gap) const = 0;

public:
    [[nodiscard]] int base_cymbal_value() const override { return 65; }
    [[nodiscard]] int base_note_value() const override { return 50; }
    [[nodiscard]] double burst_size() const override { return 0.25; }
    [[nodiscard]] bool chords_multiply_sustains() const override
    {
        return false;
    }
    [[nodiscard]] bool delayed_multiplier() const override { return false; }
    [[nodiscard]] double early_timing_window(double early_gap,
                                             double late_gap) const override
    {
        return timing_window(early_gap, late_gap);
    }
    [[nodiscard]] bool has_bres() const override { return false; }
    [[nodiscard]] bool has_early_whammy() const override { return true; }
    [[nodiscard]] bool has_unison_bonuses() const override { return false; }
    [[nodiscard]] bool ignore_average_multiplier() const override
    {
        return false;
    }
    [[nodiscard]] bool is_rock_band() const override { return false; }
    [[nodiscard]] double late_timing_window(double early_gap,
                                            double late_gap) const override
    {
        return timing_window(early_gap, late_gap);
    }
    [[nodiscard]] int max_multiplier() const override { return 4; }
    [[nodiscard]] bool merge_uneven_sustains() const override { return false; }
    [[nodiscard]] bool overlaps() const override { return true; }
    [[nodiscard]] bool round_tick_gap() const override { return true; }
    [[nodiscard]] SightRead::Tick snap_gap() const override
    {
        return SightRead::Tick {0};
    }
    [[nodiscard]] SpEngineValues sp_engine_values() const override
    {
        return {.phrase_amount = 0.25,
                .unison_phrase_amount = 0.5,
                .minimum_to_activate = 0.5};
    }
    [[nodiscard]] SpGainMode sp_gain_mode() const override
    {
        return SpGainMode::Beat;
    }
    [[nodiscard]] double sp_gain_rate() const override { return 1 / 30.0; }
    [[nodiscard]] SpMode sp_mode() const override { return SpMode::Measure; }
    [[nodiscard]] int sust_points_per_beat() const override { return 25; }
    [[nodiscard]] SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundUp;
    }
    [[nodiscard]] SustainTicksMetric sustain_ticks_metric() const override
    {
        return SustainTicksMetric::Beat;
    }
};

class ChGuitarEngine final : public BaseChEngine {
protected:
    [[nodiscard]] double timing_window(double early_gap,
                                       double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.07;
    }
};

class ChPrecisionGuitarEngine final : public BaseChEngine {
protected:
    [[nodiscard]] double timing_window(double early_gap,
                                       double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.0, 0.0525);
        late_gap = std::clamp(late_gap, 0.0, 0.0525);
        const auto total_gap = early_gap + late_gap;
        return 0.27619 * total_gap + 0.021;
    }
};

class ChDrumEngine final : public BaseChEngine {
protected:
    [[nodiscard]] double timing_window(double early_gap,
                                       double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.0375, 0.085);
        late_gap = std::clamp(late_gap, 0.0375, 0.085);
        const auto total_gap = early_gap + late_gap;
        return -2.23425815 * total_gap * total_gap
            + 0.9428571428571415 * total_gap - 0.01;
    }
};

class ChPrecisionDrumEngine final : public BaseChEngine {
protected:
    [[nodiscard]] double timing_window(double early_gap,
                                       double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.025, 0.04);
        late_gap = std::clamp(late_gap, 0.025, 0.04);
        const auto total_gap = early_gap + late_gap;
        return 2.4961183 * total_gap * total_gap + 0.24961183 * total_gap
            + 0.0065;
    }
};

class BaseFortniteEngine : public Engine {
public:
    [[nodiscard]] int base_note_value() const override { return 36; }
    [[nodiscard]] double burst_size() const override { return 0.0; }
    [[nodiscard]] bool chords_multiply_sustains() const override
    {
        return true;
    }
    [[nodiscard]] bool delayed_multiplier() const override { return true; }
    [[nodiscard]] double early_timing_window(double early_gap,
                                             double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.125;
    }
    [[nodiscard]] bool has_bres() const override { return false; }
    [[nodiscard]] bool has_early_whammy() const override { return false; }
    [[nodiscard]] bool has_unison_bonuses() const override { return false; }
    [[nodiscard]] bool is_rock_band() const override { return false; }
    [[nodiscard]] bool ignore_average_multiplier() const override
    {
        return true;
    }
    [[nodiscard]] double late_timing_window(double early_gap,
                                            double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.125;
    }
    [[nodiscard]] bool merge_uneven_sustains() const override { return true; }
    [[nodiscard]] bool overlaps() const override { return true; }
    [[nodiscard]] bool round_tick_gap() const override { return false; }
    [[nodiscard]] SightRead::Tick snap_gap() const override
    {
        return SightRead::Tick {0};
    }
    [[nodiscard]] SpEngineValues sp_engine_values() const override
    {
        return {.phrase_amount = 0.25,
                .unison_phrase_amount = 0.5,
                .minimum_to_activate = 0.25};
    }
    [[nodiscard]] SpGainMode sp_gain_mode() const override
    {
        return SpGainMode::Beat;
    }
    [[nodiscard]] SpMode sp_mode() const override { return SpMode::OdBeat; }
    [[nodiscard]] double sp_gain_rate() const override { return 0.0; }
    [[nodiscard]] SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundToNearest;
    }
    [[nodiscard]] SustainTicksMetric sustain_ticks_metric() const override
    {
        return SustainTicksMetric::OdBeat;
    }
};

class FortniteGuitarEngine final : public BaseFortniteEngine {
    [[nodiscard]] int max_multiplier() const override { return 4; }
    [[nodiscard]] int sust_points_per_beat() const override { return 12; }
};

class FortniteBassEngine final : public BaseFortniteEngine {
    [[nodiscard]] int max_multiplier() const override { return 6; }
    [[nodiscard]] int sust_points_per_beat() const override { return 12; }
};

class FortniteVocalsEngine final : public BaseFortniteEngine {
    [[nodiscard]] int max_multiplier() const override { return 6; }
    [[nodiscard]] int sust_points_per_beat() const override { return 25; }
};

class BaseHarmonixGhEngine : public Engine {
private:
    static constexpr double FUDGE_EPSILON = 0.0001;

public:
    [[nodiscard]] int base_note_value() const override { return 50; }
    [[nodiscard]] double burst_size() const override { return 0.0; }
    [[nodiscard]] bool chords_multiply_sustains() const override
    {
        return true;
    }
    [[nodiscard]] bool has_bres() const override { return false; }
    [[nodiscard]] bool has_early_whammy() const override { return true; }
    [[nodiscard]] bool has_unison_bonuses() const override { return false; }
    [[nodiscard]] bool ignore_average_multiplier() const override
    {
        return true;
    }
    [[nodiscard]] bool is_rock_band() const override { return false; }
    [[nodiscard]] double late_timing_window(double early_gap,
                                            double late_gap) const override
    {
        (void)early_gap;
        return std::min(0.1, late_gap / (2.0 + FUDGE_EPSILON));
    }
    [[nodiscard]] int max_multiplier() const override { return 4; }
    [[nodiscard]] bool merge_uneven_sustains() const override { return true; }
    [[nodiscard]] bool overlaps() const override { return false; }
    [[nodiscard]] bool round_tick_gap() const override { return false; }
    [[nodiscard]] SightRead::Tick snap_gap() const override
    {
        return SightRead::Tick {2};
    }
    [[nodiscard]] SpEngineValues sp_engine_values() const override
    {
        return {.phrase_amount = 0.25,
                .unison_phrase_amount = 0.5,
                .minimum_to_activate = 0.5};
    }
    [[nodiscard]] SpGainMode sp_gain_mode() const override
    {
        return SpGainMode::Beat;
    }
    [[nodiscard]] double sp_gain_rate() const override { return 0.034; }
    [[nodiscard]] SpMode sp_mode() const override { return SpMode::Measure; }
    [[nodiscard]] int sust_points_per_beat() const override { return 25; }
    [[nodiscard]] SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundToNearest;
    }
    [[nodiscard]] SustainTicksMetric sustain_ticks_metric() const override
    {
        return SustainTicksMetric::Beat;
    }
};

class Gh1Engine final : public BaseHarmonixGhEngine {
private:
    static constexpr double FUDGE_EPSILON = 0.0001;

public:
    [[nodiscard]] bool delayed_multiplier() const override { return true; }
    [[nodiscard]] double early_timing_window(double early_gap,
                                             double late_gap) const override
    {
        (void)late_gap;
        // The division by a number greater than 2 is a fudge so standard
        // doubles are not shown as possible without EHW.
        return std::min(0.1, early_gap / (2.0 + FUDGE_EPSILON));
    }
};

class Gh2Engine final : public BaseHarmonixGhEngine {
public:
    [[nodiscard]] bool delayed_multiplier() const override { return false; }
    [[nodiscard]] double early_timing_window(double early_gap,
                                             double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.1;
    }
};

class Gh3Engine final : public Engine {
private:
    static constexpr double FUDGE_EPSILON = 0.0001;

public:
    [[nodiscard]] int base_note_value() const override { return 50; }
    [[nodiscard]] double burst_size() const override { return 0.0; }
    [[nodiscard]] bool chords_multiply_sustains() const override
    {
        return false;
    }
    [[nodiscard]] bool delayed_multiplier() const override { return false; }
    [[nodiscard]] double early_timing_window(double early_gap,
                                             double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.116;
    }
    [[nodiscard]] bool has_bres() const override { return false; }
    [[nodiscard]] bool has_early_whammy() const override { return false; }
    [[nodiscard]] bool has_unison_bonuses() const override { return false; }
    [[nodiscard]] bool ignore_average_multiplier() const override
    {
        return true;
    }
    [[nodiscard]] bool is_rock_band() const override { return false; }
    [[nodiscard]] double late_timing_window(double early_gap,
                                            double late_gap) const override
    {
        (void)early_gap;
        return std::min(0.1, late_gap / (2.0 + FUDGE_EPSILON));
    }
    [[nodiscard]] int max_multiplier() const override { return 4; }
    [[nodiscard]] bool merge_uneven_sustains() const override { return false; }
    [[nodiscard]] bool overlaps() const override { return false; }
    [[nodiscard]] bool round_tick_gap() const override { return false; }
    [[nodiscard]] SightRead::Tick snap_gap() const override
    {
        return SightRead::Tick {0};
    }
    [[nodiscard]] SpEngineValues sp_engine_values() const override
    {
        return {.phrase_amount = 0.25,
                .unison_phrase_amount = 0.5,
                .minimum_to_activate = 0.5};
    }
    [[nodiscard]] SpGainMode sp_gain_mode() const override
    {
        return SpGainMode::Fretbar;
    }
    [[nodiscard]] double sp_gain_rate() const override { return 0.033332999; }
    [[nodiscard]] SpMode sp_mode() const override { return SpMode::Measure; }
    [[nodiscard]] int sust_points_per_beat() const override { return 25; }
    [[nodiscard]] SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundToNearest;
    }
    [[nodiscard]] SustainTicksMetric sustain_ticks_metric() const override
    {
        return SustainTicksMetric::Fretbar;
    }
};

class BaseRbEngine : public Engine {
protected:
    [[nodiscard]] virtual double base_timing_window() const = 0;

public:
    [[nodiscard]] int base_note_value() const override { return 25; }
    [[nodiscard]] double burst_size() const override { return 0.0; }
    [[nodiscard]] bool chords_multiply_sustains() const override
    {
        return true;
    }
    [[nodiscard]] bool delayed_multiplier() const override { return false; }
    [[nodiscard]] double early_timing_window(double early_gap,
                                             double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return base_timing_window();
    }
    [[nodiscard]] bool has_bres() const override { return true; }
    [[nodiscard]] bool has_early_whammy() const override { return true; }
    [[nodiscard]] bool ignore_average_multiplier() const override
    {
        return true;
    }
    [[nodiscard]] bool is_rock_band() const override { return true; }
    [[nodiscard]] double late_timing_window(double early_gap,
                                            double late_gap) const override
    {
        (void)early_gap;
        return std::min(base_timing_window(), late_gap / 2);
    }
    [[nodiscard]] bool merge_uneven_sustains() const override { return true; }
    [[nodiscard]] bool overlaps() const override { return true; }
    [[nodiscard]] bool round_tick_gap() const override { return false; }
    [[nodiscard]] SightRead::Tick snap_gap() const override
    {
        return SightRead::Tick {2};
    }
    [[nodiscard]] SpEngineValues sp_engine_values() const override
    {
        return {.phrase_amount = 0.251,
                .unison_phrase_amount = 0.501,
                .minimum_to_activate = 0.5};
    }
    [[nodiscard]] SpGainMode sp_gain_mode() const override
    {
        return SpGainMode::Beat;
    }
    [[nodiscard]] double sp_gain_rate() const override { return 0.034; }
    [[nodiscard]] SpMode sp_mode() const override { return SpMode::OdBeat; }
    [[nodiscard]] int sust_points_per_beat() const override { return 12; }
    [[nodiscard]] SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundToNearest;
    }
    [[nodiscard]] SustainTicksMetric sustain_ticks_metric() const override
    {
        return SustainTicksMetric::Beat;
    }
};

class RbEngine final : public BaseRbEngine {
protected:
    [[nodiscard]] double base_timing_window() const override { return 0.1; }

public:
    [[nodiscard]] bool has_unison_bonuses() const override { return false; }
    [[nodiscard]] int max_multiplier() const override { return 4; }
};

class RbBassEngine final : public BaseRbEngine {
protected:
    [[nodiscard]] double base_timing_window() const override { return 0.1; }

public:
    [[nodiscard]] bool has_unison_bonuses() const override { return false; }
    [[nodiscard]] int max_multiplier() const override { return 6; }
};

class Rb3Engine final : public BaseRbEngine {
protected:
    [[nodiscard]] double base_timing_window() const override { return 0.1; }

public:
    [[nodiscard]] bool has_unison_bonuses() const override { return true; }
    [[nodiscard]] int max_multiplier() const override { return 4; }
};

class Rb3BassEngine final : public BaseRbEngine {
protected:
    [[nodiscard]] double base_timing_window() const override { return 0.1; }

public:
    [[nodiscard]] bool has_unison_bonuses() const override { return true; }
    [[nodiscard]] int max_multiplier() const override { return 6; }
};
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

#endif
