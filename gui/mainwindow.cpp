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

#include <limits>
#include <stdexcept>

#include <QDebug>
#include <QFileDialog>

#include "image.hpp"
#include "mainwindow.hpp"
#include "optimiser.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui {std::make_unique<Ui::MainWindow>()}
{
    ui->setupUi(this);
    ui->instrumentComboBox->setEnabled(false);
    ui->difficultyComboBox->setEnabled(false);
    ui->findPathButton->setEnabled(false);

    ui->lazyWhammyLineEdit->setValidator(new QIntValidator(
        0, std::numeric_limits<int>::max(), ui->lazyWhammyLineEdit));
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

    settings.blank = ui->blankPathCheckBox->isChecked();
    settings.image_path = "path.png";
    settings.draw_bpms = ui->drawBpmsCheckBox->isChecked();
    settings.draw_solos = ui->drawSolosCheckBox->isChecked();
    settings.draw_time_sigs = ui->drawTsesCheckBox->isChecked();
    settings.difficulty = static_cast<Difficulty>(
        ui->difficultyComboBox->currentData().toInt());
    settings.instrument = static_cast<Instrument>(
        ui->instrumentComboBox->currentData().toInt());
    settings.squeeze = ui->squeezeSlider->value() / 100.0;
    settings.early_whammy = ui->earlyWhammySlider->value() / 100.0;

    const auto lazy_whammy_text = ui->lazyWhammyLineEdit->text();
    bool ok;
    auto lazy_whammy_ms = lazy_whammy_text.toInt(&ok, 10);
    if (ok) {
        settings.lazy_whammy = lazy_whammy_ms / 1000.0;
    } else {
        settings.lazy_whammy = 0.0;
    }

    return settings;
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

void MainWindow::on_findPathButton_clicked()
{
    const auto settings = get_settings();
    const auto builder = make_builder(*song, settings);
    const Image image {builder};
    image.save(settings.image_path.c_str());
}

void MainWindow::on_selectFileButton_clicked()
{
    const auto file_name = QFileDialog::getOpenFileName(
        this, "Open Image", ".", "Song charts (*.chart *.mid)");
    if (!file_name.isEmpty()) {
        song = Song::from_filename(file_name.toStdString());

        ui->instrumentComboBox->addItem("Guitar",
                                        static_cast<int>(Instrument::Guitar));
        ui->instrumentComboBox->addItem(
            "Guitar Co-op", static_cast<int>(Instrument::GuitarCoop));
        ui->instrumentComboBox->addItem("Bass",
                                        static_cast<int>(Instrument::Bass));
        ui->instrumentComboBox->addItem("Rhythm",
                                        static_cast<int>(Instrument::Rhythm));
        ui->instrumentComboBox->addItem("Keys",
                                        static_cast<int>(Instrument::Keys));
        ui->instrumentComboBox->addItem(
            "GHL Guitar", static_cast<int>(Instrument::GHLGuitar));
        ui->instrumentComboBox->addItem("GHL Bass",
                                        static_cast<int>(Instrument::GHLBass));
        ui->instrumentComboBox->addItem("Drums",
                                        static_cast<int>(Instrument::Drums));

        ui->difficultyComboBox->addItem("Easy",
                                        static_cast<int>(Difficulty::Easy));
        ui->difficultyComboBox->addItem("Medium",
                                        static_cast<int>(Difficulty::Medium));
        ui->difficultyComboBox->addItem("Hard",
                                        static_cast<int>(Difficulty::Hard));
        ui->difficultyComboBox->addItem("Expert",
                                        static_cast<int>(Difficulty::Expert));
        ui->difficultyComboBox->setCurrentIndex(3);

        ui->findPathButton->setEnabled(true);
        ui->instrumentComboBox->setEnabled(true);
        ui->difficultyComboBox->setEnabled(true);
    }
}

void MainWindow::on_squeezeSlider_valueChanged(int value)
{
    const auto ew_value = ui->earlyWhammySlider->value();
    if (ew_value > value) {
        ui->earlyWhammySlider->setValue(value);
    }
}

void MainWindow::on_earlyWhammySlider_valueChanged(int value)
{
    const auto sqz_value = ui->squeezeSlider->value();
    if (sqz_value < value) {
        ui->earlyWhammySlider->setValue(sqz_value);
    }
}
