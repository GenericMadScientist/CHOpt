/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023 Raymond Wright
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

#ifndef CHOPT_MAINWINDOW_HPP
#define CHOPT_MAINWINDOW_HPP

#include <memory>
#include <optional>
#include <set>

#include <QMainWindow>
#include <QString>
#include <QThread>

#include <sightread/song.hpp>

#include "settings.hpp"
#include "songfile.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
    std::optional<SongFile> m_loaded_file;
    std::unique_ptr<QThread> m_thread;
    Settings get_settings() const;
    void load_file(const QString& file_name);
    void populate_games(const std::set<Game>& games);
    static constexpr int MAX_SPEED = 5000;
    static constexpr int MIN_SPEED = 5;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void clear_worker_thread();
    void on_earlyWhammySlider_valueChanged(int value);
    void on_engineComboBox_currentIndexChanged(int index);
    void on_findPathButton_clicked();
    void on_instrumentComboBox_currentIndexChanged(int index);
    void on_opacitySlider_valueChanged(int value);
    void on_selectFileButton_clicked();
    void on_squeezeSlider_valueChanged(int value);
    void on_videoLagSlider_valueChanged(int value);
    void parsing_failed(const QString& file_name);
    void path_found();
    void song_read(SongFile loaded_file, const std::set<Game>& games,
                   const QString& file_name);
    void write_message(const QString& message);
};

#endif
