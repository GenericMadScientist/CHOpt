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

#include <stdexcept>

#include "argparse_wrapper.hpp"

#include "settings.hpp"

static int str_to_int(const std::string& value)
{
    std::size_t pos = 0;
    auto converted_value = std::stoi(value, &pos);
    if (pos != value.size()) {
        throw std::invalid_argument("Could not convert string to int");
    }
    return converted_value;
}

Settings from_args(int argc, char** argv)
{
    constexpr int MAX_PERCENT = 100;

    argparse::ArgumentParser program {"chopt"};

    program.add_argument("-f", "--file")
        .default_value(std::string {"-"})
        .help("chart filename");
    program.add_argument("-o", "--output")
        .default_value(std::string {"path.bmp"})
        .help("location to save output image (must be a .bmp), defaults to "
              "path.bmp");
    program.add_argument("-d", "--diff")
        .default_value(std::string {"expert"})
        .help("difficulty, defaults to expert");
    program.add_argument("--sqz", "--squeeze")
        .default_value(MAX_PERCENT)
        .help("squeeze% (0 to 100), defaults to 100")
        .action([](const std::string& value) { return str_to_int(value); });
    program.add_argument("--ew", "--early-whammy")
        .default_value(std::string {"match"})
        .help("early whammy% (0 to 100), <= squeeze, defaults to squeeze");
    program.add_argument("-b", "--blank")
        .help("give a blank chart image")
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
        .help("do not time signatures")
        .default_value(false)
        .implicit_value(true);

    program.parse_args(argc, argv);

    Settings settings;

    settings.blank = program.get<bool>("--blank");
    settings.filename = program.get<std::string>("--file");
    if (settings.filename == "-") {
        throw std::invalid_argument("No file was specified");
    }

    auto diff_string = program.get<std::string>("--diff");
    if (diff_string == "expert") {
        settings.difficulty = Difficulty::Expert;
    } else if (diff_string == "hard") {
        settings.difficulty = Difficulty::Hard;
    } else if (diff_string == "medium") {
        settings.difficulty = Difficulty::Medium;
    } else if (diff_string == "easy") {
        settings.difficulty = Difficulty::Easy;
    } else {
        throw std::invalid_argument("Unrecognised difficulty");
    }

    auto image_path = program.get<std::string>("--output");
    if (image_path.size() < 4
        || image_path.substr(image_path.size() - 4, 4) != ".bmp") {
        throw std::invalid_argument("Image output must be a bitmap (.bmp)");
    }
    settings.image_path = image_path;

    settings.draw_bpms = !program.get<bool>("--no-bpms");
    settings.draw_solos = !program.get<bool>("--no-solos");
    settings.draw_time_sigs = !program.get<bool>("--no-time-sigs");

    auto squeeze = program.get<int>("--squeeze");
    auto early_whammy = squeeze;
    if (program.get<std::string>("--early-whammy") != "match") {
        early_whammy = str_to_int(program.get<std::string>("--early-whammy"));
    }

    if (squeeze < 0 || squeeze > MAX_PERCENT) {
        throw std::invalid_argument("Squeeze must lie between 0 and 100");
    }
    if (early_whammy < 0 || early_whammy > MAX_PERCENT) {
        throw std::invalid_argument("Early whammy must lie between 0 and 100");
    }
    if (early_whammy > squeeze) {
        throw std::invalid_argument(
            "Early whammy cannot be higher than squeeze");
    }

    settings.squeeze = squeeze / 100.0;
    settings.early_whammy = early_whammy / 100.0;

    return settings;
}
