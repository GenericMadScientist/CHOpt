/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021 Raymond Wright
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

static bool phrase_contains_pos(const StarPower& phrase, int position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

template <typename OutputIt>
static void
append_sustain_points(OutputIt points, int position, int sust_length,
                      int resolution, int chord_size,
                      const TimeConverter& converter, const Engine& engine)
{
    constexpr double HALF_RES_OFFSET = 0.5;

    const double float_res = resolution;
    double float_pos = position;
    double float_sust_len = sust_length;
    double tick_gap = std::max(resolution / engine.sust_points_per_beat(), 1);
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
        *points++ = {{beat, meas}, {beat, meas}, {beat, meas}, 1,    1,
                     true,         false,        false,        false};
    }
    if (sust_ticks > 0) {
        const Beat beat {(float_pos + HALF_RES_OFFSET) / float_res};
        const auto meas = converter.beats_to_measures(beat);
        *points++
            = {{beat, meas}, {beat, meas}, {beat, meas}, sust_ticks, sust_ticks,
               true,         false,        false,        false};
    }
}

static bool skip_kick(DrumNoteColour colour, bool enable_double_kick,
                      bool disable_kick)
{
    if (colour == DrumNoteColour::Kick) {
        return disable_kick;
    }
    if (colour == DrumNoteColour::DoubleKick) {
        return !enable_double_kick;
    }
    return false;
}

template <typename InputIt>
static int get_chord_size(InputIt first, InputIt last, bool enable_double_kick,
                          bool disable_kick)
{
    int note_count = 0;
    using T = decltype(first->colour);
    for (; first < last; ++first) {
        if constexpr (std::is_same_v<T, DrumNoteColour>) {
            if (!skip_kick(first->colour, enable_double_kick, disable_kick)) {
                ++note_count;
            }
        } else {
            (void)enable_double_kick;
            (void)disable_kick;
            ++note_count;
        }
    }
    return note_count;
}

template <typename InputIt, typename OutputIt>
static void append_note_points(InputIt first, InputIt last, InputIt note_end,
                               OutputIt points, int resolution,
                               bool is_note_sp_ender, bool is_unison_sp_ender,
                               const TimeConverter& converter, double squeeze,
                               const Engine& engine, bool enable_double_kick,
                               bool disable_kick)
{
    assert(first != last); // NOLINT

    const auto EARLY_WINDOW = Second(engine.timing_window() * squeeze);
    const auto BASE_LATE_WINDOW = Second(engine.timing_window());

    const auto note_value = engine.base_note_value();
    const auto chord_size
        = get_chord_size(first, last, enable_double_kick, disable_kick);
    auto pos = first->position;
    const Beat beat {pos / static_cast<double>(resolution)};
    const auto meas = converter.beats_to_measures(beat);
    const auto note_seconds = converter.beats_to_seconds(beat);
    const auto early_beat
        = converter.seconds_to_beats(note_seconds - EARLY_WINDOW);
    const auto early_meas = converter.beats_to_measures(early_beat);
    auto late_seconds = note_seconds + BASE_LATE_WINDOW;
    if (engine.restricted_back_end() && last != note_end) {
        const Beat next_note_beat {last->position
                                   / static_cast<double>(resolution)};
        const auto next_note_seconds
            = converter.beats_to_seconds(next_note_beat);
        const auto midpoint_seconds = (note_seconds + next_note_seconds) * 0.5;
        late_seconds = std::min(late_seconds, midpoint_seconds);
    }
    const auto late_beat = converter.seconds_to_beats(
        note_seconds * (1.0 - squeeze) + late_seconds * squeeze);
    const auto late_meas = converter.beats_to_measures(late_beat);
    *points++ = {{beat, meas},
                 {early_beat, early_meas},
                 {late_beat, late_meas},
                 note_value * chord_size,
                 note_value * chord_size,
                 false,
                 is_note_sp_ender,
                 is_unison_sp_ender,
                 false};

    auto [min_iter, max_iter]
        = std::minmax_element(first, last, [](const auto& x, const auto& y) {
              return x.length < y.length;
          });
    if (min_iter->length == max_iter->length
        || engine.merge_uneven_sustains()) {
        append_sustain_points(points, pos, min_iter->length, resolution,
                              chord_size, converter, engine);
    } else {
        for (auto p = first; p < last; ++p) {
            append_sustain_points(points, pos, p->length, resolution,
                                  chord_size, converter, engine);
        }
    }
}

static bool is_kick_colour(DrumNoteColour colour)
{
    return colour == DrumNoteColour::Kick
        || colour == DrumNoteColour::DoubleKick;
}

static void add_drum_activation_points(const NoteTrack<DrumNoteColour>& track,
                                       std::vector<Point>& points)
{
    for (auto fill : track.drum_fills()) {
        const Beat fill_start {fill.position
                               / static_cast<double>(track.resolution())};
        const Beat fill_end {(fill.position + fill.length)
                             / static_cast<double>(track.resolution())};
        const auto earliest
            = std::find_if(points.begin(), points.end(), [&](auto p) {
                  return p.position.beat >= fill_start;
              });
        const auto earliest_after
            = std::find_if(earliest, points.end(),
                           [&](auto p) { return p.position.beat > fill_end; });
        if (earliest == earliest_after) {
            continue;
        }
        const auto last_note = std::prev(std::find_if(
            track.notes().cbegin(), track.notes().cend(), [&](auto n) {
                return n.position > (fill.position + fill.length);
            }));
        auto first_note = last_note;
        bool has_non_kick = !is_kick_colour(last_note->colour);
        while (!has_non_kick && first_note > track.notes().cbegin()
               && std::prev(first_note)->position == last_note->position) {
            --first_note;
            has_non_kick = !is_kick_colour(first_note->colour);
        }
        if (has_non_kick) {
            auto first_point = std::prev(earliest_after);
            while (first_point > points.begin()
                   && std::prev(first_point)->position.beat
                       >= std::prev(earliest_after)->position.beat) {
                --first_point;
            }
            first_point->is_activation_note = true;
        }
    }
}

static std::vector<Point> points_from_track(
    const NoteTrack<DrumNoteColour>& track, const TimeConverter& converter,
    const std::vector<int>& unison_phrases, double squeeze, Second video_lag,
    const Engine& engine, bool enable_double_kick, bool disable_kick)
{
    const auto& notes = track.notes();
    std::vector<Point> points;

    const auto has_relevant_bre = track.bre().has_value() && engine.has_bres();
    auto current_phrase = track.sp_phrases().cbegin();
    for (auto p = notes.cbegin(); p != notes.cend();) {
        if (skip_kick(p->colour, enable_double_kick, disable_kick)) {
            ++p;
            continue;
        }
        if (has_relevant_bre && p->position >= track.bre()->start) {
            break;
        }
        const auto q = std::find_if_not(
            std::next(p), notes.cend(), [&](auto note) {
                return skip_kick(note.colour, enable_double_kick, disable_kick);
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
        append_note_points(p, q, notes.cend(), std::back_inserter(points),
                           track.resolution(), is_note_sp_ender,
                           is_unison_sp_ender, converter, squeeze, engine,
                           enable_double_kick, disable_kick);
        p = q;
    }

    std::stable_sort(points.begin(), points.end(),
                     [](const auto& x, const auto& y) {
                         return x.position.beat < y.position.beat;
                     });

    add_drum_activation_points(track, points);

    auto combo = 0;
    for (auto& point : points) {
        if (!point.is_hold_point) {
            ++combo;
        }
        const auto multiplier
            = std::min(combo / 10 + 1, engine.max_multiplier());
        point.value *= multiplier;
    }

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

    return points;
}

template <typename T>
static std::vector<Point>
points_from_track(const NoteTrack<T>& track, const TimeConverter& converter,
                  const std::vector<int>& unison_phrases, double squeeze,
                  Second video_lag, const Engine& engine)
{
    static_assert(!std::is_same_v<T, DrumNoteColour>);

    const auto& notes = track.notes();
    std::vector<Point> points;

    const auto has_relevant_bre = track.bre().has_value() && engine.has_bres();
    auto current_phrase = track.sp_phrases().cbegin();
    for (auto p = notes.cbegin(); p != notes.cend();) {
        if (has_relevant_bre && p->position >= track.bre()->start) {
            break;
        }
        const auto q = std::find_if_not(p, notes.cend(), [=](const auto& x) {
            return x.position == p->position;
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
        append_note_points(p, q, notes.cend(), std::back_inserter(points),
                           track.resolution(), is_note_sp_ender,
                           is_unison_sp_ender, converter, squeeze, engine,
                           false, false);
        p = q;
    }

    std::stable_sort(points.begin(), points.end(),
                     [](const auto& x, const auto& y) {
                         return x.position.beat < y.position.beat;
                     });

    auto combo = 0;
    for (auto& point : points) {
        if (!point.is_hold_point) {
            ++combo;
        }
        const auto multiplier
            = std::min(combo / 10 + 1, engine.max_multiplier());
        point.value *= multiplier;
    }

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

    return points;
}

template <typename P>
static std::vector<PointPtr>
next_matching_vector(const std::vector<Point>& points, P predicate)
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

static std::vector<PointPtr>
next_non_hold_vector(const std::vector<Point>& points)
{
    return next_matching_vector(points,
                                [](const auto& p) { return !p.is_hold_point; });
}

static std::vector<PointPtr>
next_sp_note_vector(const std::vector<Point>& points)
{
    return next_matching_vector(
        points, [](const auto& p) { return p.is_sp_granting_note; });
}

static std::vector<std::tuple<Position, int>>
solo_boosts_from_solos(const std::vector<Solo>& solos, int resolution,
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

static std::vector<int> score_totals(const std::vector<Point>& points)
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

static std::string to_colour_string(const std::vector<NoteColour>& colours)
{
    const std::vector<std::tuple<NoteColour, std::string>> COLOUR_NAMES {
        {NoteColour::Green, "G"},  {NoteColour::Red, "R"},
        {NoteColour::Yellow, "Y"}, {NoteColour::Blue, "B"},
        {NoteColour::Orange, "O"}, {NoteColour::Open, "open"}};

    std::string colour_string;
    for (const auto& [colour, string] : COLOUR_NAMES) {
        if (std::find(colours.cbegin(), colours.cend(), colour)
            != colours.cend()) {
            colour_string += string;
        }
    }

    return colour_string;
}

static std::string to_colour_string(const std::vector<GHLNoteColour>& colours)
{
    const std::vector<std::tuple<GHLNoteColour, std::string>> COLOUR_NAMES {
        {GHLNoteColour::WhiteLow, "W1"},  {GHLNoteColour::WhiteMid, "W2"},
        {GHLNoteColour::WhiteHigh, "W3"}, {GHLNoteColour::BlackLow, "B1"},
        {GHLNoteColour::BlackMid, "B2"},  {GHLNoteColour::BlackHigh, "B3"},
        {GHLNoteColour::Open, "open"}};

    std::string colour_string;
    for (const auto& [colour, string] : COLOUR_NAMES) {
        if (std::find(colours.cbegin(), colours.cend(), colour)
            != colours.cend()) {
            colour_string += string;
        }
    }

    return colour_string;
}

static std::string to_colour_string(DrumNoteColour colour)
{
    const std::array<std::tuple<DrumNoteColour, std::string>, 9> COLOUR_NAMES {
        {{DrumNoteColour::Red, "R"},
         {DrumNoteColour::Yellow, "Y"},
         {DrumNoteColour::Blue, "B"},
         {DrumNoteColour::Green, "G"},
         {DrumNoteColour::YellowCymbal, "Y cymbal"},
         {DrumNoteColour::BlueCymbal, "B cymbal"},
         {DrumNoteColour::GreenCymbal, "G cymbal"},
         {DrumNoteColour::Kick, "kick"},
         {DrumNoteColour::DoubleKick, "kick"}}};

    for (const auto& [colour_key, string] : COLOUR_NAMES) {
        if (colour_key == colour) {
            return string;
        }
    }

    throw std::invalid_argument("Invalid drum colour");
}

template <typename T>
static std::vector<std::string> note_colours(const std::vector<Note<T>>& notes,
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
        if constexpr (std::is_same_v<T, DrumNoteColour>) {
            colours.push_back(to_colour_string(note_ptr->colour));
            ++note_ptr;
        } else {
            std::vector<T> current_colours;
            const auto position = note_ptr->position;
            while ((note_ptr != notes.cend())
                   && (note_ptr->position == position)) {
                current_colours.push_back(note_ptr->colour);
                ++note_ptr;
            }
            colours.push_back(to_colour_string(current_colours));
        }
    }
    return colours;
}

PointSet::PointSet(const NoteTrack<NoteColour>& track,
                   const TimeConverter& converter,
                   const std::vector<int>& unison_phrases, double squeeze,
                   Second video_lag, const Engine& engine,
                   bool enable_double_kick, bool disable_kick)
    : m_points {points_from_track(track, converter, unison_phrases, squeeze,
                                  video_lag, engine)}
    , m_next_non_hold_point {next_non_hold_vector(m_points)}
    , m_next_sp_granting_note {next_sp_note_vector(m_points)}
    , m_solo_boosts {solo_boosts_from_solos(track.solos(), track.resolution(),
                                            converter)}
    , m_cumulative_score_totals {score_totals(m_points)}
    , m_video_lag {video_lag}
    , m_colours {note_colours(track.notes(), m_points)}
{
    (void)enable_double_kick;
    (void)disable_kick;
}

PointSet::PointSet(const NoteTrack<GHLNoteColour>& track,
                   const TimeConverter& converter,
                   const std::vector<int>& unison_phrases, double squeeze,
                   Second video_lag, const Engine& engine,
                   bool enable_double_kick, bool disable_kick)
    : m_points {points_from_track(track, converter, unison_phrases, squeeze,
                                  video_lag, engine)}
    , m_next_non_hold_point {next_non_hold_vector(m_points)}
    , m_next_sp_granting_note {next_sp_note_vector(m_points)}
    , m_solo_boosts {solo_boosts_from_solos(track.solos(), track.resolution(),
                                            converter)}
    , m_cumulative_score_totals {score_totals(m_points)}
    , m_video_lag {video_lag}
    , m_colours {note_colours(track.notes(), m_points)}
{
    (void)enable_double_kick;
    (void)disable_kick;
}

PointSet::PointSet(const NoteTrack<DrumNoteColour>& track,
                   const TimeConverter& converter,
                   const std::vector<int>& unison_phrases, double squeeze,
                   Second video_lag, const Engine& engine,
                   bool enable_double_kick, bool disable_kick)
    : m_points {points_from_track(track, converter, unison_phrases, squeeze,
                                  video_lag, engine, enable_double_kick,
                                  disable_kick)}
    , m_next_non_hold_point {next_non_hold_vector(m_points)}
    , m_next_sp_granting_note {next_sp_note_vector(m_points)}
    , m_solo_boosts {solo_boosts_from_solos(track.solos(), track.resolution(),
                                            converter)}
    , m_cumulative_score_totals {score_totals(m_points)}
    , m_video_lag {video_lag}
    , m_colours {note_colours(track.notes(), m_points)}
{
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
