/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022 Raymond Wright
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

#include "engine.hpp"
#include "settings.hpp"
#include "songparts.hpp"
#include "time.hpp"
#include "timeconverter.hpp"

// fill_start is used for Drums, giving the start of the fill that makes a point
// an activation note if it is one, or nullopt otherwise.
struct Point {
    Position position;
    Position hit_window_start;
    Position hit_window_end;
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
    std::vector<std::tuple<Position, int>> m_solo_boosts;
    std::vector<int> m_cumulative_score_totals;
    Second m_video_lag;
    std::vector<std::string> m_colours;

    static std::vector<Point>
    points_from_track(const NoteTrack& track, const TimeConverter& converter,
                      const std::vector<int>& unison_phrases,
                      const SqueezeSettings& squeeze_settings,
                      const DrumSettings& drum_settings, const Engine& engine);

    static std::vector<PointPtr>
    next_non_hold_vector(const std::vector<Point>& points);
    static std::vector<PointPtr>
    next_sp_note_vector(const std::vector<Point>& points);
    static std::vector<int> score_totals(const std::vector<Point>& points);
    static std::vector<std::tuple<Position, int>>
    solo_boosts_from_solos(const std::vector<Solo>& solos, int resolution,
                           const TimeConverter& converter);

    static std::string colours_string(const Note& note);

    static std::vector<PointPtr>
    first_after_current_sp_vector(const std::vector<Point>& points,
                                  const NoteTrack& track, const Engine& engine)
    {
        std::vector<PointPtr> results;
        auto current_sp = track.sp_phrases().cbegin();
        for (auto p = points.cbegin(); p < points.cend();) {
            current_sp = std::find_if(
                current_sp, track.sp_phrases().cend(), [&](auto sp) {
                    return Beat((sp.position + sp.length)
                                / static_cast<double>(track.resolution()))
                        > p->position.beat;
                });
            Beat sp_start {std::numeric_limits<double>::infinity()};
            Beat sp_end {std::numeric_limits<double>::infinity()};
            if (current_sp != track.sp_phrases().cend()) {
                sp_start = Beat {current_sp->position
                                 / static_cast<double>(track.resolution())};
                sp_end = Beat {(current_sp->position + current_sp->length)
                               / static_cast<double>(track.resolution())};
            }
            if (p->position.beat < sp_start || engine.overlaps()) {
                results.push_back(++p);
                continue;
            }
            const auto q
                = std::find_if(std::next(p), points.cend(), [&](auto pt) {
                      return pt.position.beat >= sp_end;
                  });
            while (p < q) {
                results.push_back(q);
                ++p;
            }
        }
        return results;
    }

    static std::vector<std::string>
    note_colours(const std::vector<Note>& notes,
                 const std::vector<Point>& points)
    {
        std::vector<std::string> colours;
        colours.reserve(points.size());
        auto note_ptr = notes.cbegin();
        for (const auto& p : points) {
            if (p.is_hold_point) {
                colours.emplace_back("");
                continue;
            }
            colours.push_back(colours_string(*note_ptr));
            ++note_ptr;
        }
        return colours;
    }

public:
    PointSet(const NoteTrack& track, const TimeConverter& converter,
             const std::vector<int>& unison_phrases,
             const SqueezeSettings& squeeze_settings,
             const DrumSettings& drum_settings, const Engine& engine)
        : m_video_lag {squeeze_settings.video_lag}
    {
        m_points = points_from_track(track, converter, unison_phrases,
                                     squeeze_settings, drum_settings, engine);
        m_solo_boosts = solo_boosts_from_solos(track.solos(drum_settings),
                                               track.resolution(), converter);
        m_first_after_current_sp
            = first_after_current_sp_vector(m_points, track, engine);
        m_next_non_hold_point = next_non_hold_vector(m_points);
        m_next_sp_granting_note = next_sp_note_vector(m_points);
        m_cumulative_score_totals = score_totals(m_points);
        m_colours = note_colours(track.notes(), m_points);
    }
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
    [[nodiscard]] const std::vector<std::tuple<Position, int>>&
    solo_boosts() const
    {
        return m_solo_boosts;
    }
    [[nodiscard]] Second video_lag() const { return m_video_lag; }
};

#endif
