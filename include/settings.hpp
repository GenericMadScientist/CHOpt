/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024, 2025 Raymond Wright
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

#ifndef CHOPT_SETTINGS_HPP
#define CHOPT_SETTINGS_HPP

#include <memory>
#include <set>
#include <string>

#include <QStringList>

#include <sightread/drumsettings.hpp>
#include <sightread/songparts.hpp>
#include <sightread/time.hpp>

#include "engine.hpp"

enum class Game {
    CloneHero,
    FortniteFestival,
    GuitarHeroOne,
    RockBand,
    RockBandThree
};

std::unique_ptr<Engine> game_to_engine(Game game,
                                       SightRead::Instrument instrument,
                                       bool precision_mode);

struct SqueezeSettings {
    SightRead::Second lazy_whammy {0.0};
    SightRead::Second video_lag {0.0};
    SightRead::Second whammy_delay {0.0};

    static SqueezeSettings default_settings()
    {
        return {SightRead::Second(0.0), SightRead::Second(0.0),
                SightRead::Second(0.0)};
    }
};

struct PathingSettings {
    std::unique_ptr<Engine> engine;
    double squeeze;
    double early_whammy;
    SightRead::DrumSettings drum_settings;
    SqueezeSettings squeeze_settings;
};

// This struct represents the options chosen on the command line by the user.
struct Settings {
    bool blank;
    std::string filename;
    std::string image_path;
    bool draw_image;
    bool draw_bpms;
    bool draw_solos;
    bool draw_time_sigs;
    SightRead::Difficulty difficulty;
    SightRead::Instrument instrument;
    int speed;
    bool is_lefty_flip;
    Game game;
    PathingSettings pathing_settings;
    float opacity;
};

// Parses the command line options.
Settings from_args(const QStringList& args);

#endif
