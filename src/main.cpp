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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "cimg_wrapper.hpp"
#include "cxxopts_wrapper.hpp"

#include "chart.hpp"
#include "image.hpp"
#include "optimiser.hpp"

int main(int argc, char* argv[])
{
    constexpr int MAX_PERCENT = 100;

    try {
        cxxopts::Options options("chopt",
                                 "Star Power optimiser for Clone Hero");

        options.add_options()("b,blank", "Give a blank chart image")(
            "d,diff", "Difficulty",
            cxxopts::value<std::string>()->default_value("expert"))(
            "f,file", "Chart filename", cxxopts::value<std::string>())(
            "squeeze", "Squeeze% (0 to 100)",
            cxxopts::value<int>()->default_value("100"))(
            "early-whammy", "Early whammy% (0 to 100), <= squeeze",
            cxxopts::value<int>()->default_value("100"))(
            "o,output", "Location to save output image (must be a .bmp)",
            cxxopts::value<std::string>()->default_value("path.bmp"))(
            "h,help", "Print usage");

        auto result = options.parse(argc, argv);

        if (result.count("help") != 0) {
            std::cout << options.help() << std::endl;
            return EXIT_SUCCESS;
        }
        if (result.count("file") == 0) {
            std::cerr << "Please specify a file!" << std::endl;
            return EXIT_FAILURE;
        }

        auto filename = result["file"].as<std::string>();
        auto diff_name = result["diff"].as<std::string>();
        auto output_path = result["output"].as<std::string>();

        if (output_path.size() < 4
            || output_path.substr(output_path.size() - 4, 4) != ".bmp") {
            std::cerr << "Image output must be a bitmap (.bmp)!" << std::endl;
            return EXIT_FAILURE;
        }

        auto difficulty = Difficulty::Expert;
        if (diff_name == "expert") {
            difficulty = Difficulty::Expert;
        } else if (diff_name == "hard") {
            difficulty = Difficulty::Hard;
        } else if (diff_name == "medium") {
            difficulty = Difficulty::Medium;
        } else if (diff_name == "easy") {
            difficulty = Difficulty::Easy;
        } else {
            std::cerr << "Unrecognised difficulty " << diff_name << std::endl;
            return EXIT_FAILURE;
        }

        auto squeeze = result["squeeze"].as<int>();
        auto early_whammy = result["early-whammy"].as<int>();
        if (squeeze < 0 || squeeze > MAX_PERCENT) {
            std::cerr << "Squeeze must lie between 0 and 100" << std::endl;
            return EXIT_FAILURE;
        }
        if (early_whammy < 0 || early_whammy > MAX_PERCENT) {
            std::cerr << "Early whammy must lie between 0 and 100" << std::endl;
            return EXIT_FAILURE;
        }
        if (early_whammy > squeeze) {
            std::cerr << "Early whammy cannot be higher than squeeze"
                      << std::endl;
            return EXIT_FAILURE;
        }

        std::ifstream in(filename);
        if (!in.is_open()) {
            std::cerr << "File did not open, please specify a valid file!"
                      << std::endl;
            return EXIT_FAILURE;
        }
        std::string contents {std::istreambuf_iterator<char>(in),
                              std::istreambuf_iterator<char>()};
        const auto chart = Chart::parse_chart(contents);
        const auto& track = chart.note_track(difficulty);
        auto instructions = create_instructions(track, chart.resolution(),
                                                chart.sync_track());

        if (!result["blank"].as<bool>()) {
            const auto processed_track
                = ProcessedTrack(track, chart.resolution(), chart.sync_track());
            const auto path = processed_track.optimal_path();
            for (const auto& act : path.activations) {
                auto start = act.act_start->position.beat.value();
                auto end = act.act_end->position.beat.value();
                instructions.blue_ranges.emplace_back(start, end);
            }
            std::cout << processed_track.path_summary(path) << std::endl;
        }
        const auto image = create_path_image(instructions);
        image.save(output_path.c_str());
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unexpected non-exception error!" << std::endl;
        return EXIT_FAILURE;
    }
}
