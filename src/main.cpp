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

#include "chart.hpp"
#include "optimiser.hpp"

int main(int argc, char* argv[])
{
    try {
        if (argc < 2) {
            std::cerr << "Please specify a file!\n";
            return EXIT_FAILURE;
        }
        // clang-tidy complains about the following pointer arithmetic, but
        // using gsl::span just for this is overkill.
        std::ifstream in(argv[1]); // NOLINT
        if (!in.is_open()) {
            std::cerr << "File did not open, please specify a valid file!\n";
            return EXIT_FAILURE;
        }
        std::string contents((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
        const auto chart = Chart::parse_chart(contents);
        const auto& track = chart.note_track(Difficulty::Expert);
        const auto processed_track
            = ProcessedTrack(track, chart.header(), chart.sync_track());
        const auto path = processed_track.optimal_path();
        std::cout << "Number of activations: " << path.activations.size()
                  << '\n';
        for (const auto& act : path.activations) {
            std::cout << "Beat " << act.act_start->beat_position.value()
                      << " to Beat " << act.act_end->beat_position.value()
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
