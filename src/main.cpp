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

#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <stdexcept>
#include <string>

#include "nowide_wrapper.hpp"

#include "image.hpp"
#include "optimiser.hpp"
#include "settings.hpp"
#include "song.hpp"
#include "time.hpp"

int main(int argc, char** argv)
{
    try {
        nowide::args a(argc, argv);
        const auto settings = from_args(argc, argv);
        const auto song = Song::from_filename(settings.filename);
        const auto instruments = song.instruments();
        if (std::find(instruments.cbegin(), instruments.cend(),
                      settings.instrument)
            == instruments.cend()) {
            throw std::invalid_argument(
                "Chosen instrument not present in song");
        }
        const auto difficulties = song.difficulties(settings.instrument);
        if (std::find(difficulties.cbegin(), difficulties.cend(),
                      settings.difficulty)
            == difficulties.cend()) {
            throw std::invalid_argument(
                "Difficulty not available for chosen instrument");
        }
        const std::atomic<bool> terminate {false};
        const auto builder = make_builder(
            song, settings, [&](auto p) { nowide::cout << p << std::endl; },
            &terminate);
        const Image image {builder};
        image.save(settings.image_path.c_str());
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        nowide::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        nowide::cerr << "Unexpected non-exception error!" << std::endl;
        return EXIT_FAILURE;
    }
}
