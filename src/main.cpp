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

#include <nowide/args.hpp>
#include <nowide/iostream.hpp>

#include "image.hpp"
#include "optimiser.hpp"
#include "settings.hpp"
#include "song.hpp"
#include "time.hpp"

const static NoteTrack<NoteColour>&
track_from_inst_diff(const Settings& settings, const Song& song)
{
    switch (settings.instrument) {
    case Instrument::Guitar:
        return song.guitar_note_track(settings.difficulty);
    case Instrument::GuitarCoop:
        return song.guitar_coop_note_track(settings.difficulty);
    case Instrument::Bass:
        return song.bass_note_track(settings.difficulty);
    case Instrument::Rhythm:
        return song.rhythm_note_track(settings.difficulty);
    case Instrument::Keys:
        return song.keys_note_track(settings.difficulty);
    case Instrument::GHLGuitar:
        throw std::invalid_argument("GHL Guitar is not 5 fret");
    case Instrument::GHLBass:
        throw std::invalid_argument("GHL Bass is not 5 fret");
    case Instrument::Drums:
        throw std::invalid_argument("Drums is not 5 fret");
    }

    throw std::invalid_argument("Invalid instrument");
}

template <typename T>
static ImageBuilder make_builder_from_track(const Song& song,
                                            const NoteTrack<T>& track,
                                            const Settings& settings)
{
    ImageBuilder builder {track, song.resolution(), song.sync_track()};
    builder.add_song_header(song.song_header());
    builder.add_sp_phrases(track, song.resolution());

    if (settings.draw_bpms) {
        builder.add_bpms(song.sync_track(), song.resolution());
    }

    if (settings.draw_solos) {
        builder.add_solo_sections(track.solos(), song.resolution());
    }

    if (settings.draw_time_sigs) {
        builder.add_time_sigs(song.sync_track(), song.resolution());
    }

    const ProcessedSong processed_track {track,
                                         song.resolution(),
                                         song.sync_track(),
                                         settings.early_whammy,
                                         settings.squeeze,
                                         Second {settings.lazy_whammy}};
    Path path;

    if (!settings.blank) {
        const Optimiser optimiser {&processed_track};
        path = optimiser.optimal_path();
        builder.add_sp_acts(processed_track.points(), path);
        nowide::cout << processed_track.path_summary(path) << std::endl;
    }

    builder.add_measure_values(processed_track.points(), path);
    builder.add_sp_values(processed_track.sp_data());

    return builder;
}

static ImageBuilder make_builder(const Song& song, const Settings& settings)
{
    if (settings.instrument == Instrument::GHLGuitar) {
        const auto& track = song.ghl_guitar_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings);
    }
    if (settings.instrument == Instrument::GHLBass) {
        const auto& track = song.ghl_bass_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings);
    }
    if (settings.instrument == Instrument::Drums) {
        const auto& track = song.drum_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings);
    }
    const auto& track = track_from_inst_diff(settings, song);
    return make_builder_from_track(song, track, settings);
}

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
        const auto builder = make_builder(song, settings);
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
