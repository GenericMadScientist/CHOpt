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

#include <QDebug>
#include <QFileDialog>

#include "image.hpp"
#include "mainwindow.hpp"
#include "optimiser.hpp"
#include "settings.hpp"
#include "song.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui {std::make_unique<Ui::MainWindow>()}
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() = default;

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

Settings MainWindow::get_settings() const
{
    Settings settings;

    settings.blank = false;
    settings.filename = file_name;
    settings.image_path = "path.png";
    settings.draw_bpms = true;
    settings.draw_solos = true;
    settings.draw_time_sigs = true;
    settings.difficulty = Difficulty::Expert;
    settings.instrument = Instrument::Guitar;
    settings.squeeze = 1;
    settings.early_whammy = 1;
    settings.lazy_whammy = 1;

    return settings;
}

void MainWindow::on_findPathButton_clicked()
{
    const auto settings = get_settings();
    const auto song = Song::from_filename(file_name);
    qDebug() << "Parsing complete";
    const auto& track = track_from_inst_diff(settings, song);
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

    qDebug() << "Optimisation begins";

    const ProcessedSong processed_track {track,
                                         song.resolution(),
                                         song.sync_track(),
                                         settings.early_whammy,
                                         settings.squeeze,
                                         Second {settings.lazy_whammy}};
    Path path;

    if (settings.blank) {
        qDebug() << "Blank path chosen";
    } else {
        const Optimiser optimiser {&processed_track};
        path = optimiser.optimal_path();
        builder.add_sp_acts(processed_track.points(), path);
        qDebug() << processed_track.path_summary(path).c_str();
    }

    builder.add_measure_values(processed_track.points(), path);
    builder.add_sp_values(processed_track.sp_data());

    const Image image {builder};
    image.save(settings.image_path.c_str());
}

void MainWindow::on_selectFileButton_clicked()
{
    file_name = QFileDialog::getOpenFileName(this, "Open Image", ".",
                                             "Song charts (*.chart *.mid)")
                    .toStdString();
}
