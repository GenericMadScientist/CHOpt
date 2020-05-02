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

#ifndef CHOPT_IMAGE_HPP
#define CHOPT_IMAGE_HPP

#include <memory>
#include <tuple>
#include <vector>

#include "chart.hpp"
#include "processed.hpp"

struct DrawnRow {
    double start;
    double end;
};

struct DrawnNote {
    double beat;
    double length;
    NoteColour colour;
    bool is_sp_note;
};

class DrawingInstructions {
private:
    std::vector<DrawnRow> m_rows;
    std::vector<double> m_half_beat_lines;
    std::vector<double> m_beat_lines;
    std::vector<double> m_measure_lines;
    std::vector<DrawnNote> m_notes;
    std::vector<std::tuple<double, double>> m_green_ranges;
    std::vector<std::tuple<double, double>> m_blue_ranges;

public:
    DrawingInstructions(const NoteTrack& track, int resolution,
                        const SyncTrack& sync_track);
    void add_sp_phrases(const NoteTrack& track, int resolution);
    void add_sp_acts(const Path& path);

    [[nodiscard]] const std::vector<double>& beat_lines() const
    {
        return m_beat_lines;
    }
    [[nodiscard]] const std::vector<std::tuple<double, double>>&
    blue_ranges() const
    {
        return m_blue_ranges;
    }
    [[nodiscard]] const std::vector<std::tuple<double, double>>&
    green_ranges() const
    {
        return m_green_ranges;
    }
    [[nodiscard]] const std::vector<double>& half_beat_lines() const
    {
        return m_half_beat_lines;
    }
    [[nodiscard]] const std::vector<double>& measure_lines() const
    {
        return m_measure_lines;
    }
    [[nodiscard]] const std::vector<DrawnNote>& notes() const
    {
        return m_notes;
    }
    [[nodiscard]] const std::vector<DrawnRow>& rows() const { return m_rows; }
};

class ImageImpl;

class Image {
private:
    std::unique_ptr<ImageImpl> m_impl;

public:
    explicit Image(const DrawingInstructions& instructions);
    ~Image();
    Image(const Image&) = delete;
    Image(Image&& image) noexcept;
    Image& operator=(const Image&) = delete;
    Image& operator=(Image&& image) noexcept;

    void save(const char* filename) const;
};

#endif
