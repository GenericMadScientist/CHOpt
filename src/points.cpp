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
bool phrase_contains_pos(const SightRead::StarPower& phrase,
                         SightRead::Tick position)
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
void append_sustain_points(OutputIt points, SightRead::Tick position,
                           SightRead::Tick sust_length, int resolution,
                           int chord_size, const SpTimeMap& time_map,
                           const Engine& engine)
{
    constexpr double HALF_RES_OFFSET = 0.5;

    const double float_res = resolution;
    double float_pos = position.value();
    double float_sust_len = sust_length.value();
    double tick_gap = song_tick_gap(resolution, engine);
    auto float_sust_ticks = sust_length.value() / tick_gap;
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
        const SightRead::Beat beat {(float_pos - HALF_RES_OFFSET) / float_res};
        const auto meas = time_map.to_sp_measures(beat);
        --sust_ticks;
        *points++ = {{beat, meas}, {beat, meas}, {beat, meas}, {}, 1, 1,
                     true,         false,        false};
    }
    if (sust_ticks > 0) {
        const SightRead::Beat beat {(float_pos + HALF_RES_OFFSET) / float_res};
        const auto meas = time_map.to_sp_measures(beat);
        *points++ = {{beat, meas}, {beat, meas}, {beat, meas}, {},   sust_ticks,
                     sust_ticks,   true,         false,        false};
    }
}

int get_chord_size(const SightRead::Note& note,
                   const SightRead::DrumSettings& drum_settings)
{
    if (note.is_skipped_kick(drum_settings)) {
        return 0;
    }
    int note_count = 0;
    for (auto length : note.lengths) {
        if (length != SightRead::Tick {-1}) {
            ++note_count;
        }
    }
    return note_count;
}

template <typename OutputIt>
void append_note_points(std::vector<SightRead::Note>::const_iterator note,
                        const std::vector<SightRead::Note>& notes,
                        OutputIt points, const SpTimeMap& time_map,
                        int resolution, bool is_note_sp_ender,
                        bool is_unison_sp_ender, double squeeze,
                        const Engine& engine,
                        const SightRead::DrumSettings& drum_settings)
{
    auto note_value = engine.base_note_value();
    if (note->flags & SightRead::FLAGS_DRUMS) {
        if (note->flags & SightRead::FLAGS_CYMBAL) {
            note_value = engine.base_cymbal_value();
        }
        if (note->flags & (SightRead::FLAGS_GHOST | SightRead::FLAGS_ACCENT)) {
            note_value *= 2;
        }
    }
    const auto chord_size = get_chord_size(*note, drum_settings);
    const auto pos = note->position;
    const auto beat = time_map.to_beats(pos);
    const auto meas = time_map.to_sp_measures(beat);
    const auto note_seconds = time_map.to_seconds(beat);

    auto early_gap = std::numeric_limits<double>::infinity();
    if (note != notes.cbegin()) {
        const auto prev_note_beat
            = time_map.to_beats(std::prev(note)->position);
        const auto prev_note_seconds = time_map.to_seconds(prev_note_beat);
        early_gap = (note_seconds - prev_note_seconds).value();
    }
    auto late_gap = std::numeric_limits<double>::infinity();
    if (std::next(note) != notes.cend()) {
        const auto next_note_beat
            = time_map.to_beats(std::next(note)->position);
        const auto next_note_seconds = time_map.to_seconds(next_note_beat);
        late_gap = (next_note_seconds - note_seconds).value();
    }

    const SightRead::Second early_window {
        engine.early_timing_window(early_gap, late_gap) * squeeze};
    const SightRead::Second late_window {
        engine.late_timing_window(early_gap, late_gap) * squeeze};

    const auto early_beat = time_map.to_beats(note_seconds - early_window);
    const auto early_meas = time_map.to_sp_measures(early_beat);
    const auto late_beat = time_map.to_beats(note_seconds + late_window);
    const auto late_meas = time_map.to_sp_measures(late_beat);
    *points++
        = {{beat, meas}, {early_beat, early_meas}, {late_beat, late_meas},
           {},           note_value * chord_size,  note_value * chord_size,
           false,        is_note_sp_ender,         is_unison_sp_ender};

    SightRead::Tick min_length {std::numeric_limits<int>::max()};
    SightRead::Tick max_length {0};
    for (auto length : note->lengths) {
        if (length == SightRead::Tick {-1}) {
            continue;
        }
        min_length = std::min(length, min_length);
        max_length = std::max(length, max_length);
    }

    if (min_length == max_length || engine.merge_uneven_sustains()) {
        append_sustain_points(points, pos, min_length, resolution, chord_size,
                              time_map, engine);
    } else {
        for (auto length : note->lengths) {
            if (length != SightRead::Tick {-1}) {
                append_sustain_points(points, pos, length, resolution,
                                      chord_size, time_map, engine);
            }
        }
    }
}

std::vector<Point>::iterator closest_point(std::vector<Point>& points,
                                           SightRead::Beat fill_end)
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

void add_drum_activation_points(const SightRead::NoteTrack& track,
                                std::vector<Point>& points)
{
    if (points.empty()) {
        return;
    }
    const auto& tempo_map = track.global_data().tempo_map();
    for (auto fill : track.drum_fills()) {
        const auto fill_start = tempo_map.to_beats(fill.position);
        const auto fill_end = tempo_map.to_beats(fill.position + fill.length);
        const auto best_point = closest_point(points, fill_end);
        best_point->fill_start = tempo_map.to_seconds(fill_start);
    }
}

void shift_points_by_video_lag(std::vector<Point>& points,
                               const SpTimeMap& time_map,
                               SightRead::Second video_lag)
{
    const auto add_video_lag = [&](auto& position) {
        auto seconds = time_map.to_seconds(position.beat);
        seconds += video_lag;
        position.beat = time_map.to_beats(seconds);
        position.sp_measure = time_map.to_sp_measures(position.beat);
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

bool has_split_notes(SightRead::TrackType track_type)
{
    return track_type == SightRead::TrackType::Drums
        || track_type == SightRead::TrackType::FortniteFestival;
}

std::vector<Point> unmultiplied_points(
    const SightRead::NoteTrack& track, const SpTimeMap& time_map,
    const std::vector<SightRead::Tick>& unison_phrases,
    const SqueezeSettings& squeeze_settings,
    const SightRead::DrumSettings& drum_settings, const Engine& engine)
{
    const auto& notes = track.notes();
    const auto has_relevant_bre = track.bre().has_value() && engine.has_bres();

    std::vector<Point> points;
    auto current_phrase = track.sp_phrases().cbegin();

    for (auto p = notes.cbegin(); p != notes.cend();) {
        if (track.track_type() == SightRead::TrackType::Drums) {
            if (p->is_skipped_kick(drum_settings)) {
                ++p;
                continue;
            }
        }
        if (has_relevant_bre && p->position >= track.bre()->start) {
            break;
        }
        const auto search_start
            = has_split_notes(track.track_type()) ? std::next(p) : p;
        const auto q = std::find_if_not(
            search_start, notes.cend(), [=](const auto& note) {
                if (track.track_type() == SightRead::TrackType::Drums) {
                    return note.is_skipped_kick(drum_settings);
                }
                if (track.track_type()
                    == SightRead::TrackType::FortniteFestival) {
                    return false;
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
        append_note_points(p, notes, std::back_inserter(points), time_map,
                           track.global_data().resolution(), is_note_sp_ender,
                           is_unison_sp_ender, squeeze_settings.squeeze, engine,
                           drum_settings);
        p = q;
    }

    std::stable_sort(points.begin(), points.end(),
                     [](const auto& x, const auto& y) {
                         return x.position.beat < y.position.beat;
                     });

    return points;
}

std::vector<Point>
non_drum_points(const SightRead::NoteTrack& track, const SpTimeMap& time_map,
                const std::vector<SightRead::Tick>& unison_phrases,
                const SqueezeSettings& squeeze_settings, const Engine& engine)
{
    auto points = unmultiplied_points(
        track, time_map, unison_phrases, squeeze_settings,
        SightRead::DrumSettings::default_settings(), engine);
    apply_multiplier(points, engine);
    shift_points_by_video_lag(points, time_map, squeeze_settings.video_lag);
    return points;
}

std::string colours_string(const SightRead::Note& note)
{
    std::string colours;

    if ((note.flags & SightRead::FLAGS_FIVE_FRET_GUITAR) != 0U) {
        const std::array<std::string, 6> COLOUR_NAMES {"G", "R", "Y",
                                                       "B", "O", "open"};
        for (auto i = 0U; i < COLOUR_NAMES.size(); ++i) {
            if (note.lengths.at(i) != SightRead::Tick {-1}) {
                colours += COLOUR_NAMES.at(i);
            }
        }
        return colours;
    }
    if ((note.flags & SightRead::FLAGS_SIX_FRET_GUITAR) != 0U) {
        const std::array<std::string, 7> COLOUR_NAMES {"W1", "W2", "W3",  "B1",
                                                       "B2", "B3", "open"};
        for (auto i = 0U; i < COLOUR_NAMES.size(); ++i) {
            if (note.lengths.at(i) != SightRead::Tick {-1}) {
                colours += COLOUR_NAMES.at(i);
            }
        }
    }
    if ((note.flags & SightRead::FLAGS_DRUMS) != 0U) {
        const std::array<std::string, 6> COLOUR_NAMES {"R", "Y",    "B",
                                                       "G", "kick", "kick"};
        for (auto i = 0U; i < COLOUR_NAMES.size(); ++i) {
            if (note.lengths.at(i) != SightRead::Tick {-1}) {
                colours += COLOUR_NAMES.at(i);
            }
        }
        if ((note.flags & SightRead::FLAGS_GHOST) != 0U) {
            colours += " ghost";
        }
        if ((note.flags & SightRead::FLAGS_ACCENT) != 0U) {
            colours += " accent";
        }
        if ((note.flags & SightRead::FLAGS_CYMBAL) != 0U) {
            colours += " cymbal";
        }
    }

    return colours;
}

std::vector<PointPtr>
first_after_current_sp_vector(const std::vector<Point>& points,
                              const SightRead::NoteTrack& track,
                              const Engine& engine)
{
    std::vector<PointPtr> results;
    const auto& tempo_map = track.global_data().tempo_map();
    auto current_sp = track.sp_phrases().cbegin();
    for (auto p = points.cbegin(); p < points.cend();) {
        current_sp
            = std::find_if(current_sp, track.sp_phrases().cend(), [&](auto sp) {
                  return tempo_map.to_beats(sp.position + sp.length)
                      > p->position.beat;
              });
        SightRead::Beat sp_start {std::numeric_limits<double>::infinity()};
        SightRead::Beat sp_end {std::numeric_limits<double>::infinity()};
        if (current_sp != track.sp_phrases().cend()) {
            sp_start = tempo_map.to_beats(current_sp->position);
            sp_end
                = tempo_map.to_beats(current_sp->position + current_sp->length);
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

std::vector<std::string> note_colours(const std::vector<SightRead::Note>& notes,
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

std::vector<Point>
points_from_track(const SightRead::NoteTrack& track, const SpTimeMap& time_map,
                  const std::vector<SightRead::Tick>& unison_phrases,
                  const SqueezeSettings& squeeze_settings,
                  const SightRead::DrumSettings& drum_settings,
                  const Engine& engine)
{
    if (track.track_type() != SightRead::TrackType::Drums) {
        return non_drum_points(track, time_map, unison_phrases,
                               squeeze_settings, engine);
    }
    auto points = unmultiplied_points(track, time_map, unison_phrases,
                                      squeeze_settings, drum_settings, engine);
    add_drum_activation_points(track, points);
    apply_multiplier(points, engine);
    shift_points_by_video_lag(points, time_map, squeeze_settings.video_lag);
    return points;
}

std::vector<PointPtr> next_non_hold_vector(const std::vector<Point>& points)
{
    return next_matching_vector(points,
                                [](const auto& p) { return !p.is_hold_point; });
}

std::vector<PointPtr> next_sp_note_vector(const std::vector<Point>& points)
{
    return next_matching_vector(
        points, [](const auto& p) { return p.is_sp_granting_note; });
}

std::vector<int> score_totals(const std::vector<Point>& points)
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

std::vector<std::tuple<SpPosition, int>>
solo_boosts_from_solos(const std::vector<SightRead::Solo>& solos,
                       const SpTimeMap& time_map)
{
    std::vector<std::tuple<SpPosition, int>> solo_boosts;
    solo_boosts.reserve(solos.size());
    for (const auto& solo : solos) {
        const auto end_beat = time_map.to_beats(solo.end);
        const SpMeasure end_meas = time_map.to_sp_measures(end_beat);
        const SpPosition end_pos {end_beat, end_meas};
        solo_boosts.emplace_back(end_pos, solo.value);
    }
    return solo_boosts;
}
}

PointSet::PointSet(const SightRead::NoteTrack& track, const SpTimeMap& time_map,
                   const std::vector<SightRead::Tick>& unison_phrases,
                   const SqueezeSettings& squeeze_settings,
                   const SightRead::DrumSettings& drum_settings,
                   const Engine& engine)
    : m_points {points_from_track(track, time_map, unison_phrases,
                                  squeeze_settings, drum_settings, engine)}
    , m_first_after_current_sp {first_after_current_sp_vector(m_points, track,
                                                              engine)}
    , m_next_non_hold_point {next_non_hold_vector(m_points)}
    , m_next_sp_granting_note {next_sp_note_vector(m_points)}
    , m_solo_boosts {solo_boosts_from_solos(track.solos(drum_settings),
                                            time_map)}
    , m_cumulative_score_totals {score_totals(m_points)}
    , m_video_lag {squeeze_settings.video_lag}
    , m_colours {note_colours(track.notes(), m_points)}
{
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
