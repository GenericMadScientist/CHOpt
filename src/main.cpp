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
#include <iostream>
#include <stdexcept>
#include <string>

#include "chart.hpp"
#include "image.hpp"
#include "optimiser.hpp"
#include "settings.hpp"
#include "time.hpp"

const static NoteTrack& track_from_inst_diff(const Settings& settings,
                                             const Chart& chart)
{
    switch (settings.instrument) {
    case Instrument::Guitar:
        return chart.guitar_note_track(settings.difficulty);
    case Instrument::GuitarCoop:
        return chart.guitar_coop_note_track(settings.difficulty);
    case Instrument::Bass:
        return chart.bass_note_track(settings.difficulty);
    case Instrument::Rhythm:
        return chart.rhythm_note_track(settings.difficulty);
    case Instrument::Keys:
        return chart.keys_note_track(settings.difficulty);
    }
    throw std::invalid_argument("Invalid instrument");
}

int main(int argc, char** argv)
{
    try {
        const auto settings = from_args(argc, argv);
        const auto chart = Chart::from_filename(settings.filename);
        const auto& track = track_from_inst_diff(settings, chart);
        ImageBuilder builder {track, chart.resolution(), chart.sync_track()};
        builder.add_song_header(chart.song_header());
        builder.add_sp_phrases(track, chart.resolution());

        if (settings.draw_bpms) {
            builder.add_bpms(chart.sync_track(), chart.resolution());
        }

        if (settings.draw_solos) {
            builder.add_solo_sections(track, chart.resolution());
        }

        if (settings.draw_time_sigs) {
            builder.add_time_sigs(chart.sync_track(), chart.resolution());
        }

        const ProcessedSong processed_track {track,
                                             chart.resolution(),
                                             chart.sync_track(),
                                             settings.early_whammy,
                                             settings.squeeze,
                                             Second {settings.lazy_whammy}};
        Path path;

        if (!settings.blank) {
            const Optimiser optimiser {&processed_track};
            path = optimiser.optimal_path();
            builder.add_sp_acts(processed_track.points(), path);
            std::cout << processed_track.path_summary(path) << std::endl;
        }

        builder.add_measure_values(processed_track.points(), path);
        builder.add_sp_values(processed_track.sp_data());
        const Image image {builder};
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
