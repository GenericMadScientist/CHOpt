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

#include <QFileDialog>

#include "image.hpp"
#include "mainwindow.hpp"
#include "optimiser.hpp"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(Difficulty)
Q_DECLARE_METATYPE(Instrument)
Q_DECLARE_METATYPE(std::optional<Song>)

template <typename T, typename F>
static ImageBuilder make_builder_from_track(const Song& song,
                                            const NoteTrack<T>& track,
                                            const Settings& settings, F write)
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
        write("Optimising, please wait...");
        const Optimiser optimiser {&processed_track};
        path = optimiser.optimal_path();
        builder.add_sp_acts(processed_track.points(), path);
        write(processed_track.path_summary(path).c_str());
    }

    builder.add_measure_values(processed_track.points(), path);
    builder.add_sp_values(processed_track.sp_data());

    return builder;
}

template <typename F>
static ImageBuilder make_builder(const Song& song, const Settings& settings,
                                 F write)
{
    if (settings.instrument == Instrument::GHLGuitar) {
        const auto& track = song.ghl_guitar_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings, write);
    }
    if (settings.instrument == Instrument::GHLBass) {
        const auto& track = song.ghl_bass_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings, write);
    }
    if (settings.instrument == Instrument::Drums) {
        const auto& track = song.drum_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings, write);
    }
    const auto& track = track_from_inst_diff(settings, song);
    return make_builder_from_track(song, track, settings, write);
}

class ParserThread : public QThread {
    Q_OBJECT

private:
    QString m_file_name;

public:
    ParserThread(QObject* parent = nullptr)
        : QThread(parent)
    {
    }

    void run() override
    {
        try {
            emit result_ready(Song::from_filename(m_file_name.toStdString()));
        } catch (const std::exception&) {
            emit result_ready({});
        }
    }

    void set_file_name(const QString& file_name) { m_file_name = file_name; }

signals:
    void result_ready(const std::optional<Song>& song);
};

class OptimiserThread : public QThread {
    Q_OBJECT

private:
    Settings m_settings;
    std::optional<Song> m_song;
    QString m_file_name;

public:
    OptimiserThread(QObject* parent = nullptr)
        : QThread(parent)
    {
    }

    void run() override
    {
        const auto builder
            = make_builder(*m_song, m_settings,
                           [&](const QString& text) { emit write_text(text); });
        emit write_text("Saving image...");
        const Image image {builder};
        image.save(m_file_name.toStdString().c_str());
        emit write_text("Image saved");
    }

    void set_data(const Settings& settings, const Song& song,
                  const QString& file_name)
    {
        m_settings = settings;
        m_song = song;
        m_file_name = file_name;
    }

signals:
    void write_text(const QString& text);
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_ui {std::make_unique<Ui::MainWindow>()}
{
    qRegisterMetaType<std::optional<Song>>();

    m_ui->setupUi(this);
    m_ui->instrumentComboBox->setEnabled(false);
    m_ui->difficultyComboBox->setEnabled(false);
    m_ui->findPathButton->setEnabled(false);

    m_ui->lazyWhammyLineEdit->setValidator(new QIntValidator(
        0, std::numeric_limits<int>::max(), m_ui->lazyWhammyLineEdit));
}

MainWindow::~MainWindow()
{
    if (m_thread != nullptr) {
        m_thread->quit();
        // We give the thread 5 seconds to obey, then kill it. Although all the
        // thread does apart from CPU-bound work is write to a file at the very
        // end, so the call to terminate is not so bad.
        if (!m_thread->wait(5000)) {
            m_thread->terminate();
            m_thread->wait();
        }
    }
}

void MainWindow::write_message(const QString& message)
{
    m_ui->messageBox->append(message);
}

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

    settings.blank = m_ui->blankPathCheckBox->isChecked();
    settings.draw_bpms = m_ui->drawBpmsCheckBox->isChecked();
    settings.draw_solos = m_ui->drawSolosCheckBox->isChecked();
    settings.draw_time_sigs = m_ui->drawTsesCheckBox->isChecked();
    settings.difficulty
        = m_ui->difficultyComboBox->currentData().value<Difficulty>();
    settings.instrument
        = m_ui->instrumentComboBox->currentData().value<Instrument>();
    settings.squeeze = m_ui->squeezeSlider->value() / 100.0;
    settings.early_whammy = m_ui->earlyWhammySlider->value() / 100.0;

    const auto lazy_whammy_text = m_ui->lazyWhammyLineEdit->text();
    bool ok;
    auto lazy_whammy_ms = lazy_whammy_text.toInt(&ok, 10);
    if (ok) {
        settings.lazy_whammy = lazy_whammy_ms / 1000.0;
    } else {
        settings.lazy_whammy = 0.0;
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

    m_ui->selectFileButton->setEnabled(false);

    auto* worker_thread = new ParserThread(this);
    worker_thread->set_file_name(file_name);
    connect(worker_thread, &ParserThread::result_ready, this,
            &MainWindow::song_read);
    connect(worker_thread, &ParserThread::finished, worker_thread,
            &QObject::deleteLater);
    m_thread = worker_thread;
    worker_thread->start();
}

void MainWindow::on_findPathButton_clicked()
{
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

    const auto settings = get_settings();
    auto* worker_thread = new OptimiserThread(this);
    worker_thread->set_data(settings, *m_song, file_name);
    connect(worker_thread, &OptimiserThread::write_text, this,
            &MainWindow::write_message);
    connect(worker_thread, &OptimiserThread::finished, this,
            &MainWindow::path_found);
    connect(worker_thread, &OptimiserThread::finished, worker_thread,
            &QObject::deleteLater);
    m_thread = worker_thread;
    worker_thread->start();
}

void MainWindow::song_read(const std::optional<Song>& song)
{
    m_thread = nullptr;

    if (!song.has_value()) {
        write_message("Song file invalid");
        m_ui->selectFileButton->setEnabled(true);
        return;
    }

    m_song = song;

    m_ui->instrumentComboBox->clear();
    const std::map<Instrument, QString> INST_NAMES {
        {Instrument::Guitar, "Guitar"},
        {Instrument::GuitarCoop, "Guitar Co-op"},
        {Instrument::Bass, "Bass"},
        {Instrument::Rhythm, "Rhythm"},
        {Instrument::Keys, "Keys"},
        {Instrument::GHLGuitar, "GHL Guitar"},
        {Instrument::GHLBass, "GHL Bass"},
        {Instrument::Drums, "Drums"}};
    for (auto inst : m_song->instruments()) {
        m_ui->instrumentComboBox->addItem(INST_NAMES.at(inst),
                                          QVariant::fromValue(inst));
    }
    m_ui->instrumentComboBox->setCurrentIndex(0);

    m_ui->difficultyComboBox->clear();
    m_ui->difficultyComboBox->addItem("Easy",
                                      QVariant::fromValue(Difficulty::Easy));
    m_ui->difficultyComboBox->addItem("Medium",
                                      QVariant::fromValue(Difficulty::Medium));
    m_ui->difficultyComboBox->addItem("Hard",
                                      QVariant::fromValue(Difficulty::Hard));
    m_ui->difficultyComboBox->addItem("Expert",
                                      QVariant::fromValue(Difficulty::Expert));
    m_ui->difficultyComboBox->setCurrentIndex(3);

    write_message("Song loaded");

    m_ui->findPathButton->setEnabled(true);
    m_ui->instrumentComboBox->setEnabled(true);
    m_ui->difficultyComboBox->setEnabled(true);
    m_ui->selectFileButton->setEnabled(true);
}

void MainWindow::path_found()
{
    m_thread = nullptr;
    m_ui->selectFileButton->setEnabled(true);
    m_ui->findPathButton->setEnabled(true);
}

void MainWindow::on_squeezeSlider_valueChanged(int value)
{
    const auto ew_value = m_ui->earlyWhammySlider->value();
    if (ew_value > value) {
        m_ui->earlyWhammySlider->setValue(value);
    }
}

void MainWindow::on_earlyWhammySlider_valueChanged(int value)
{
    const auto sqz_value = m_ui->squeezeSlider->value();
    if (sqz_value < value) {
        m_ui->earlyWhammySlider->setValue(sqz_value);
    }
}

#include "mainwindow.moc"
