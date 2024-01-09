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

#include <cstdint>
#include <stdexcept>

#include <QCommandLineParser>

#include "settings.hpp"

namespace {
bool is_valid_image_path(std::string_view path)
{
    return path.ends_with(".bmp") || path.ends_with(".png");
}

SightRead::Difficulty string_to_diff(std::string_view text)
{
    if (text == "expert") {
        return SightRead::Difficulty::Expert;
    }
    if (text == "hard") {
        return SightRead::Difficulty::Hard;
    }
    if (text == "medium") {
        return SightRead::Difficulty::Medium;
    }
    if (text == "easy") {
        return SightRead::Difficulty::Easy;
    }
    throw std::invalid_argument("Unrecognised difficulty");
}

SightRead::Instrument string_to_inst(std::string_view text, Game game)
{
    if (text == "guitar") {
        if (game == Game::FortniteFestival) {
            return SightRead::Instrument::FortniteGuitar;
        }
        return SightRead::Instrument::Guitar;
    }
    if (text == "coop") {
        return SightRead::Instrument::GuitarCoop;
    }
    if (text == "bass") {
        if (game == Game::FortniteFestival) {
            return SightRead::Instrument::FortniteBass;
        }
        return SightRead::Instrument::Bass;
    }
    if (text == "rhythm") {
        return SightRead::Instrument::Rhythm;
    }
    if (text == "keys") {
        return SightRead::Instrument::Keys;
    }
    if (text == "ghl") {
        return SightRead::Instrument::GHLGuitar;
    }
    if (text == "ghlbass") {
        return SightRead::Instrument::GHLBass;
    }
    if (text == "ghlrhythm") {
        return SightRead::Instrument::GHLRhythm;
    }
    if (text == "ghlcoop") {
        return SightRead::Instrument::GHLGuitarCoop;
    }
    if (text == "drums") {
        if (game == Game::FortniteFestival) {
            return SightRead::Instrument::FortniteDrums;
        }
        return SightRead::Instrument::Drums;
    }
    if (text == "vocals") {
        return SightRead::Instrument::FortniteVocals;
    }
    throw std::invalid_argument("Unrecognised instrument");
}

Game game_from_string(std::string_view game)
{
    const std::map<std::string_view, Game> game_map {
        {"ch", Game::CloneHero},
        {"fnf", Game::FortniteFestival},
        {"gh1", Game::GuitarHeroOne},
        {"rb", Game::RockBand},
        {"rb3", Game::RockBandThree}};
    return game_map.at(game);
}

std::unique_ptr<QCommandLineParser> arg_parser()
{
    auto parser = std::make_unique<QCommandLineParser>();
    parser->setApplicationDescription(
        "A program to generate optimal Star Power paths for Clone Hero");
    parser->addOption(
        {{"?", "h", "help"}, "Displays help on commandline options."});
    parser->addVersionOption();
    parser->addOptions(
        {{{"f", "file"}, "Chart filename.", "file"},
         {{"o", "output"},
          "Location to save output image (must be a .bmp or .png). Default "
          "path.png.",
          "output",
          "path.png"},
         {{"d", "diff"},
          "Difficulty, options are easy, medium, hard, expert. Default expert.",
          "difficulty",
          "expert"},
         {{"i", "instrument"},
          "Instrument, options are guitar, coop, bass, rhythm, keys, ghl, "
          "ghlbass, ghlrhythm, ghlcoop, drums, vocals. Default guitar.",
          "instrument",
          "guitar"},
         {{"sqz", "squeeze"},
          "Squeeze% (0 to 100). Default 100.",
          "squeeze",
          "100"},
         {{"ew", "early-whammy"},
          "Early whammy% (0 to 100), <= squeeze, defaults to squeeze.",
          "early-whammy"},
         {{"lazy", "lazy-whammy"},
          "Time before whammying starts on sustains in milliseconds. Default "
          "0.",
          "lazy-whammy",
          "0"},
         {{"delay", "whammy-delay"},
          "Time after an activation ends before whammying can resume. Default "
          "0.",
          "whammy-delay",
          "0"},
         {{"lag", "video-lag"},
          "Video lag calibration setting in milliseconds. Default 0.",
          "video-lag",
          "0"},
         {{"s", "speed"}, "Speed in %. Default 100.", "speed", "100"},
         {{"l", "lefty-flip"}, "Draw with lefty flip."},
         {"no-double-kick", "Disable 2x kick for drum charts."},
         {"no-kick", "Disable single kicks for drum charts."},
         {"no-pro-drums", "Disable pro drums."},
         {"enable-dynamics",
          "Enable double points for ghost and accent notes."},
         {"engine", "Engine, options are ch, fnf, gh1, rb, rb3. Default ch.",
          "engine", "ch"},
         {{"p", "precision-mode"}, "Turn on precision mode for CH."},
         {{"b", "blank"}, "Give a blank chart image."},
         {"no-image", "Do not create an image."},
         {"no-bpms", "Do not draw BPMs."},
         {"no-solos", "Do not draw solo sections."},
         {"no-time-sigs", "Do not draw time signatures."},
         {"act-opacity",
          "Opacity of drawn activations (0.0 to 1.0). Default 0.33.",
          "act-opacity", "0.33"}});
    return parser;
}
}

std::unique_ptr<Engine>
game_to_engine(Game game, SightRead::Instrument instrument, bool precision_mode)
{
    switch (game) {
    case Game::CloneHero:
        if (instrument == SightRead::Instrument::Drums) {
            if (precision_mode) {
                return std::make_unique<ChPrecisionDrumEngine>();
            }
            return std::make_unique<ChDrumEngine>();
        }
        if (precision_mode) {
            return std::make_unique<ChPrecisionGuitarEngine>();
        }
        return std::make_unique<ChGuitarEngine>();
    case Game::FortniteFestival:
        if (instrument == SightRead::Instrument::FortniteBass) {
            return std::make_unique<FortniteBassEngine>();
        }
        if (instrument == SightRead::Instrument::FortniteVocals) {
            return std::make_unique<FortniteVocalsEngine>();
        }
        return std::make_unique<FortniteGuitarEngine>();
    case Game::GuitarHeroOne:
        return std::make_unique<Gh1Engine>();
    case Game::RockBand:
        if (instrument == SightRead::Instrument::Bass) {
            return std::make_unique<RbBassEngine>();
        }
        return std::make_unique<RbEngine>();
    case Game::RockBandThree:
        if (instrument == SightRead::Instrument::Bass) {
            return std::make_unique<Rb3BassEngine>();
        }
        return std::make_unique<Rb3Engine>();
    default:
        throw std::invalid_argument("Invalid Game");
    }
}

Settings from_args(const QStringList& args)
{
    constexpr int MAX_PERCENT = 100;
    constexpr int MAX_SPEED = 5000;
    constexpr int MAX_VIDEO_LAG = 200;
    constexpr int MIN_SPEED = 5;
    constexpr double MS_PER_SECOND = 1000.0;

    auto parser = arg_parser();
    parser->process(args);

    if (parser->isSet("help")) {
        parser->showHelp();
    }

    Settings settings;

    settings.blank = parser->isSet("blank");
    settings.filename = parser->value("file").toStdString();
    if (settings.filename.empty()) {
        throw std::invalid_argument("No file was specified");
    }

    const auto engine_name = parser->value("engine").toStdString();
    settings.game = game_from_string(engine_name);

    settings.difficulty = string_to_diff(parser->value("diff").toStdString());
    settings.instrument = string_to_inst(
        parser->value("instrument").toStdString(), settings.game);

    const auto precision_mode = parser->isSet("precision-mode");
    settings.engine
        = game_to_engine(settings.game, settings.instrument, precision_mode);

    settings.image_path = parser->value("output").toStdString();
    if (!is_valid_image_path(settings.image_path)) {
        throw std::invalid_argument(
            "Image output must be a bitmap or png (.bmp / .png)");
    }

    settings.is_lefty_flip = parser->isSet("lefty-flip");
    settings.draw_image = !parser->isSet("no-image");
    settings.draw_bpms = !parser->isSet("no-bpms");
    settings.draw_solos = !parser->isSet("no-solos");
    settings.draw_time_sigs = !parser->isSet("no-time-sigs");
    settings.drum_settings.enable_double_kick
        = !parser->isSet("no-double-kick");
    settings.drum_settings.disable_kick = parser->isSet("no-kick");
    settings.drum_settings.pro_drums = !parser->isSet("no-pro-drums");
    settings.drum_settings.enable_dynamics = parser->isSet("enable-dynamics");

    const auto squeeze = parser->value("squeeze").toInt();
    auto early_whammy = squeeze;
    if (parser->isSet("early-whammy")) {
        early_whammy = parser->value("early-whammy").toInt();
    }
    const auto lazy_whammy = parser->value("lazy-whammy").toInt();
    const auto whammy_delay = parser->value("whammy-delay").toInt();

    if (squeeze < 0 || squeeze > MAX_PERCENT) {
        throw std::invalid_argument("Squeeze must lie between 0 and 100");
    }
    if (early_whammy < 0 || early_whammy > MAX_PERCENT) {
        throw std::invalid_argument("Early whammy must lie between 0 and 100");
    }
    if (lazy_whammy < 0) {
        throw std::invalid_argument(
            "Lazy whammy must be greater than or equal to 0");
    }
    if (whammy_delay < 0) {
        throw std::invalid_argument(
            "Whammy delay must be greater than or equal to 0");
    }

    settings.squeeze_settings.squeeze = squeeze / 100.0;
    settings.squeeze_settings.early_whammy = early_whammy / 100.0;
    settings.squeeze_settings.lazy_whammy
        = SightRead::Second {lazy_whammy / MS_PER_SECOND};
    settings.squeeze_settings.whammy_delay
        = SightRead::Second {whammy_delay / MS_PER_SECOND};

    const auto video_lag = parser->value("video-lag").toInt();
    if (video_lag < -MAX_VIDEO_LAG || video_lag > MAX_VIDEO_LAG) {
        throw std::invalid_argument(
            "Video lag setting unsupported by Clone Hero");
    }

    settings.squeeze_settings.video_lag
        = SightRead::Second {video_lag / MS_PER_SECOND};

    const auto speed = parser->value("speed").toInt();
    if (speed < MIN_SPEED || speed > MAX_SPEED || speed % MIN_SPEED != 0) {
        throw std::invalid_argument("Speed unsupported by Clone Hero");
    }

    settings.speed = speed;

    const auto opacity = parser->value("act-opacity").toFloat();
    if (opacity < 0.0F || opacity > 1.0F) {
        throw std::invalid_argument(
            "Activation opacity should lie between 0.0 and 1.0");
    }

    settings.opacity = opacity;

    return settings;
}
