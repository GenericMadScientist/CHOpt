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
#include <string>

#include "chart.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Please specify a file!" << std::endl;
        return EXIT_FAILURE;
    }
    // clang-tidy complains about the following pointer arithmetic, but using
    // gsl::span just for this is overkill.
    std::ifstream in(argv[1]); // NOLINT
    std::string contents((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    std::cout << contents.size() << std::endl;
    Chart chart(contents);
    (void)chart;
    return EXIT_SUCCESS;
}
