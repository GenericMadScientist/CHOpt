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

#include "cxxopts_wrapper.hpp"

#include "chart.hpp"
#include "optimiser.hpp"

int main(int argc, char* argv[])
{
    try {
        cxxopts::Options options("chopt",
                                 "Star Power optimiser for Clone Hero");

        options.add_options()(
            "d,diff", "Difficulty",
            cxxopts::value<std::string>()->default_value("expert"))(
            "f,file", "Chart filename",
            cxxopts::value<std::string>())("h,help", "Print usage");

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
        const auto processed_track
            = ProcessedTrack(track, chart.resolution(), chart.sync_track());
        const auto path = processed_track.optimal_path();
        std::cout << "Number of activations: " << path.activations.size()
                  << '\n';
        for (const auto& act : path.activations) {
            std::cout << "Beat " << act.act_start->position.beat.value()
                      << " to Beat " << act.act_end->position.beat.value()
                      << '\n';
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unexpected non-exception error!" << std::endl;
        return EXIT_FAILURE;
    }
}
