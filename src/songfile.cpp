/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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

#include <filesystem>
#include <set>
#include <string_view>

#include <boost/nowide/fstream.hpp>

#include <sightread/midiparser.hpp>

#include "chartparser.hpp"
#include "ini.hpp"
#include "songfile.hpp"
#include "stringutil.hpp"

namespace {
std::set<SightRead::Instrument> permitted_instruments(Game game)
{
    switch (game) {
    case Game::CloneHero:
        return SightRead::all_instruments();
    case Game::GuitarHeroOne:
        return {SightRead::Instrument::Guitar};
    case Game::RockBand:
        return {SightRead::Instrument::Guitar, SightRead::Instrument::Bass};
    case Game::RockBandThree:
        return {SightRead::Instrument::Guitar, SightRead::Instrument::Bass,
                SightRead::Instrument::Keys};
    default:
        throw std::invalid_argument("Invalid Game");
    }
}

bool parse_solos(Game game) { return game != Game::GuitarHeroOne; }
}

SongFile::SongFile(const std::string& filename)
{
    std::string ini_file;
    const std::filesystem::path song_path {filename};
    const auto song_directory = song_path.parent_path();
    const auto ini_path = song_directory / "song.ini";
    boost::nowide::ifstream ini_in {ini_path.string()};
    if (ini_in.is_open()) {
        ini_file = std::string {std::istreambuf_iterator<char>(ini_in),
                                std::istreambuf_iterator<char>()};
    }
    m_metadata = parse_ini(ini_file);

    if (filename.ends_with(".chart")) {
        m_file_type = FileType::Chart;
    } else if (filename.ends_with(".mid")) {
        m_file_type = FileType::Midi;
    } else {
        throw std::invalid_argument("file should be .chart or .mid");
    }

    boost::nowide::ifstream in {filename, std::ios::binary};
    if (!in.is_open()) {
        throw std::invalid_argument("File did not open");
    }
    m_loaded_file = std::vector<std::uint8_t> {
        std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

SightRead::Song SongFile::load_song(Game game) const
{
    switch (m_file_type) {
    case FileType::Chart: {
        std::string_view chart_buffer {
            reinterpret_cast<const char*>(m_loaded_file.data()), // NOLINT
            m_loaded_file.size()};
        std::string u8_string;

        try {
            u8_string = to_utf8_string(chart_buffer);
        } catch (const std::invalid_argument& e) {
            throw SightRead::ParseError(e.what());
        }
        SightRead::ChartParser parser {m_metadata};
        parser.permit_instruments(permitted_instruments(game));
        parser.parse_solos(parse_solos(game));
        return parser.parse(u8_string);
    }
    case FileType::Midi:
        std::span<const std::uint8_t> midi_buffer {m_loaded_file.data(),
                                                   m_loaded_file.size()};
        SightRead::MidiParser parser {m_metadata};
        parser.permit_instruments(permitted_instruments(game));
        parser.parse_solos(parse_solos(game));
        return parser.parse(midi_buffer);
    }
    throw std::runtime_error("Invalid file type");
}
