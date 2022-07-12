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

#include <iostream>

#include <iterator>
#include <stdexcept>

#include "imagebuilder.hpp"
#include "optimiser.hpp"

constexpr int MAX_BEATS_PER_LINE = 16;

static double get_beat_rate(const SyncTrack& sync_track, int resolution,
                            double beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return BASE_BEAT_RATE;
    }
    --ts;
    return BASE_BEAT_RATE * ts->numerator / ts->denominator;
}

static int get_numer(const SyncTrack& sync_track, int resolution, double beat)
{
    constexpr int BASE_NUMERATOR = 4;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return BASE_NUMERATOR;
    }
    --ts;
    return ts->numerator;
}

static double get_denom(const SyncTrack& sync_track, int resolution,
                        double beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return 1.0;
    }
    --ts;
    return BASE_BEAT_RATE / ts->denominator;
}

static DrumNoteColour cymbal_to_tom(DrumNoteColour colour)
{
    if (colour == DrumNoteColour::YellowCymbal) {
        return DrumNoteColour::Yellow;
    }
    if (colour == DrumNoteColour::BlueCymbal) {
        return DrumNoteColour::Blue;
    }
    if (colour == DrumNoteColour::GreenCymbal) {
        return DrumNoteColour::Green;
    }
    return colour;
}

static DrumNoteColour disco_flip(DrumNoteColour colour, int position,
                                 const std::vector<DiscoFlip>& disco_flips)
{
    for (const auto& flip : disco_flips) {
        if (flip.position > position
            || (flip.position + flip.length) < position) {
            continue;
        }
        if (colour == DrumNoteColour::YellowCymbal) {
            return DrumNoteColour::Red;
        }
        if (colour == DrumNoteColour::Red) {
            return DrumNoteColour::YellowCymbal;
        }
        return colour;
    }
    return colour;
}

static std::vector<DrawnNote<DrumNoteColour>>
drawn_notes(const NoteTrack<DrumNoteColour>& track,
            const DrumSettings& drum_settings)
{
    std::vector<DrawnNote<DrumNoteColour>> notes;

    for (const auto& note : track.notes()) {
        if (note.colour == DrumNoteColour::DoubleKick
            && !drum_settings.enable_double_kick) {
            continue;
        }
        if (note.colour == DrumNoteColour::Kick && drum_settings.disable_kick) {
            continue;
        }
        const auto note_colour = drum_settings.pro_drums
            ? disco_flip(note.colour, note.position, track.disco_flips())
            : cymbal_to_tom(note.colour);
        const auto beat
            = note.position / static_cast<double>(track.resolution());
        const auto length
            = note.length / static_cast<double>(track.resolution());
        auto is_sp_note = false;
        for (const auto& phrase : track.sp_phrases()) {
            if (note.position >= phrase.position
                && note.position < phrase.position + phrase.length) {
                is_sp_note = true;
            }
        }
        notes.push_back({beat, length, note_colour, is_sp_note});
    }

    return notes;
}

template <typename T>
static std::vector<DrawnNote<T>> drawn_notes(const NoteTrack<T>& track)
{
    std::vector<DrawnNote<T>> notes;

    for (const auto& note : track.notes()) {
        auto beat = note.position / static_cast<double>(track.resolution());
        auto length = note.length / static_cast<double>(track.resolution());
        auto is_sp_note = false;
        for (const auto& phrase : track.sp_phrases()) {
            if (note.position >= phrase.position
                && note.position < phrase.position + phrase.length) {
                is_sp_note = true;
            }
        }
        notes.push_back({beat, length, note.colour, is_sp_note});
    }

    return notes;
}

template <typename T>
static std::vector<DrawnRow> drawn_rows(const NoteTrack<T>& track,
                                        const SyncTrack& sync_track)
{
    int max_pos = 0;
    for (const auto& note : track.notes()) {
        auto length = note.position + note.length;
        if (length > max_pos) {
            max_pos = length;
        }
    }

    const auto max_beat = max_pos / static_cast<double>(track.resolution());
    auto current_beat = 0.0;
    std::vector<DrawnRow> rows;

    while (current_beat <= max_beat) {
        auto row_length = 0.0;
        while (true) {
            auto contribution = get_beat_rate(sync_track, track.resolution(),
                                              current_beat + row_length);
            if (contribution > MAX_BEATS_PER_LINE && row_length == 0.0) {
                // Break up a measure that spans more than a full row.
                while (contribution > MAX_BEATS_PER_LINE) {
                    rows.push_back(
                        {current_beat, current_beat + MAX_BEATS_PER_LINE});
                    current_beat += MAX_BEATS_PER_LINE;
                    contribution -= MAX_BEATS_PER_LINE;
                }
            }
            if (contribution + row_length > MAX_BEATS_PER_LINE) {
                break;
            }
            row_length += contribution;
            if (current_beat + row_length > max_beat) {
                break;
            }
        }
        rows.push_back({current_beat, current_beat + row_length});
        current_beat += row_length;
    }

    return rows;
}

ImageBuilder::ImageBuilder(const NoteTrack<NoteColour>& track,
                           const SyncTrack& sync_track, bool is_lefty_flip)
    : m_track_type {TrackType::FiveFret}
    , m_is_lefty_flip {is_lefty_flip}
    , m_rows {drawn_rows(track, sync_track)}
    , m_notes {drawn_notes(track)}

{
    constexpr double HALF_BEAT = 0.5;

    for (const auto& row : m_rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length
                = get_beat_rate(sync_track, track.resolution(), start);
            auto numer = get_numer(sync_track, track.resolution(), start);
            auto denom = get_denom(sync_track, track.resolution(), start);
            m_measure_lines.push_back(start);
            m_half_beat_lines.push_back(start + HALF_BEAT * denom);
            for (int i = 1; i < numer; ++i) {
                m_beat_lines.push_back(start + i * denom);
                m_half_beat_lines.push_back(start + (i + HALF_BEAT) * denom);
            }
            start += meas_length;
        }
    }
    m_measure_lines.push_back(m_rows.back().end);
}

ImageBuilder::ImageBuilder(const NoteTrack<GHLNoteColour>& track,
                           const SyncTrack& sync_track, bool is_lefty_flip)
    : m_track_type {TrackType::SixFret}
    , m_is_lefty_flip {is_lefty_flip}
    , m_rows {drawn_rows(track, sync_track)}
    , m_ghl_notes {drawn_notes(track)}
{
    constexpr double HALF_BEAT = 0.5;

    for (const auto& row : m_rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length
                = get_beat_rate(sync_track, track.resolution(), start);
            auto numer = get_numer(sync_track, track.resolution(), start);
            auto denom = get_denom(sync_track, track.resolution(), start);
            m_measure_lines.push_back(start);
            m_half_beat_lines.push_back(start + HALF_BEAT * denom);
            for (int i = 1; i < numer; ++i) {
                m_beat_lines.push_back(start + i * denom);
                m_half_beat_lines.push_back(start + (i + HALF_BEAT) * denom);
            }
            start += meas_length;
        }
    }
    m_measure_lines.push_back(m_rows.back().end);
}

ImageBuilder::ImageBuilder(const NoteTrack<DrumNoteColour>& track,
                           const SyncTrack& sync_track,
                           const DrumSettings& drum_settings,
                           bool is_lefty_flip)
    : m_track_type {TrackType::Drums}
    , m_is_lefty_flip {is_lefty_flip}
    , m_rows {drawn_rows(track, sync_track)}
    , m_drum_notes {drawn_notes(track, drum_settings)}
{
    constexpr double HALF_BEAT = 0.5;

    for (const auto& row : m_rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length
                = get_beat_rate(sync_track, track.resolution(), start);
            auto numer = get_numer(sync_track, track.resolution(), start);
            auto denom = get_denom(sync_track, track.resolution(), start);
            m_measure_lines.push_back(start);
            m_half_beat_lines.push_back(start + HALF_BEAT * denom);
            for (int i = 1; i < numer; ++i) {
                m_beat_lines.push_back(start + i * denom);
                m_half_beat_lines.push_back(start + (i + HALF_BEAT) * denom);
            }
            start += meas_length;
        }
    }
    m_measure_lines.push_back(m_rows.back().end);
}

void ImageBuilder::add_bpms(const SyncTrack& sync_track, int resolution)
{
    constexpr double MS_PER_SECOND = 1000.0;

    m_bpms.clear();

    for (const auto& bpm : sync_track.bpms()) {
        auto pos = bpm.position / static_cast<double>(resolution);
        auto tempo = static_cast<double>(bpm.bpm) / MS_PER_SECOND;
        if (pos < m_rows.back().end) {
            m_bpms.emplace_back(pos, tempo);
        }
    }
}

void ImageBuilder::add_bre(const BigRockEnding& bre, int resolution,
                           const TimeConverter& converter)
{
    const auto seconds_start = converter.beats_to_seconds(
        Beat {bre.start / static_cast<double>(resolution)});
    const auto seconds_end = converter.beats_to_seconds(
        Beat {bre.end / static_cast<double>(resolution)});
    const auto seconds_gap = seconds_end - seconds_start;
    const auto bre_value = static_cast<int>(750 + 500 * seconds_gap.value());

    m_total_score += bre_value;
    m_score_values.back() += bre_value;

    m_bre_ranges.emplace_back(bre.start / static_cast<double>(resolution),
                              bre.end / static_cast<double>(resolution));
}

static bool is_kick_colour(DrumNoteColour colour)
{
    return colour == DrumNoteColour::Kick
        || colour == DrumNoteColour::DoubleKick;
}

static bool is_fill_active(const std::vector<Note<DrumNoteColour>>& notes,
                           DrumFill fill)
{
    if (notes.empty()) {
        return false;
    }
    const auto fill_end = fill.position + fill.length;
    auto best_position = notes.front().position;
    auto has_non_kick = !is_kick_colour(notes.front().colour);
    for (auto i = 1U; i < notes.size(); ++i) {
        if (std::abs(fill_end - notes[i].position)
            > std::abs(fill_end - best_position)) {
            break;
        }
        if (notes[i].position == best_position) {
            has_non_kick |= !is_kick_colour(notes[i].colour);
        } else {
            best_position = notes[i].position;
            has_non_kick = !is_kick_colour(notes[i].colour);
        }
    }
    return has_non_kick;
}

void ImageBuilder::add_drum_fills(const NoteTrack<DrumNoteColour>& track)
{
    const auto resolution = static_cast<double>(track.resolution());
    for (auto fill : track.drum_fills()) {
        if (is_fill_active(track.notes(), fill)) {
            m_fill_ranges.emplace_back(fill.position / resolution,
                                       (fill.position + fill.length)
                                           / resolution);
        }
    }
}

void ImageBuilder::add_measure_values(const PointSet& points,
                                      const TimeConverter& converter,
                                      const Path& path)
{
    // This is needed below because the subtract_video_lag calculation can
    // introduce some floating point errors.
    constexpr double MEAS_EPSILON = 1 / 100000.0;

    m_base_values.clear();
    m_base_values.resize(m_measure_lines.size() - 1);

    m_score_values.clear();
    m_score_values.resize(m_measure_lines.size() - 1);

    const auto subtract_video_lag = [&](auto beat) {
        const auto seconds
            = converter.beats_to_seconds(beat) - points.video_lag();
        return (seconds.value() < 0.0) ? Beat(0.0)
                                       : converter.seconds_to_beats(seconds);
    };

    auto base_value_iter = m_base_values.begin();
    auto meas_iter = std::next(m_measure_lines.cbegin());
    auto score_value_iter = m_score_values.begin();
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        const auto adjusted_p_pos
            = subtract_video_lag(p->position.beat).value();
        while (meas_iter != m_measure_lines.cend()
               && (*meas_iter - MEAS_EPSILON) <= adjusted_p_pos) {
            ++meas_iter;
            ++base_value_iter;
            ++score_value_iter;
        }
        *base_value_iter += p->base_value;
        *score_value_iter += p->value;
    }

    meas_iter = std::next(m_measure_lines.cbegin());
    score_value_iter = m_score_values.begin();
    for (const auto& solo : points.solo_boosts()) {
        const auto& [solo_end, solo_score] = solo;
        while (meas_iter != m_measure_lines.cend()
               && *meas_iter <= solo_end.beat.value()) {
            ++meas_iter;
            ++score_value_iter;
        }
        // This is needed if a solo section ends after the last drawn measure of
        // the song.
        if (score_value_iter == m_score_values.end()) {
            --score_value_iter;
        }
        *score_value_iter += solo_score;
    }

    for (const auto& act : path.activations) {
        meas_iter = std::next(m_measure_lines.cbegin());
        score_value_iter = m_score_values.begin();
        for (auto p = act.act_start; p <= act.act_end; ++p) {
            while (meas_iter != m_measure_lines.cend()
                   && *meas_iter
                       <= subtract_video_lag(p->position.beat).value()) {
                ++meas_iter;
                ++score_value_iter;
            }
            *score_value_iter += p->value;
        }
    }

    int score_total = 0;
    for (auto& score : m_score_values) {
        score_total += score;
        score = score_total;
    }
}

void ImageBuilder::add_solo_sections(const std::vector<Solo>& solos,
                                     int resolution)
{
    for (const auto& solo : solos) {
        auto start = solo.start / static_cast<double>(resolution);
        auto end = solo.end / static_cast<double>(resolution);
        m_solo_ranges.emplace_back(start, end);
    }
}

void ImageBuilder::add_song_header(std::string song_name, std::string artist,
                                   std::string charter, int speed)
{
    constexpr int DEFAULT_SPEED = 100;
    m_song_name = std::move(song_name);
    if (speed != DEFAULT_SPEED) {
        const auto speedup_string = " (" + std::to_string(speed) + "%)";
        m_song_name += speedup_string;
    }
    m_artist = std::move(artist);
    m_charter = std::move(charter);
}

void ImageBuilder::add_sp_acts(const PointSet& points,
                               const TimeConverter& converter, const Path& path)
{
    std::vector<std::tuple<double, double>> no_whammy_ranges;

    const auto subtract_video_lag = [&](auto beat) {
        const auto seconds
            = converter.beats_to_seconds(beat) - points.video_lag();
        return (seconds.value() < 0.0) ? Beat(0.0)
                                       : converter.seconds_to_beats(seconds);
    };

    for (const auto& act : path.activations) {
        auto blue_start = act.sp_start;
        auto blue_end = act.sp_end;
        if (act.act_start != points.cbegin()) {
            blue_start
                = std::max(blue_start, std::prev(act.act_start)->position.beat);
        }
        if (std::next(act.act_end) != points.cend()) {
            blue_end
                = std::min(blue_end, std::next(act.act_end)->position.beat);
        }
        blue_end
            = std::min(subtract_video_lag(blue_end), Beat {m_rows.back().end});
        m_blue_ranges.emplace_back(subtract_video_lag(blue_start).value(),
                                   blue_end.value());
        if (act.sp_start > act.act_start->position.beat) {
            m_red_ranges.emplace_back(
                subtract_video_lag(act.act_start->position.beat).value(),
                subtract_video_lag(act.sp_start).value());
        }
        if (act.sp_end < act.act_end->position.beat) {
            m_red_ranges.emplace_back(
                subtract_video_lag(act.sp_end).value(),
                subtract_video_lag(act.act_end->position.beat).value());
        }
        no_whammy_ranges.emplace_back(act.whammy_end.value(),
                                      act.sp_end.value());
    }

    auto p = no_whammy_ranges.cbegin();
    auto q = m_green_ranges.cbegin();
    while (p < no_whammy_ranges.cend() && q < m_green_ranges.cend()) {
        auto start = std::max(std::get<0>(*p), std::get<0>(*q));
        auto end = std::min(std::get<1>(*p), std::get<1>(*q));
        if (start < end) {
            m_yellow_ranges.emplace_back(start, end);
        }
        if (std::get<1>(*q) > std::get<1>(*p)) {
            ++p;
        } else {
            ++q;
        }
    }
}

void ImageBuilder::add_sp_phrases(const NoteTrack<NoteColour>& track,
                                  const std::vector<int>& unison_phrases)
{
    constexpr double MINIMUM_GREEN_RANGE_SIZE = 0.1;

    for (const auto& phrase : track.sp_phrases()) {
        auto p = track.notes().cbegin();
        while (p->position < phrase.position) {
            ++p;
        }
        auto start = p->position / static_cast<double>(track.resolution());
        auto phrase_end = phrase.position + phrase.length;
        auto end = start;
        while (p < track.notes().cend() && p->position < phrase_end) {
            auto current_end = (p->position + p->length)
                / static_cast<double>(track.resolution());
            end = std::max(end, current_end);
            ++p;
        }
        end = std::max(end, start + MINIMUM_GREEN_RANGE_SIZE);
        m_green_ranges.emplace_back(start, end);
        if (std::find(unison_phrases.cbegin(), unison_phrases.cend(),
                      phrase.position)
            != unison_phrases.cend()) {
            m_unison_ranges.emplace_back(start, end);
        }
    }
}

void ImageBuilder::add_sp_phrases(const NoteTrack<GHLNoteColour>& track,
                                  const std::vector<int>& unison_phrases)
{
    constexpr double MINIMUM_GREEN_RANGE_SIZE = 0.1;

    for (const auto& phrase : track.sp_phrases()) {
        auto p = track.notes().cbegin();
        while (p->position < phrase.position) {
            ++p;
        }
        auto start = p->position / static_cast<double>(track.resolution());
        auto phrase_end = phrase.position + phrase.length;
        auto end = start;
        while (p < track.notes().cend() && p->position < phrase_end) {
            auto current_end = (p->position + p->length)
                / static_cast<double>(track.resolution());
            end = std::max(end, current_end);
            ++p;
        }
        end = std::max(end, start + MINIMUM_GREEN_RANGE_SIZE);
        m_green_ranges.emplace_back(start, end);
        if (std::find(unison_phrases.cbegin(), unison_phrases.cend(),
                      phrase.position)
            != unison_phrases.cend()) {
            m_unison_ranges.emplace_back(start, end);
        }
    }
}

void ImageBuilder::add_sp_phrases(const NoteTrack<DrumNoteColour>& track,
                                  const std::vector<int>& unison_phrases)
{
    constexpr double MINIMUM_GREEN_RANGE_SIZE = 0.1;

    for (const auto& phrase : track.sp_phrases()) {
        auto p = track.notes().cbegin();
        while (p->position < phrase.position) {
            ++p;
        }
        auto start = p->position / static_cast<double>(track.resolution());
        auto phrase_end = phrase.position + phrase.length;
        auto end = start;
        while (p < track.notes().cend() && p->position < phrase_end) {
            auto current_end = (p->position + p->length)
                / static_cast<double>(track.resolution());
            end = std::max(end, current_end);
            ++p;
        }
        end = std::max(end, start + MINIMUM_GREEN_RANGE_SIZE);
        m_green_ranges.emplace_back(start, end);
        if (std::find(unison_phrases.cbegin(), unison_phrases.cend(),
                      phrase.position)
            != unison_phrases.cend()) {
            m_unison_ranges.emplace_back(start, end);
        }
    }
}

enum class SpDrainEventType { Measure, SpPhrase, ActStart, ActEnd, WhammyEnd };

static std::tuple<std::vector<std::tuple<Beat, SpDrainEventType>>, bool, Beat>
form_events(Beat start, Beat end, const PointSet& points, const Path& path)
{
    std::vector<std::tuple<Beat, SpDrainEventType>> events;
    events.emplace_back(start, SpDrainEventType::Measure);
    events.emplace_back(end, SpDrainEventType::Measure);
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        if (p->is_sp_granting_note && p->position.beat >= start
            && p->position.beat < end) {
            events.emplace_back(p->position.beat, SpDrainEventType::SpPhrase);
        }
    }
    auto is_sp_active = false;
    Beat whammy_end {std::numeric_limits<double>::infinity()};
    for (auto act : path.activations) {
        if (act.sp_end < start) {
            continue;
        }
        whammy_end = act.whammy_end;
        if (act.sp_start >= end) {
            continue;
        }
        if (act.sp_start >= start) {
            events.emplace_back(act.sp_start, SpDrainEventType::ActStart);
        } else if (act.sp_end < end) {
            is_sp_active = true;
            events.emplace_back(act.sp_end, SpDrainEventType::ActEnd);
        } else {
            is_sp_active = true;
        }
    }
    std::stable_sort(events.begin(), events.end());
    return {events, is_sp_active, whammy_end};
}

void ImageBuilder::add_sp_percent_values(const SpData& sp_data,
                                         const TimeConverter& converter,
                                         const PointSet& points,
                                         const Path& path)
{
    constexpr double SP_PHRASE_AMOUNT = 0.25;

    m_sp_percent_values.clear();
    m_sp_percent_values.resize(m_measure_lines.size() - 1);

    double total_sp = 0.0;
    for (std::size_t i = 0; i < m_measure_lines.size() - 1; ++i) {
        Beat start {m_measure_lines[i]};
        Beat end {std::numeric_limits<double>::infinity()};
        if (i < m_measure_lines.size() - 1) {
            end = Beat {m_measure_lines[i + 1]};
        }
        auto [events, is_sp_active, whammy_end]
            = form_events(start, end, points, path);
        for (std::size_t j = 0; j < events.size() - 1; ++j) {
            const auto start_beat = std::get<0>(events[j]);
            const auto end_beat = std::get<0>(events[j + 1]);
            if (is_sp_active) {
                Position start_pos {start_beat,
                                    converter.beats_to_measures(start_beat)};
                Position end_pos {end_beat,
                                  converter.beats_to_measures(end_beat)};
                Position whammy_pos {whammy_end,
                                     converter.beats_to_measures(whammy_end)};
                total_sp = sp_data.propagate_sp_over_whammy_min(
                    start_pos, end_pos, total_sp, whammy_pos);
            } else if (whammy_end > start_beat) {
                total_sp += sp_data.available_whammy(
                    start_beat, std::min(end_beat, whammy_end));
            }
            switch (std::get<1>(events[j + 1])) {
            case SpDrainEventType::ActStart:
                is_sp_active = true;
                break;
            case SpDrainEventType::ActEnd:
                is_sp_active = false;
                break;
            case SpDrainEventType::SpPhrase:
                total_sp += SP_PHRASE_AMOUNT;
                break;
            case SpDrainEventType::Measure:
                break;
            }
            total_sp = std::clamp(total_sp, 0.0, 1.0);
        }

        m_sp_percent_values[i] = total_sp;
    }
}

void ImageBuilder::add_sp_values(const SpData& sp_data, const Engine& engine)
{
    m_sp_values.clear();
    m_sp_values.resize(m_measure_lines.size() - 1);

    for (std::size_t i = 0; i < m_measure_lines.size() - 1; ++i) {
        Beat start {m_measure_lines[i]};
        Beat end {std::numeric_limits<double>::infinity()};
        if (i < m_measure_lines.size() - 1) {
            end = Beat {m_measure_lines[i + 1]};
        }
        m_sp_values[i]
            = sp_data.available_whammy(start, end) / engine.sp_gain_rate();
    }
}

void ImageBuilder::add_time_sigs(const SyncTrack& sync_track, int resolution)
{
    for (const auto& ts : sync_track.time_sigs()) {
        auto pos = ts.position / static_cast<double>(resolution);
        auto num = ts.numerator;
        auto denom = ts.denominator;
        if (pos < m_rows.back().end) {
            m_time_sigs.emplace_back(pos, num, denom);
        }
    }
}

void ImageBuilder::set_total_score(const PointSet& points,
                                   const std::vector<Solo>& solos,
                                   const Path& path)
{
    auto no_sp_score = std::accumulate(
        points.cbegin(), points.cend(), 0,
        [](const auto x, const auto& y) { return x + y.value; });
    no_sp_score += std::accumulate(
        solos.cbegin(), solos.cend(), 0,
        [](const auto x, const auto& y) { return x + y.value; });
    m_total_score = no_sp_score + path.score_boost;
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

template <typename T>
static ImageBuilder build_with_drum_params(const NoteTrack<T>& track,
                                           const SyncTrack& sync_track,
                                           const Settings& settings)
{
    if constexpr (std::is_same_v<T, DrumNoteColour>) {
        return {track, sync_track, settings.drum_settings,
                settings.is_lefty_flip};
    } else {
        return {track, sync_track, settings.is_lefty_flip};
    }
}

template <typename T>
static ImageBuilder
make_builder_from_track(const Song& song, const NoteTrack<T>& track,
                        const Settings& settings,
                        const std::function<void(const char*)>& write,
                        const std::atomic<bool>* terminate)
{
    auto new_track = track;
    if (song.is_from_midi()) {
        new_track = track.trim_sustains();
    }
    new_track = new_track.snap_chords(settings.engine->snap_gap());
    if constexpr (std::is_same_v<T, DrumNoteColour>) {
        if (!settings.engine->is_rock_band()
            && new_track.drum_fills().empty()) {
            new_track.generate_drum_fills({song.sync_track(),
                                           new_track.resolution(),
                                           *settings.engine,
                                           {}});
        }
        if (!settings.drum_settings.enable_dynamics) {
            new_track.disable_dynamics();
        }
    }
    const auto sync_track = song.sync_track().speedup(settings.speed);

    auto builder = build_with_drum_params(new_track, sync_track, settings);
    builder.add_song_header(song.name(), song.artist(), song.charter(),
                            settings.speed);
    if (settings.engine->has_unison_bonuses()) {
        builder.add_sp_phrases(new_track, song.unison_phrase_positions());
    } else {
        builder.add_sp_phrases(new_track, {});
    }

    if constexpr (std::is_same_v<T, DrumNoteColour>) {
        builder.add_drum_fills(new_track);
    }

    if (settings.draw_bpms) {
        builder.add_bpms(sync_track, new_track.resolution());
    }

    const auto solos = new_track.solos(settings.drum_settings);
    if (settings.draw_solos) {
        builder.add_solo_sections(solos, new_track.resolution());
    }

    if (settings.draw_time_sigs) {
        builder.add_time_sigs(sync_track, new_track.resolution());
    }

    // The 0.1% squeeze minimum is to get around dumb floating point rounding
    // issues that visibly affect the path at 0% squeeze.
    auto squeeze_settings = settings.squeeze_settings;
    constexpr double SQUEEZE_EPSILON = 0.001;
    squeeze_settings.squeeze
        = std::max(squeeze_settings.squeeze, SQUEEZE_EPSILON);
    const ProcessedSong processed_track {new_track,
                                         sync_track,
                                         settings.squeeze_settings,
                                         settings.drum_settings,
                                         *settings.engine,
                                         song.od_beats(),
                                         song.unison_phrase_positions()};
    Path path;

    if (!settings.blank) {
        const auto is_rb_drums
            = std::is_same_v<T,
                             DrumNoteColour> && settings.engine->is_rock_band();
        if (is_rb_drums) {
            write("Optimisation disabled for Rock Band drums, planned for a "
                  "future release");
        } else {
            write("Optimising, please wait...");
            const Optimiser optimiser {&processed_track, terminate,
                                       settings.speed,
                                       squeeze_settings.whammy_delay};
            path = optimiser.optimal_path();
            write(processed_track.path_summary(path).c_str());
            builder.add_sp_acts(processed_track.points(),
                                processed_track.converter(), path);
            builder.activation_opacity() = settings.opacity;
        }
    }

    builder.add_measure_values(processed_track.points(),
                               processed_track.converter(), path);
    builder.add_sp_values(processed_track.sp_data(), *settings.engine);
    builder.set_total_score(processed_track.points(), solos, path);
    if (settings.engine->has_bres() && new_track.bre().has_value()) {
        builder.add_bre(*(new_track.bre()), new_track.resolution(),
                        processed_track.converter());
    }

    return builder;
}

ImageBuilder make_builder(const Song& song, const Settings& settings,
                          const std::function<void(const char*)>& write,
                          const std::atomic<bool>* terminate)
{
    if (settings.instrument == Instrument::GHLGuitar) {
        const auto& track = song.ghl_guitar_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings, write, terminate);
    }
    if (settings.instrument == Instrument::GHLBass) {
        const auto& track = song.ghl_bass_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings, write, terminate);
    }
    if (settings.instrument == Instrument::Drums) {
        const auto& track = song.drum_note_track(settings.difficulty);
        return make_builder_from_track(song, track, settings, write, terminate);
    }
    const auto& track = track_from_inst_diff(settings, song);
    return make_builder_from_track(song, track, settings, write, terminate);
}
