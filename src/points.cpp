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

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <type_traits>

#include "points.hpp"

namespace {
bool phrase_contains_pos(const StarPower& phrase, int position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

double song_tick_gap(int resolution, const Engine& engine)
{
    double quotient
        = resolution / static_cast<double>(engine.sust_points_per_beat());
    if (engine.round_tick_gap()) {
        quotient = std::floor(quotient);
    }
    return std::max(quotient, 1.0);
}

template <typename OutputIt>
void append_sustain_points(OutputIt points, int position, int sust_length,
                           int resolution, int chord_size,
                           const TimeConverter& converter, const Engine& engine)
{
    constexpr double HALF_RES_OFFSET = 0.5;

    const double float_res = resolution;
    double float_pos = position;
    double float_sust_len = sust_length;
    double tick_gap = song_tick_gap(resolution, engine);
    auto float_sust_ticks = sust_length / tick_gap;
    switch (engine.sustain_rounding()) {
    case SustainRoundingPolicy::RoundUp:
        float_sust_ticks = std::ceil(float_sust_ticks);
        break;
    case SustainRoundingPolicy::RoundToNearest:
        float_sust_ticks = std::round(float_sust_ticks);
        break;
    }
    auto sust_ticks = static_cast<int>(float_sust_ticks);
    if (engine.chords_multiply_sustains()) {
        tick_gap /= chord_size;
        sust_ticks *= chord_size;
    }

    while (float_sust_len > engine.burst_size() * resolution
           && sust_ticks > 0) {
        float_pos += tick_gap;
        float_sust_len -= tick_gap;
        const Beat beat {(float_pos - HALF_RES_OFFSET) / float_res};
        const auto meas = converter.beats_to_measures(beat);
        --sust_ticks;
        *points++ = {{beat, meas}, {beat, meas}, {beat, meas}, {}, 1, 1,
                     true,         false,        false};
    }
    if (sust_ticks > 0) {
        const Beat beat {(float_pos + HALF_RES_OFFSET) / float_res};
        const auto meas = converter.beats_to_measures(beat);
        *points++ = {{beat, meas}, {beat, meas}, {beat, meas}, {},   sust_ticks,
                     sust_ticks,   true,         false,        false};
    }
}

int get_chord_size(const Note& note, const DrumSettings& drum_settings)
{
    if (note.is_skipped_kick(drum_settings)) {
        return 0;
    }
    int note_count = 0;
    for (auto length : note.lengths) {
        if (length != -1) {
            ++note_count;
        }
    }
    return note_count;
}

template <typename OutputIt>
void append_note_points(std::vector<Note>::const_iterator note,
                        const std::vector<Note>& notes, OutputIt points,
                        int resolution, bool is_note_sp_ender,
                        bool is_unison_sp_ender, const TimeConverter& converter,
                        double squeeze, const Engine& engine,
                        const DrumSettings& drum_settings)
{
    auto note_value = engine.base_note_value();
    if (note->flags & FLAGS_DRUMS) {
        if (note->flags & FLAGS_CYMBAL) {
            note_value = engine.base_cymbal_value();
        }
        if (note->flags & (FLAGS_GHOST | FLAGS_ACCENT)) {
            note_value *= 2;
        }
    }
    const auto chord_size = get_chord_size(*note, drum_settings);
    const auto pos = note->position;
    const Beat beat {pos / static_cast<double>(resolution)};
    const auto meas = converter.beats_to_measures(beat);
    const auto note_seconds = converter.beats_to_seconds(beat);

    auto early_gap = std::numeric_limits<double>::infinity();
    if (note != notes.cbegin()) {
        const Beat prev_note_beat {std::prev(note)->position
                                   / static_cast<double>(resolution)};
        const auto prev_note_seconds
            = converter.beats_to_seconds(prev_note_beat);
        early_gap = (note_seconds - prev_note_seconds).value();
    }
    auto late_gap = std::numeric_limits<double>::infinity();
    if (std::next(note) != notes.cend()) {
        const Beat next_note_beat {std::next(note)->position
                                   / static_cast<double>(resolution)};
        const auto next_note_seconds
            = converter.beats_to_seconds(next_note_beat);
        late_gap = (next_note_seconds - note_seconds).value();
    }

    const Second early_window {engine.early_timing_window(early_gap, late_gap)
                               * squeeze};
    const Second late_window {engine.late_timing_window(early_gap, late_gap)
                              * squeeze};

    const auto early_beat
        = converter.seconds_to_beats(note_seconds - early_window);
    const auto early_meas = converter.beats_to_measures(early_beat);
    const auto late_beat
        = converter.seconds_to_beats(note_seconds + late_window);
    const auto late_meas = converter.beats_to_measures(late_beat);
    *points++
        = {{beat, meas}, {early_beat, early_meas}, {late_beat, late_meas},
           {},           note_value * chord_size,  note_value * chord_size,
           false,        is_note_sp_ender,         is_unison_sp_ender};

    auto min_length = std::numeric_limits<int>::max();
    auto max_length = 0;
    for (auto length : note->lengths) {
        if (length == -1) {
            continue;
        }
        min_length = std::min(length, min_length);
        max_length = std::max(length, max_length);
    }

    if (min_length == max_length || engine.merge_uneven_sustains()) {
        append_sustain_points(points, pos, min_length, resolution, chord_size,
                              converter, engine);
    } else {
        for (auto length : note->lengths) {
            if (length != -1) {
                append_sustain_points(points, pos, length, resolution,
                                      chord_size, converter, engine);
            }
        }
    }
}

std::vector<Point>::iterator closest_point(std::vector<Point>& points,
                                           Beat fill_end)
{
    assert(!points.empty()); // NOLINT
    auto nearest = points.begin();
    auto best_gap = std::abs((nearest->position.beat - fill_end).value());
    for (auto p = std::next(points.begin()); p < points.end(); ++p) {
        if (p->position.beat <= nearest->position.beat) {
            continue;
        }
        const auto new_gap = std::abs((p->position.beat - fill_end).value());
        if (new_gap > best_gap) {
            break;
        }
        nearest = p;
        best_gap = new_gap;
    }
    return nearest;
}

void add_drum_activation_points(const NoteTrack& track,
                                const TimeConverter& converter,
                                std::vector<Point>& points)
{
    if (points.empty()) {
        return;
    }
    for (auto fill : track.drum_fills()) {
        const Beat fill_start {fill.position
                               / static_cast<double>(track.resolution())};
        const Beat fill_end {(fill.position + fill.length)
                             / static_cast<double>(track.resolution())};
        const auto best_point = closest_point(points, fill_end);
        bool has_non_kick = false;
        for (const auto& note : track.notes()) {
            if (note.is_kick_note()) {
                continue;
            }
            const Beat note_position {
                note.position / static_cast<double>(track.resolution())};
            if (note_position < best_point->position.beat) {
                continue;
            }
            if (note_position > best_point->position.beat) {
                break;
            }
            has_non_kick = true;
            break;
        }
        if (has_non_kick) {
            best_point->fill_start = converter.beats_to_seconds(fill_start);
        }
    }
}

void shift_points_by_video_lag(std::vector<Point>& points,
                               const TimeConverter& converter, Second video_lag)
{
    const auto add_video_lag = [&](auto& position) {
        auto seconds = converter.beats_to_seconds(position.beat);
        seconds += video_lag;
        position.beat = converter.seconds_to_beats(seconds);
        position.measure = converter.beats_to_measures(position.beat);
    };

    for (auto& point : points) {
        if (point.is_hold_point) {
            continue;
        }
        add_video_lag(point.position);
        add_video_lag(point.hit_window_start);
        add_video_lag(point.hit_window_end);
    }
}

template <typename P>
std::vector<PointPtr> next_matching_vector(const std::vector<Point>& points,
                                           P predicate)
{
    if (points.empty()) {
        return {};
    }
    std::vector<PointPtr> next_matching_points;
    auto next_matching_point = points.cend();
    for (auto p = std::prev(points.cend());; --p) {
        if (predicate(*p)) {
            next_matching_point = p;
        }
        next_matching_points.push_back(next_matching_point);
        // We can't have the loop condition be p >= points.cbegin() because
        // decrementing past .begin() is undefined behaviour.
        if (p == points.cbegin()) {
            break;
        }
    }
    std::reverse(next_matching_points.begin(), next_matching_points.end());
    return next_matching_points;
}

template <typename T>
std::string to_guitar_colour_string(
    const std::vector<T>& colours,
    const std::vector<std::tuple<T, std::string>>& colour_names)
{
    std::string colour_string;

    for (const auto& [colour, string] : colour_names) {
        if (std::find(colours.cbegin(), colours.cend(), colour)
            != colours.cend()) {
            colour_string += string;
        }
    }

    return colour_string;
}

void apply_multiplier(std::vector<Point>& points, const Engine& engine)
{
    constexpr int COMBO_PER_MULTIPLIER_LEVEL = 10;

    auto combo = 0;
    for (auto& point : points) {
        if (!point.is_hold_point) {
            ++combo;
        }
        auto multiplier = std::min(combo / COMBO_PER_MULTIPLIER_LEVEL + 1,
                                   engine.max_multiplier());
        if (!point.is_hold_point && engine.delayed_multiplier()) {
            multiplier = std::min((combo - 1) / COMBO_PER_MULTIPLIER_LEVEL + 1,
                                  engine.max_multiplier());
        }
        point.value *= multiplier;
    }
}

std::vector<Point> unmultiplied_points(const NoteTrack& track,
                                       const TimeConverter& converter,
                                       const std::vector<int>& unison_phrases,
                                       const SqueezeSettings& squeeze_settings,
                                       const DrumSettings& drum_settings,
                                       const Engine& engine)
{
    const auto& notes = track.notes();
    const auto has_relevant_bre = track.bre().has_value() && engine.has_bres();

    std::vector<Point> points;
    auto current_phrase = track.sp_phrases().cbegin();

    for (auto p = notes.cbegin(); p != notes.cend();) {
        if (track.track_type() == TrackType::Drums) {
            if (p->is_skipped_kick(drum_settings)) {
                ++p;
                continue;
            }
        }
        if (has_relevant_bre && p->position >= track.bre()->start) {
            break;
        }
        const auto search_start
            = track.track_type() == TrackType::Drums ? std::next(p) : p;
        const auto q = std::find_if_not(
            search_start, notes.cend(), [=](const auto& note) {
                if (track.track_type() == TrackType::Drums) {
                    return note.is_skipped_kick(drum_settings);
                }
                return note.position == p->position;
            });
        auto is_note_sp_ender = false;
        auto is_unison_sp_ender = false;
        if (current_phrase != track.sp_phrases().cend()
            && phrase_contains_pos(*current_phrase, p->position)
            && ((q == notes.cend())
                || !phrase_contains_pos(*current_phrase, q->position))) {
            is_note_sp_ender = true;
            if (engine.has_unison_bonuses()
                && std::find(unison_phrases.cbegin(), unison_phrases.cend(),
                             current_phrase->position)
                    != unison_phrases.cend()) {
                is_unison_sp_ender = true;
            }
            ++current_phrase;
        }
        append_note_points(p, notes, std::back_inserter(points),
                           track.resolution(), is_note_sp_ender,
                           is_unison_sp_ender, converter,
                           squeeze_settings.squeeze, engine, drum_settings);
        p = q;
    }

    std::stable_sort(points.begin(), points.end(),
                     [](const auto& x, const auto& y) {
                         return x.position.beat < y.position.beat;
                     });

    return points;
}

std::vector<Point> non_drum_points(const NoteTrack& track,
                                   const TimeConverter& converter,
                                   const std::vector<int>& unison_phrases,
                                   const SqueezeSettings& squeeze_settings,
                                   const Engine& engine)
{
    auto points = unmultiplied_points(track, converter, unison_phrases,
                                      squeeze_settings,
                                      DrumSettings::default_settings(), engine);
    apply_multiplier(points, engine);
    shift_points_by_video_lag(points, converter, squeeze_settings.video_lag);
    return points;
}

std::string colours_string(const Note& note)
{
    std::string colours;

    if ((note.flags & FLAGS_FIVE_FRET_GUITAR) != 0U) {
        const std::array<std::string, 6> COLOUR_NAMES {"G", "R", "Y",
                                                       "B", "O", "open"};
        for (auto i = 0U; i < COLOUR_NAMES.size(); ++i) {
            if (note.lengths.at(i) != -1) {
                colours += COLOUR_NAMES.at(i);
            }
        }
        return colours;
    }
    if ((note.flags & FLAGS_SIX_FRET_GUITAR) != 0U) {
        const std::array<std::string, 7> COLOUR_NAMES {"W1", "W2", "W3",  "B1",
                                                       "B2", "B3", "open"};
        for (auto i = 0U; i < COLOUR_NAMES.size(); ++i) {
            if (note.lengths.at(i) != -1) {
                colours += COLOUR_NAMES.at(i);
            }
        }
    }
    if ((note.flags & FLAGS_DRUMS) != 0U) {
        const std::array<std::string, 6> COLOUR_NAMES {"R", "Y",    "B",
                                                       "G", "kick", "kick"};
        for (auto i = 0U; i < COLOUR_NAMES.size(); ++i) {
            if (note.lengths.at(i) != -1) {
                colours += COLOUR_NAMES.at(i);
            }
        }
        if ((note.flags & FLAGS_GHOST) != 0U) {
            colours += " ghost";
        }
        if ((note.flags & FLAGS_ACCENT) != 0U) {
            colours += " accent";
        }
        if ((note.flags & FLAGS_CYMBAL) != 0U) {
            colours += " cymbal";
        }
    }

    return colours;
}

std::vector<PointPtr>
first_after_current_sp_vector(const std::vector<Point>& points,
                              const NoteTrack& track, const Engine& engine)
{
    std::vector<PointPtr> results;
    auto current_sp = track.sp_phrases().cbegin();
    for (auto p = points.cbegin(); p < points.cend();) {
        current_sp
            = std::find_if(current_sp, track.sp_phrases().cend(), [&](auto sp) {
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
        const auto q = std::find_if(std::next(p), points.cend(), [&](auto pt) {
            return pt.position.beat >= sp_end;
        });
        while (p < q) {
            results.push_back(q);
            ++p;
        }
    }
    return results;
}

std::vector<std::string> note_colours(const std::vector<Note>& notes,
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
}

std::vector<Point> PointSet::points_from_track(
    const NoteTrack& track, const TimeConverter& converter,
    const std::vector<int>& unison_phrases,
    const SqueezeSettings& squeeze_settings, const DrumSettings& drum_settings,
    const Engine& engine)
{
    if (track.track_type() != TrackType::Drums) {
        return non_drum_points(track, converter, unison_phrases,
                               squeeze_settings, engine);
    }
    auto points = unmultiplied_points(track, converter, unison_phrases,
                                      squeeze_settings, drum_settings, engine);
    add_drum_activation_points(track, converter, points);
    apply_multiplier(points, engine);
    shift_points_by_video_lag(points, converter, squeeze_settings.video_lag);
    return points;
}

std::vector<PointPtr>
PointSet::next_non_hold_vector(const std::vector<Point>& points)
{
    return next_matching_vector(points,
                                [](const auto& p) { return !p.is_hold_point; });
}

std::vector<PointPtr>
PointSet::next_sp_note_vector(const std::vector<Point>& points)
{
    return next_matching_vector(
        points, [](const auto& p) { return p.is_sp_granting_note; });
}

std::vector<int> PointSet::score_totals(const std::vector<Point>& points)
{
    std::vector<int> scores;
    scores.reserve(points.size() + 1);
    scores.push_back(0);
    auto sum = 0;
    for (const auto& p : points) {
        sum += p.value;
        scores.push_back(sum);
    }
    return scores;
}

std::vector<std::tuple<Position, int>>
PointSet::solo_boosts_from_solos(const std::vector<Solo>& solos, int resolution,
                                 const TimeConverter& converter)
{
    std::vector<std::tuple<Position, int>> solo_boosts;
    solo_boosts.reserve(solos.size());
    for (const auto& solo : solos) {
        Beat end_beat {solo.end / static_cast<double>(resolution)};
        Measure end_meas = converter.beats_to_measures(end_beat);
        Position end_pos {end_beat, end_meas};
        solo_boosts.emplace_back(end_pos, solo.value);
    }
    return solo_boosts;
}

PointSet::PointSet(const NoteTrack& track, const TimeConverter& converter,
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

PointPtr PointSet::first_after_current_phrase(PointPtr point) const
{
    const auto index
        = static_cast<std::size_t>(std::distance(m_points.cbegin(), point));
    return m_first_after_current_sp[index];
}

PointPtr PointSet::next_non_hold_point(PointPtr point) const
{
    const auto index
        = static_cast<std::size_t>(std::distance(m_points.cbegin(), point));
    return m_next_non_hold_point[index];
}

PointPtr PointSet::next_sp_granting_note(PointPtr point) const
{
    const auto index
        = static_cast<std::size_t>(std::distance(m_points.cbegin(), point));
    return m_next_sp_granting_note[index];
}

int PointSet::range_score(PointPtr start, PointPtr end) const
{
    const auto start_index
        = static_cast<std::size_t>(std::distance(m_points.cbegin(), start));
    const auto end_index
        = static_cast<std::size_t>(std::distance(m_points.cbegin(), end));
    return m_cumulative_score_totals[end_index]
        - m_cumulative_score_totals[start_index];
}
