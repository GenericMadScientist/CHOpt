/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include <cstdint>
#include <stdexcept>

#include <boost/nowide/iostream.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "settings.hpp"

namespace po = boost::program_options;

namespace {
bool is_valid_image_path(std::string_view path)
{
    return path.ends_with(".bmp") || path.ends_with(".png");
}

Difficulty string_to_diff(std::string_view text)
{
    if (text == "expert") {
        return Difficulty::Expert;
    }
    if (text == "hard") {
        return Difficulty::Hard;
    }
    if (text == "medium") {
        return Difficulty::Medium;
    }
    if (text == "easy") {
        return Difficulty::Easy;
    }
    throw std::invalid_argument("Unrecognised difficulty");
}

Instrument string_to_inst(std::string_view text)
{
    if (text == "guitar") {
        return Instrument::Guitar;
    }
    if (text == "coop") {
        return Instrument::GuitarCoop;
    }
    if (text == "bass") {
        return Instrument::Bass;
    }
    if (text == "rhythm") {
        return Instrument::Rhythm;
    }
    if (text == "keys") {
        return Instrument::Keys;
    }
    if (text == "ghl") {
        return Instrument::GHLGuitar;
    }
    if (text == "ghlbass") {
        return Instrument::GHLBass;
    }
    if (text == "drums") {
        return Instrument::Drums;
    }
    throw std::invalid_argument("Unrecognised instrument");
}

Game game_from_string(std::string_view game)
{
    const std::map<std::string_view, Game> game_map {
        {"ch", Game::CloneHero},
        {"gh1", Game::GuitarHeroOne},
        {"rb", Game::RockBand},
        {"rb3", Game::RockBandThree}};
    return game_map.at(game);
}
}

std::unique_ptr<Engine> game_to_engine(Game game, Instrument instrument,
                                       bool precision_mode)
{
    switch (game) {
    case Game::CloneHero:
        if (instrument == Instrument::Drums) {
            if (precision_mode) {
                return std::make_unique<ChPrecisionDrumEngine>();
            }
            return std::make_unique<ChDrumEngine>();
        }
        if (precision_mode) {
            return std::make_unique<ChPrecisionGuitarEngine>();
        }
        return std::make_unique<ChGuitarEngine>();
    case Game::GuitarHeroOne:
        return std::make_unique<Gh1Engine>();
    case Game::RockBand:
        if (instrument == Instrument::Bass) {
            return std::make_unique<RbBassEngine>();
        }
        return std::make_unique<RbEngine>();
    case Game::RockBandThree:
        if (instrument == Instrument::Bass) {
            return std::make_unique<Rb3BassEngine>();
        }
        return std::make_unique<Rb3Engine>();
    }

    throw std::invalid_argument("Invalid Game");
}

std::optional<Settings> from_args(int argc, char** argv)
{
    constexpr float DEFAULT_OPACITY = 0.33F;
    constexpr int DEFAULT_SPEED = 100;
    constexpr int MAX_PERCENT = 100;
    constexpr int MAX_SPEED = 5000;
    constexpr int MAX_VIDEO_LAG = 200;
    constexpr int MIN_SPEED = 5;
    constexpr double MS_PER_SECOND = 1000.0;

    po::options_description desc {"Allowed options"};

    auto add_option = desc.add_options();
    add_option("help,h", "produce help message");
    add_option("file,f", po::value<std::string>(), "chart filename");
    add_option("output,o", po::value<std::string>()->default_value("path.png"),
               "location to save output image (must be a .bmp or .png)");
    add_option("diff,d", po::value<std::string>()->default_value("expert"),
               "difficulty, options are easy, medium, hard, expert");
    add_option("instrument,i",
               po::value<std::string>()->default_value("guitar"),
               "instrument, options are guitar, coop, bass, rhythm, keys, ghl, "
               "ghlbass, drums");
    add_option("squeeze,sqz", po::value<int>()->default_value(MAX_PERCENT),
               "squeeze% (0 to 100)");
    add_option("early-whammy,ew", po::value<int>(),
               "early whammy% (0 to 100), <= squeeze, defaults to squeeze");
    add_option("lazy-whammy,lazy", po::value<int>()->default_value(0),
               "time before whammying starts on sustains in milliseconds");
    add_option("whammy-delay,delay", po::value<int>()->default_value(0),
               "time after an activation ends before whammying can resume");
    add_option("video-lag,lag", po::value<int>()->default_value(0),
               "video lag calibration setting in milliseconds");
    add_option("speed,s", po::value<int>()->default_value(DEFAULT_SPEED),
               "speed in %");
    add_option("lefty-flip,l", "draw with lefty flip");
    add_option("no-double-kick", "disable 2x kick for drum charts");
    add_option("no-kick", "disable single kicks for drum charts");
    add_option("no-pro-drums", "disable pro drums");
    add_option("enable-dynamics",
               "enable double points for ghost and accented notes");
    add_option("engine", po::value<std::string>()->default_value("ch"),
               "engine, options are ch, gh1, rb, and rb3");
    add_option("precision-mode,p", "turn on precision mode for CH");
    add_option("blank,b", "give a blank chart image");
    add_option("no-image", "do not create an image");
    add_option("no-bpms", "do not draw BPMs");
    add_option("no-solos", "do not draw solo sections");
    add_option("no-time-sigs", "do not draw time signatures");
    add_option("act-opacity",
               po::value<float>()->default_value(DEFAULT_OPACITY),
               "opacity of drawn activations (0.0 to 1.0)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help") != 0) {
        boost::nowide::cout << desc << '\n';
        return {};
    }

    Settings settings;

    settings.blank = vm.count("blank") != 0;
    if (vm.count("file") == 0) {
        throw std::invalid_argument("No file was specified");
    }
    settings.filename = vm["file"].as<std::string>();

    settings.difficulty = string_to_diff(vm["diff"].as<std::string>());
    settings.instrument = string_to_inst(vm["instrument"].as<std::string>());

    settings.image_path = vm["output"].as<std::string>();
    if (!is_valid_image_path(settings.image_path)) {
        throw std::invalid_argument(
            "Image output must be a bitmap or png (.bmp / .png)");
    }

    settings.is_lefty_flip = vm.count("lefty-flip") != 0;
    settings.draw_image = vm.count("no-image") == 0;
    settings.draw_bpms = vm.count("no-bpms") == 0;
    settings.draw_solos = vm.count("no-solos") == 0;
    settings.draw_time_sigs = vm.count("no-time-sigs") == 0;
    settings.drum_settings.enable_double_kick = vm.count("no-double-kick") == 0;
    settings.drum_settings.disable_kick = vm.count("no-kick") != 0;
    settings.drum_settings.pro_drums = vm.count("no-pro-drums") == 0;
    settings.drum_settings.enable_dynamics = vm.count("enable-dynamics") != 0;

    const auto squeeze = vm["squeeze"].as<int>();
    auto early_whammy = squeeze;
    if (vm.count("early-whammy") != 0) {
        early_whammy = vm["early-whammy"].as<int>();
    }
    const auto lazy_whammy = vm["lazy-whammy"].as<int>();
    const auto whammy_delay = vm["whammy-delay"].as<int>();

    if (squeeze < 0 || squeeze > MAX_PERCENT) {
        throw std::invalid_argument("Squeeze must lie between 0 and 100");
    }
    if (early_whammy < 0 || early_whammy > MAX_PERCENT) {
        throw std::invalid_argument("Early whammy must lie between 0 and 100");
    }
    if (lazy_whammy < 0) {
        throw std::invalid_argument(
            "Lazy whammy must be greater than or equal to 0");
    }
    if (whammy_delay < 0) {
        throw std::invalid_argument(
            "Whammy delay must be greater than or equal to 0");
    }

    settings.squeeze_settings.squeeze = squeeze / 100.0;
    settings.squeeze_settings.early_whammy = early_whammy / 100.0;
    settings.squeeze_settings.lazy_whammy
        = Second {lazy_whammy / MS_PER_SECOND};
    settings.squeeze_settings.whammy_delay
        = Second {whammy_delay / MS_PER_SECOND};

    const auto video_lag = vm["video-lag"].as<int>();
    if (video_lag < -MAX_VIDEO_LAG || video_lag > MAX_VIDEO_LAG) {
        throw std::invalid_argument(
            "Video lag setting unsupported by Clone Hero");
    }

    settings.squeeze_settings.video_lag = Second {video_lag / MS_PER_SECOND};

    const auto precision_mode = vm.count("precision-mode") != 0;
    const auto engine_name = vm["engine"].as<std::string>();
    settings.game = game_from_string(engine_name);
    settings.engine
        = game_to_engine(settings.game, settings.instrument, precision_mode);

    const auto speed = vm["speed"].as<int>();
    if (speed < MIN_SPEED || speed > MAX_SPEED || speed % MIN_SPEED != 0) {
        throw std::invalid_argument("Speed unsupported by Clone Hero");
    }

    settings.speed = speed;

    const auto opacity = vm["act-opacity"].as<float>();
    if (opacity < 0.0F || opacity > 1.0F) {
        throw std::invalid_argument(
            "Activation opacity should lie between 0.0 and 1.0");
    }

    settings.opacity = opacity;

    return settings;
}
