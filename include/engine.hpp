/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2021 Raymond Wright
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

class Engine {
public:
    virtual int base_note_value() const = 0;
    virtual double burst_size() const = 0;
    virtual bool do_chords_multiply_sustains() const = 0;
    virtual int sust_points_per_beat() const = 0;
    virtual ~Engine() = default;
};

class ChEngine : public Engine {
public:
    int base_note_value() const override { return 50; }
    double burst_size() const override { return 0.25; }
    bool do_chords_multiply_sustains() const override { return false; }
    int sust_points_per_beat() const override { return 25; }
};

class RbEngine : public Engine {
public:
    int base_note_value() const override { return 25; }
    double burst_size() const override { return 0.0; }
    bool do_chords_multiply_sustains() const override { return true; }
    int sust_points_per_beat() const override { return 12; }
};

#endif
