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

#ifndef CHOPT_MAINWINDOW_HPP
#define CHOPT_MAINWINDOW_HPP

#include <memory>

#include <QMainWindow>

#include "settings.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::string file_name;
    Settings get_settings() const;

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_earlyWhammySlider_valueChanged(int value);
    void on_findPathButton_clicked();
    void on_selectFileButton_clicked();
    void on_squeezeSlider_valueChanged(int value);
};

#endif
