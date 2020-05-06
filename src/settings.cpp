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

#include "cxxopts_wrapper.hpp"

#include "settings.hpp"

Settings from_args(int argc, char** argv)
{
    constexpr int MAX_PERCENT = 100;

    cxxopts::Options options("chopt", "Star Power optimiser for Clone Hero");

    auto adder = options.add_options();
    adder("b,blank", "Give a blank chart image");
    adder("d,diff", "Difficulty",
          cxxopts::value<std::string>()->default_value("expert"));
    adder("f,file", "Chart filename", cxxopts::value<std::string>());
    adder("h,help", "Print usage");
    adder("o,output", "Location to save output image (must be a .bmp)",
          cxxopts::value<std::string>()->default_value("path.bmp"));
    adder("no-bpms", "Do not draw BPMs");
    adder("no-solos", "Do not draw solo sections");
    adder("squeeze", "Squeeze% (0 to 100)",
          cxxopts::value<int>()->default_value("100"));
    adder("early-whammy", "Early whammy% (0 to 100), <= squeeze",
          cxxopts::value<int>()->default_value("100"));

    auto result = options.parse(argc, argv);

    Settings settings;
    settings.help = result["help"].as<bool>();
    settings.help_message = options.help();

    if (settings.help) {
        return settings;
    }

    settings.blank = result["blank"].as<bool>();
    if (result.count("file") != 0) {
        settings.filename = result["file"].as<std::string>();
    } else {
        throw std::invalid_argument("No file was specified");
    }

    auto diff_string = result["diff"].as<std::string>();
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

    auto image_path = result["output"].as<std::string>();
    if (image_path.size() < 4
        || image_path.substr(image_path.size() - 4, 4) != ".bmp") {
        throw std::invalid_argument("Image output must be a bitmap (.bmp)");
    }
    settings.image_path = image_path;

    settings.draw_bpms = !result["no-bpms"].as<bool>();
    settings.draw_solos = !result["no-solos"].as<bool>();

    auto squeeze = result["squeeze"].as<int>();
    auto early_whammy = result["early-whammy"].as<int>();

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
