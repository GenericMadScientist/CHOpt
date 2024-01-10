/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024 Raymond Wright
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

#include <atomic>
#include <cstdio>
#include <exception>

#include <QCoreApplication>
#include <QTextStream>

#include <sightread/time.hpp>

#include "image.hpp"
#include "optimiser.hpp"
#include "settings.hpp"
#include "songfile.hpp"

int main(int argc, char** argv)
{
    QTextStream q_stdout(stdout);
    QTextStream q_stderr(stderr);

    try {
        QCoreApplication app {argc, argv};
        QCoreApplication::setApplicationName("CHOpt");
        QCoreApplication::setApplicationVersion("1.8.1");

        const auto settings = from_args(QCoreApplication::arguments());
        const SongFile song_file {settings.filename};
        auto song = song_file.load_song(settings.game);
        const auto& track
            = song.track(settings.instrument, settings.difficulty);
        const std::atomic<bool> terminate {false};
        const auto builder = make_builder(
            song, track, settings, [&](auto p) { q_stdout << p << '\n'; },
            &terminate);
        q_stdout.flush();
        if (settings.draw_image) {
            const Image image {builder};
            image.save(settings.image_path.c_str());
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        q_stderr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        q_stderr << "Unexpected non-exception error!" << '\n';
        return EXIT_FAILURE;
    }
}
