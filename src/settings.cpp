/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021 Raymond Wright
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

#include "argparse_wrapper.hpp"

#include "settings.hpp"

static int str_to_int(const std::string& value)
{
    std::size_t pos = 0;
    const auto converted_value = std::stoi(value, &pos);
    if (pos != value.size()) {
        throw std::invalid_argument("Could not convert string to int");
    }
    return converted_value;
}

static float str_to_float(const std::string& value)
{
    std::size_t pos = 0;
    const auto converted_value = std::stof(value, &pos);
    if (pos != value.size()) {
        throw std::invalid_argument("Could not convert string to float");
    }
    return converted_value;
}

static bool is_valid_image_path(const std::string& path)
{
    if (path.size() < 4) {
        return false;
    }
    const auto file_type = path.substr(path.size() - 4, 4);
    if (file_type == ".bmp") {
        return true;
    }
    if (file_type == ".png") {
        return true;
    }
    return false;
}

static Difficulty string_to_diff(std::string_view text)
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

static Instrument string_to_inst(std::string_view text)
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

Settings from_args(int argc, char** argv)
{
    constexpr float DEFAULT_OPACITY = 0.33F;
    constexpr int DEFAULT_SPEED = 100;
    constexpr int MAX_PERCENT = 100;
    constexpr int MAX_SPEED = 5000;
    constexpr int MAX_VIDEO_LAG = 200;
    constexpr int MIN_SPEED = 5;
    constexpr double MS_PER_SECOND = 1000.0;

    argparse::ArgumentParser program {"CHOpt"};

    program.add_argument("-f", "--file")
        .default_value(std::string {"-"})
        .help("chart filename");
    program.add_argument("-o", "--output")
        .default_value(std::string {"path.png"})
        .help("location to save output image (must be a .bmp or .png), "
              "defaults to path.png");
    program.add_argument("-d", "--diff")
        .default_value(std::string {"expert"})
        .help("difficulty, options are easy, medium, hard, expert, defaults to "
              "expert");
    program.add_argument("-i", "--instrument")
        .default_value(std::string {"guitar"})
        .help("instrument, options are guitar, coop, bass, rhythm, keys, ghl, "
              "ghlbass, drums, defaults to guitar");
    program.add_argument("--sqz", "--squeeze")
        .default_value(MAX_PERCENT)
        .help("squeeze% (0 to 100), defaults to 100")
        .action([](const std::string& value) { return str_to_int(value); });
    program.add_argument("--ew", "--early-whammy")
        .default_value(std::string {"match"})
        .help("early whammy% (0 to 100), <= squeeze, defaults to squeeze");
    program.add_argument("--lazy", "--lazy-whammy")
        .default_value(0)
        .help("time before whammying starts on sustains in milliseconds, "
              "defaults to 0")
        .action([](const std::string& value) { return str_to_int(value); });
    program.add_argument("--delay", "--whammy-delay")
        .default_value(0)
        .help("time after an activation ends before whammying can resume, "
              "defaults to 0")
        .action([](const std::string& value) { return str_to_int(value); });
    program.add_argument("--lag", "--video-lag")
        .default_value(0)
        .help("video lag calibration setting in milliseconds, defaults to 0")
        .action([](const std::string& value) { return str_to_int(value); });
    program.add_argument("-s", "--speed")
        .help("speed in %, defaults to 100")
        .default_value(DEFAULT_SPEED)
        .action([](const std::string& value) { return str_to_int(value); });
    program.add_argument("--double-kick")
        .help("enable 2x kick for drum charts")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--no-kick")
        .help("disable single kicks")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--engine")
        .default_value(std::string {"ch"})
        .help("engine, options are ch, rb, and rb3, defaults to ch");
    program.add_argument("-b", "--blank")
        .help("give a blank chart image")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--no-image")
        .help("do not create an image")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--no-bpms")
        .help("do not draw BPMs")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--no-solos")
        .help("do not draw solo sections")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--no-time-sigs")
        .help("do not draw time signatures")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("--act-opacity")
        .help("opacity of drawn activations (0.0 to 1.0), defaults to 0.33")
        .default_value(DEFAULT_OPACITY)
        .action([](const std::string& value) { return str_to_float(value); });

    program.parse_args(argc, argv);

    Settings settings;

    settings.blank = program.get<bool>("--blank");
    settings.filename = program.get<std::string>("--file");
    if (settings.filename == "-") {
        throw std::invalid_argument("No file was specified");
    }

    settings.difficulty = string_to_diff(program.get<std::string>("--diff"));
    settings.instrument
        = string_to_inst(program.get<std::string>("--instrument"));

    const auto image_path = program.get<std::string>("--output");
    if (!is_valid_image_path(image_path)) {
        throw std::invalid_argument(
            "Image output must be a bitmap or png (.bmp / .png)");
    }
    settings.image_path = image_path;

    settings.draw_image = !program.get<bool>("--no-image");
    settings.draw_bpms = !program.get<bool>("--no-bpms");
    settings.draw_solos = !program.get<bool>("--no-solos");
    settings.draw_time_sigs = !program.get<bool>("--no-time-sigs");
    settings.enable_double_kick = program.get<bool>("--double-kick");
    settings.disable_kick = program.get<bool>("--no-kick");

    const auto squeeze = program.get<int>("--squeeze");
    auto early_whammy = squeeze;
    if (program.get<std::string>("--early-whammy") != "match") {
        early_whammy = str_to_int(program.get<std::string>("--early-whammy"));
    }
    const auto lazy_whammy = program.get<int>("--lazy-whammy");
    const auto whammy_delay = program.get<int>("--whammy-delay");

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

    settings.squeeze = squeeze / 100.0;
    settings.early_whammy = early_whammy / 100.0;
    settings.lazy_whammy = lazy_whammy / MS_PER_SECOND;
    settings.whammy_delay = whammy_delay / MS_PER_SECOND;

    const auto video_lag = program.get<int>("--video-lag");
    if (video_lag < -MAX_VIDEO_LAG || video_lag > MAX_VIDEO_LAG) {
        throw std::invalid_argument(
            "Video lag setting unsupported by Clone Hero");
    }

    settings.video_lag = video_lag / MS_PER_SECOND;

    const auto engine_name = program.get<std::string>("--engine");
    if (engine_name == "ch") {
        settings.engine = std::make_unique<ChEngine>();
    } else if (engine_name == "rb") {
        if (settings.instrument == Instrument::Bass) {
            settings.engine = std::make_unique<RbBassEngine>();
        } else {
            settings.engine = std::make_unique<RbEngine>();
        }
    } else if (engine_name == "rb3") {
        if (settings.instrument == Instrument::Bass) {
            settings.engine = std::make_unique<Rb3BassEngine>();
        } else {
            settings.engine = std::make_unique<Rb3Engine>();
        }
    } else {
        throw std::invalid_argument("Invalid engine specified");
    }

    const auto speed = program.get<int>("--speed");
    if (speed < MIN_SPEED || speed > MAX_SPEED || speed % MIN_SPEED != 0) {
        throw std::invalid_argument("Speed unsupported by Clone Hero");
    }

    settings.speed = speed;

    const auto opacity = program.get<float>("--act-opacity");
    if (opacity < 0.0F || opacity > 1.0F) {
        throw std::invalid_argument(
            "Activation opacity should lie between 0.0 and 1.0");
    }

    settings.opacity = opacity;

    return settings;
}
