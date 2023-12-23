/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QUrl>

#include "image.hpp"
#include "json_settings.hpp"
#include "mainwindow.hpp"
#include "optimiser.hpp"
#include "ui_mainwindow.h"

class ParserThread : public QThread {
    Q_OBJECT

private:
    QString m_file_name;

    static std::set<Game> song_file_games(const SongFile& song_file)
    {
        const std::set<Game> all_games {Game::CloneHero, Game::FortniteFestival,
                                        Game::GuitarHeroOne, Game::RockBand,
                                        Game::RockBandThree};
        std::set<Game> supported_games;
        for (const auto game : all_games) {
            try {
                song_file.load_song(game);
                supported_games.insert(game);
            } catch (const std::exception&) {
            }
        }
        return supported_games;
    }

public:
    explicit ParserThread(QObject* parent = nullptr)
        : QThread(parent)
    {
    }

    void run() override
    {
        try {
            SongFile song_file {m_file_name.toStdString()};
            auto games = song_file_games(song_file);
            if (games.empty()) {
                emit parsing_failed(m_file_name);
            }
            emit result_ready(std::move(song_file), std::move(games),
                              m_file_name);
        } catch (const std::exception&) {
            emit parsing_failed(m_file_name);
        }
    }

    void set_file_name(const QString& file_name) { m_file_name = file_name; }

signals:
    void parsing_failed(const QString& file_name);
    void result_ready(SongFile loaded_file, std::set<Game> games,
                      const QString& file_name);
};

class OptimiserThread : public QThread {
    Q_OBJECT

private:
    std::atomic<bool> m_terminate = false;
    Settings m_settings;
    std::optional<SightRead::Song> m_song;
    QString m_file_name;

public:
    explicit OptimiserThread(QObject* parent = nullptr)
        : QThread(parent)
    {
    }

    void run() override
    {
        try {
            const auto& track
                = m_song->track(m_settings.instrument, m_settings.difficulty);
            const auto builder = make_builder(
                *m_song, track, m_settings,
                [&](const QString& text) { emit write_text(text); },
                &m_terminate);
            emit write_text("Saving image...");
            const Image image {builder};
            image.save(m_file_name.toStdString().c_str());
            emit write_text("Image saved");
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_file_name));
        } catch (const std::runtime_error&) {
            // We ignore this exception because it's how we break out of the
            // computation on program close.
        }
    }

    void set_data(Settings settings, SightRead::Song song,
                  const QString& file_name)
    {
        m_settings = std::move(settings);
        m_song = std::move(song);
        m_file_name = file_name;
    }

    void end_thread() { m_terminate = true; }

signals:
    void write_text(const QString& text);
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui {std::make_unique<Ui::MainWindow>()}
{
    // This is the maximum for our validators instead of MAX_INT because
    // with MAX_INT the user can enter 9,999,999,999 which causes an
    // overflow.
    constexpr int max_digits_int = 999999999;

    m_ui->setupUi(this);
    m_ui->instrumentComboBox->setEnabled(false);
    m_ui->difficultyComboBox->setEnabled(false);
    m_ui->engineComboBox->setEnabled(false);
    m_ui->findPathButton->setEnabled(false);

    m_ui->lazyWhammyLineEdit->setValidator(
        new QIntValidator(0, max_digits_int, m_ui->lazyWhammyLineEdit));
    m_ui->whammyDelayLineEdit->setValidator(
        new QIntValidator(0, max_digits_int, m_ui->whammyDelayLineEdit));
    m_ui->speedLineEdit->setValidator(
        new QIntValidator(5, 5000, m_ui->speedLineEdit));

    m_ui->squeezeLabel->setMinimumWidth(30);
    m_ui->earlyWhammyLabel->setMinimumWidth(30);
    m_ui->videoLagLabel->setMinimumWidth(30);
    m_ui->opacityLabel->setMinimumWidth(30);

    const auto settings = load_saved_settings(
        QCoreApplication::applicationDirPath().toStdString());
    m_ui->squeezeSlider->setValue(settings.squeeze);
    m_ui->earlyWhammySlider->setValue(settings.early_whammy);
    m_ui->lazyWhammyLineEdit->setText(QString::number(settings.lazy_whammy));
    m_ui->whammyDelayLineEdit->setText(QString::number(settings.whammy_delay));
    m_ui->videoLagSlider->setValue(settings.video_lag);
    m_ui->leftyCheckBox->setChecked(settings.is_lefty_flip);

    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    JsonSettings settings;
    settings.squeeze = m_ui->squeezeSlider->value();
    settings.early_whammy = m_ui->earlyWhammySlider->value();
    settings.video_lag = m_ui->videoLagSlider->value();
    settings.is_lefty_flip = m_ui->leftyCheckBox->isChecked();

    bool ok;
    const auto lazy_whammy_text = m_ui->lazyWhammyLineEdit->text();
    const auto lazy_whammy_ms = lazy_whammy_text.toInt(&ok, 10);
    if (ok) {
        settings.lazy_whammy = lazy_whammy_ms;
    } else {
        settings.lazy_whammy = 0;
    }
    const auto whammy_delay_text = m_ui->whammyDelayLineEdit->text();
    const auto whammy_delay_ms = whammy_delay_text.toInt(&ok, 10);
    if (ok) {
        settings.whammy_delay = whammy_delay_ms;
    } else {
        settings.whammy_delay = 0;
    }

    save_settings(settings,
                  QCoreApplication::applicationDirPath().toStdString());

    if (m_thread != nullptr) {
        auto* opt_thread = dynamic_cast<OptimiserThread*>(m_thread);
        if (opt_thread != nullptr) {
            opt_thread->end_thread();
        }
        m_thread->quit();
        // We give the thread 5 seconds to obey, then kill it. Although all
        // the thread does apart from CPU-bound work is write to a file at
        // the very end, so the call to terminate is not so bad.
        if (!m_thread->wait(5000)) {
            m_thread->terminate();
            m_thread->wait();
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const auto urls = event->mimeData()->urls();
    if (urls.size() != 1) {
        write_message("Only one file may be dragged and dropped");
        return;
    }

    load_file(urls[0].toLocalFile());
}

void MainWindow::write_message(const QString& message)
{
    m_ui->messageBox->append(message);
}

Settings MainWindow::get_settings() const
{
    Settings settings;

    settings.blank = m_ui->blankPathCheckBox->isChecked();
    settings.draw_bpms = m_ui->drawBpmsCheckBox->isChecked();
    settings.draw_solos = m_ui->drawSolosCheckBox->isChecked();
    settings.draw_time_sigs = m_ui->drawTsesCheckBox->isChecked();
    settings.drum_settings.enable_double_kick
        = m_ui->doubleKickCheckBox->isChecked();
    settings.drum_settings.disable_kick = m_ui->noKickCheckBox->isChecked();
    settings.drum_settings.pro_drums = m_ui->proDrumsCheckBox->isChecked();
    settings.drum_settings.enable_dynamics
        = m_ui->dynamicsCheckBox->isChecked();
    settings.difficulty = m_ui->difficultyComboBox->currentData()
                              .value<SightRead::Difficulty>();
    settings.instrument = m_ui->instrumentComboBox->currentData()
                              .value<SightRead::Instrument>();
    settings.squeeze_settings.squeeze = m_ui->squeezeSlider->value() / 100.0;
    settings.squeeze_settings.early_whammy
        = m_ui->earlyWhammySlider->value() / 100.0;
    settings.squeeze_settings.video_lag
        = SightRead::Second {m_ui->videoLagSlider->value() / 1000.0};
    settings.game = m_ui->engineComboBox->currentData().value<Game>();
    const auto precision_mode = m_ui->precisionModeCheckBox->isChecked();
    settings.engine
        = game_to_engine(settings.game, settings.instrument, precision_mode);
    settings.is_lefty_flip = m_ui->leftyCheckBox->isChecked();
    settings.opacity = m_ui->opacitySlider->value() / 100.0F;

    const auto lazy_whammy_text = m_ui->lazyWhammyLineEdit->text();
    bool ok;
    auto lazy_whammy_ms = lazy_whammy_text.toInt(&ok, 10);
    if (ok) {
        settings.squeeze_settings.lazy_whammy
            = SightRead::Second {lazy_whammy_ms / 1000.0};
    } else {
        settings.squeeze_settings.lazy_whammy = SightRead::Second {0.0};
    }

    const auto whammy_delay_text = m_ui->whammyDelayLineEdit->text();
    auto whammy_delay_ms = whammy_delay_text.toInt(&ok, 10);
    if (ok) {
        settings.squeeze_settings.whammy_delay
            = SightRead::Second {whammy_delay_ms / 1000.0};
    } else {
        settings.squeeze_settings.whammy_delay = SightRead::Second {0.0};
    }

    const auto speed_text = m_ui->speedLineEdit->text();
    auto speed = speed_text.toInt(&ok, 10);
    if (ok) {
        settings.speed = speed;
    } else {
        settings.speed = 100;
    }

    return settings;
}

void MainWindow::on_selectFileButton_clicked()
{
    const auto file_name = QFileDialog::getOpenFileName(
        this, "Open song", "../", "Song charts (*.chart *.mid)");
    if (file_name.isEmpty()) {
        return;
    }

    load_file(file_name);
}

void MainWindow::load_file(const QString& file_name)
{
    if (!file_name.endsWith(".chart") && !file_name.endsWith(".mid")) {
        write_message("File must be .chart or .mid");
        return;
    }

    m_ui->selectFileButton->setEnabled(false);
    setAcceptDrops(false);

    auto* worker_thread = new ParserThread(this);
    worker_thread->set_file_name(file_name);
    connect(worker_thread, &ParserThread::result_ready, this,
            &MainWindow::song_read);
    connect(worker_thread, &ParserThread::parsing_failed, this,
            &MainWindow::parsing_failed);
    connect(worker_thread, &ParserThread::finished, worker_thread,
            &QObject::deleteLater);
    m_thread = worker_thread;
    worker_thread->start();
}

void MainWindow::populate_games(const std::set<Game>& games)
{
    m_ui->engineComboBox->clear();
    const std::array<std::pair<Game, QString>, 5> full_game_set {
        {{Game::CloneHero, "Clone Hero"},
         {Game::FortniteFestival, "Fortnite Festival"},
         {Game::GuitarHeroOne, "Guitar Hero 1"},
         {Game::RockBand, "Rock Band"},
         {Game::RockBandThree, "Rock Band 3"}}};
    for (const auto& [game, name] : full_game_set) {
        if (games.contains(game)) {
            m_ui->engineComboBox->addItem(name, QVariant::fromValue(game));
        }
    }
    m_ui->engineComboBox->setCurrentIndex(0);
}

void MainWindow::on_findPathButton_clicked()
{
    const auto speed_text = m_ui->speedLineEdit->text();
    bool ok;
    const auto speed = speed_text.toInt(&ok, 10);
    if (!ok || speed < 5 || speed > 5000 || speed % 5 != 0) {
        write_message("Speed not supported by Clone Hero");
        return;
    }

    const auto file_name = QFileDialog::getSaveFileName(this, "Save image", ".",
                                                        "Images (*.png *.bmp)");
    if (file_name.isEmpty()) {
        return;
    }
    if (!file_name.endsWith(".bmp") && !file_name.endsWith(".png")) {
        write_message("Not a valid image file");
        return;
    }

    m_ui->selectFileButton->setEnabled(false);
    m_ui->findPathButton->setEnabled(false);

    auto* worker_thread = new OptimiserThread(this);
    auto settings = get_settings();
    auto song = m_loaded_file->load_song(settings.game);
    worker_thread->set_data(std::move(settings), std::move(song), file_name);
    connect(worker_thread, &OptimiserThread::write_text, this,
            &MainWindow::write_message);
    connect(worker_thread, &OptimiserThread::finished, this,
            &MainWindow::path_found);
    connect(worker_thread, &OptimiserThread::finished, worker_thread,
            &QObject::deleteLater);
    m_thread = worker_thread;
    worker_thread->start();
}

void MainWindow::parsing_failed(const QString& file_name)
{
    m_thread = nullptr;
    write_message(file_name + " invalid");
    m_ui->selectFileButton->setEnabled(true);
}

void MainWindow::song_read(SongFile loaded_file, const std::set<Game>& games,
                           const QString& file_name)
{
    m_thread = nullptr;
    m_loaded_file = std::move(loaded_file);

    populate_games(games);

    write_message(file_name + " loaded");

    m_ui->findPathButton->setEnabled(true);
    m_ui->instrumentComboBox->setEnabled(true);
    m_ui->difficultyComboBox->setEnabled(true);
    m_ui->engineComboBox->setEnabled(true);
    m_ui->selectFileButton->setEnabled(true);
    setAcceptDrops(true);
}

void MainWindow::path_found()
{
    m_thread = nullptr;
    m_ui->selectFileButton->setEnabled(true);
    m_ui->findPathButton->setEnabled(true);
}

void MainWindow::on_engineComboBox_currentIndexChanged(int index)
{
    m_ui->instrumentComboBox->clear();

    if (index == -1) {
        return;
    }
    const std::map<SightRead::Instrument, QString> INST_NAMES {
        {SightRead::Instrument::Guitar, "Guitar"},
        {SightRead::Instrument::GuitarCoop, "Guitar Co-op"},
        {SightRead::Instrument::Bass, "Bass"},
        {SightRead::Instrument::Rhythm, "Rhythm"},
        {SightRead::Instrument::Keys, "Keys"},
        {SightRead::Instrument::GHLGuitar, "GHL Guitar"},
        {SightRead::Instrument::GHLBass, "GHL Bass"},
        {SightRead::Instrument::GHLRhythm, "GHL Rhythm"},
        {SightRead::Instrument::GHLGuitarCoop, "GHL Guitar Co-op"},
        {SightRead::Instrument::Drums, "Drums"}};
    const auto game = m_ui->engineComboBox->currentData().value<Game>();
    for (auto inst : m_loaded_file->load_song(game).instruments()) {
        m_ui->instrumentComboBox->addItem(INST_NAMES.at(inst),
                                          QVariant::fromValue(inst));
    }
    m_ui->instrumentComboBox->setCurrentIndex(0);
}

void MainWindow::on_instrumentComboBox_currentIndexChanged(int index)
{
    m_ui->difficultyComboBox->clear();

    if (index == -1) {
        return;
    }
    const std::map<SightRead::Difficulty, QString> DIFF_NAMES {
        {SightRead::Difficulty::Easy, "Easy"},
        {SightRead::Difficulty::Medium, "Medium"},
        {SightRead::Difficulty::Hard, "Hard"},
        {SightRead::Difficulty::Expert, "Expert"}};
    const auto inst = m_ui->instrumentComboBox->currentData()
                          .value<SightRead::Instrument>();
    const auto game = m_ui->engineComboBox->currentData().value<Game>();
    for (auto diff : m_loaded_file->load_song(game).difficulties(inst)) {
        m_ui->difficultyComboBox->addItem(DIFF_NAMES.at(diff),
                                          QVariant::fromValue(diff));
    }
    const auto count = m_ui->difficultyComboBox->count();
    m_ui->difficultyComboBox->setCurrentIndex(count - 1);
}

void MainWindow::on_squeezeSlider_valueChanged(int value)
{
    m_ui->squeezeLabel->setText(QString::number(value));
}

void MainWindow::on_earlyWhammySlider_valueChanged(int value)
{
    m_ui->earlyWhammyLabel->setText(QString::number(value));
}

void MainWindow::on_videoLagSlider_valueChanged(int value)
{
    m_ui->videoLagLabel->setText(QString::number(value));
}

void MainWindow::on_opacitySlider_valueChanged(int value)
{
    auto text = QString::number(value / 100);
    text += '.';
    text += QString::number((value % 100) / 10);
    text += QString::number(value % 10);
    m_ui->opacityLabel->setText(text);
}

#include "mainwindow.moc"
