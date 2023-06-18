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

#ifndef CHOPT_POINTS_HPP
#define CHOPT_POINTS_HPP

#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "drumsettings.hpp"
#include "engine.hpp"
#include "settings.hpp"
#include "songparts.hpp"
#include "sptimemap.hpp"
#include "time.hpp"

// fill_start is used for Drums, giving the start of the fill that makes a point
// an activation note if it is one, or nullopt otherwise.
struct Point {
    SpPosition position;
    SpPosition hit_window_start;
    SpPosition hit_window_end;
    std::optional<Second> fill_start;
    int value;
    int base_value;
    bool is_hold_point;
    bool is_sp_granting_note;
    bool is_unison_sp_granting_note;
};

using PointPtr = std::vector<Point>::const_iterator;

class PointSet {
private:
    std::vector<Point> m_points;
    std::vector<PointPtr> m_first_after_current_sp;
    std::vector<PointPtr> m_next_non_hold_point;
    std::vector<PointPtr> m_next_sp_granting_note;
    std::vector<std::tuple<SpPosition, int>> m_solo_boosts;
    std::vector<int> m_cumulative_score_totals;
    Second m_video_lag;
    std::vector<std::string> m_colours;

public:
    PointSet(const NoteTrack& track, const SpTimeMap& time_map,
             const std::vector<Tick>& unison_phrases,
             const SqueezeSettings& squeeze_settings,
             const SightRead::DrumSettings& drum_settings,
             const Engine& engine);
    [[nodiscard]] PointPtr cbegin() const { return m_points.cbegin(); }
    [[nodiscard]] PointPtr cend() const { return m_points.cend(); }
    // Designed for engines without SP overlap, so the next activation is not
    // using part of the given phrase. If the point is not part of a phrase, or
    // the engine supports overlap, then this just returns the next point.
    [[nodiscard]] PointPtr first_after_current_phrase(PointPtr point) const;
    [[nodiscard]] PointPtr next_non_hold_point(PointPtr point) const;
    [[nodiscard]] PointPtr next_sp_granting_note(PointPtr point) const;
    [[nodiscard]] std::string colour_set(PointPtr point) const
    {
        return m_colours[static_cast<std::size_t>(
            std::distance(m_points.cbegin(), point))];
    }
    // Get the combined score of all points that are >= start and < end.
    [[nodiscard]] int range_score(PointPtr start, PointPtr end) const;
    [[nodiscard]] const std::vector<std::tuple<SpPosition, int>>&
    solo_boosts() const
    {
        return m_solo_boosts;
    }
    [[nodiscard]] Second video_lag() const { return m_video_lag; }
};

#endif
