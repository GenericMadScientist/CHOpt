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
#include "image.hpp"
#include "optimiser.hpp"
#include "settings.hpp"

int main(int argc, char** argv)
{
    try {
        const auto settings = from_args(argc, argv);

        if (settings.help) {
            std::cout << settings.help_message << std::endl;
            return EXIT_SUCCESS;
        }

        std::ifstream in(settings.filename);
        if (!in.is_open()) {
            std::cerr << "File did not open, please specify a valid file!"
                      << std::endl;
            return EXIT_FAILURE;
        }
        std::string contents {std::istreambuf_iterator<char>(in),
                              std::istreambuf_iterator<char>()};
        const auto chart = Chart::parse_chart(contents);
        const auto& track = chart.note_track(settings.difficulty);
        DrawingInstructions instructions {track, chart.resolution(),
                                          chart.sync_track()};
        instructions.add_sp_phrases(track, chart.resolution());

        if (!settings.blank) {
            const ProcessedSong processed_track {
                track, chart.resolution(), chart.sync_track(),
                settings.early_whammy, settings.squeeze};
            const Optimiser optimiser {&processed_track};
            const auto path = optimiser.optimal_path();
            instructions.add_sp_acts(path);
            std::cout << processed_track.path_summary(path) << std::endl;
        }
        const Image image {instructions};
        image.save(settings.image_path.c_str());
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unexpected non-exception error!" << std::endl;
        return EXIT_FAILURE;
    }
}
