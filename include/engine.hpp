/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2021, 2022, 2023 Raymond Wright
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

enum class SustainRoundingPolicy { RoundUp, RoundToNearest };

class Engine {
public:
    virtual int base_note_value() const = 0;
    virtual double burst_size() const = 0;
    virtual bool chords_multiply_sustains() const = 0;
    virtual bool delayed_multiplier() const = 0;
    virtual double early_timing_window(double early_gap, double late_gap) const
        = 0;
    virtual bool has_bres() const = 0;
    virtual bool has_unison_bonuses() const = 0;
    virtual bool is_rock_band() const = 0;
    virtual bool ignore_average_multiplier() const = 0;
    virtual double late_timing_window(double early_gap, double late_gap) const
        = 0;
    virtual int max_multiplier() const = 0;
    virtual bool merge_uneven_sustains() const = 0;
    virtual bool overlaps() const = 0;
    virtual bool round_tick_gap() const = 0;
    virtual int snap_gap() const = 0;
    virtual double sp_gain_rate() const = 0;
    virtual int sust_points_per_beat() const = 0;
    virtual SustainRoundingPolicy sustain_rounding() const = 0;
    virtual bool uses_beat_track() const = 0;
    virtual ~Engine() = default;
};

class BaseChEngine : public Engine {
public:
    int base_note_value() const override { return 50; }
    double burst_size() const override { return 0.25; }
    bool chords_multiply_sustains() const override { return false; }
    bool delayed_multiplier() const override { return false; }
    bool has_bres() const override { return false; }
    bool has_unison_bonuses() const override { return false; }
    bool ignore_average_multiplier() const override { return false; }
    bool is_rock_band() const override { return false; }
    int max_multiplier() const override { return 4; }
    bool merge_uneven_sustains() const override { return false; }
    bool overlaps() const override { return true; }
    bool round_tick_gap() const override { return true; }
    int snap_gap() const override { return 0; }
    double sp_gain_rate() const override { return 1 / 30.0; }
    int sust_points_per_beat() const override { return 25; }
    SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundUp;
    }
    bool uses_beat_track() const override { return false; }
};

class ChGuitarEngine final : public BaseChEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.07;
    }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.07;
    }
};

class ChPrecisionGuitarEngine final : public BaseChEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.0, 0.0525);
        late_gap = std::clamp(late_gap, 0.0, 0.0525);
        const auto total_gap = early_gap + late_gap;
        return 0.27619 * total_gap + 0.021;
    }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.0, 0.0525);
        late_gap = std::clamp(late_gap, 0.0, 0.0525);
        const auto total_gap = early_gap + late_gap;
        return 0.27619 * total_gap + 0.021;
    }
};

class ChDrumEngine final : public BaseChEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.0375, 0.085);
        late_gap = std::clamp(late_gap, 0.0375, 0.085);
        const auto total_gap = early_gap + late_gap;
        return -2.23425815 * total_gap * total_gap
            + 0.9428571428571415 * total_gap - 0.01;
    }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.0375, 0.085);
        late_gap = std::clamp(late_gap, 0.0375, 0.085);
        const auto total_gap = early_gap + late_gap;
        return -2.23425815 * total_gap * total_gap
            + 0.9428571428571415 * total_gap - 0.01;
    }
};

class ChPrecisionDrumEngine final : public BaseChEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.025, 0.04);
        late_gap = std::clamp(late_gap, 0.025, 0.04);
        const auto total_gap = early_gap + late_gap;
        return 2.4961183 * total_gap * total_gap + 0.24961183 * total_gap
            + 0.0065;
    }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        early_gap = std::clamp(early_gap, 0.025, 0.04);
        late_gap = std::clamp(late_gap, 0.025, 0.04);
        const auto total_gap = early_gap + late_gap;
        return 2.4961183 * total_gap * total_gap + 0.24961183 * total_gap
            + 0.0065;
    }
};

class Gh1Engine : public Engine {
private:
    static constexpr double FUDGE_EPSILON = 0.0001;

public:
    int base_note_value() const override { return 50; }
    double burst_size() const override { return 0.0; }
    bool chords_multiply_sustains() const override { return true; }
    bool delayed_multiplier() const override { return true; }
    double early_timing_window(double early_gap, double late_gap) const override
    {
        (void)late_gap;
        // The division by a number greater than 2 is a fudge so standard
        // doubles are not shown as possible without EHW.
        return std::min(0.1, early_gap / (2.0 + FUDGE_EPSILON));
    }
    bool has_bres() const override { return false; }
    bool has_unison_bonuses() const override { return false; }
    bool ignore_average_multiplier() const override { return true; }
    bool is_rock_band() const override { return false; }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        return std::min(0.1, late_gap / (2.0 + FUDGE_EPSILON));
    }
    int max_multiplier() const override { return 4; }
    bool merge_uneven_sustains() const override { return true; }
    bool overlaps() const override { return false; }
    bool round_tick_gap() const override { return false; }
    int snap_gap() const override { return 2; }
    double sp_gain_rate() const override { return 0.034; }
    int sust_points_per_beat() const override { return 25; }
    SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundToNearest;
    }
    bool uses_beat_track() const override { return false; }
};

class BaseRbEngine : public Engine {
public:
    int base_note_value() const override { return 25; }
    double burst_size() const override { return 0.0; }
    bool chords_multiply_sustains() const override { return true; }
    bool delayed_multiplier() const override { return false; }
    bool has_bres() const override { return true; }
    bool ignore_average_multiplier() const override { return true; }
    bool is_rock_band() const override { return true; }
    bool merge_uneven_sustains() const override { return true; }
    bool overlaps() const override { return true; }
    bool round_tick_gap() const override { return false; }
    int snap_gap() const override { return 2; }
    double sp_gain_rate() const override { return 0.034; }
    int sust_points_per_beat() const override { return 12; }
    SustainRoundingPolicy sustain_rounding() const override
    {
        return SustainRoundingPolicy::RoundToNearest;
    }
    bool uses_beat_track() const override { return true; }
};

class RbEngine final : public BaseRbEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.1;
    }
    bool has_unison_bonuses() const override { return false; }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        return std::min(0.1, late_gap / 2);
    }
    int max_multiplier() const override { return 4; }
};

class RbBassEngine final : public BaseRbEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.1;
    }
    bool has_unison_bonuses() const override { return false; }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        return std::min(0.1, late_gap / 2);
    }
    int max_multiplier() const override { return 6; }
};

class Rb3Engine final : public BaseRbEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.105;
    }
    bool has_unison_bonuses() const override { return true; }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        return std::min(0.105, late_gap / 2);
    }
    int max_multiplier() const override { return 4; }
};

class Rb3BassEngine final : public BaseRbEngine {
public:
    double early_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        (void)late_gap;
        return 0.105;
    }
    bool has_unison_bonuses() const override { return true; }
    double late_timing_window(double early_gap, double late_gap) const override
    {
        (void)early_gap;
        return std::min(0.105, late_gap / 2);
    }
    int max_multiplier() const override { return 6; }
};

#endif
