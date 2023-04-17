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

#include <iterator>
#include <stdexcept>

#include "imagebuilder.hpp"
#include "optimiser.hpp"

constexpr int MAX_BEATS_PER_LINE = 16;

namespace {
double get_beat_rate(const SyncTrack& sync_track, int resolution, double beat)
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

int get_numer(const SyncTrack& sync_track, int resolution, double beat)
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

double get_denom(const SyncTrack& sync_track, int resolution, double beat)
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

DrawnNote note_to_drawn_note(const Note& note, const NoteTrack& track)
{
    constexpr auto COLOURS_SIZE = 7;
    const auto beat = note.position / static_cast<double>(track.resolution());

    std::array<double, COLOURS_SIZE> lengths {};
    for (auto i = 0; i < COLOURS_SIZE; ++i) {
        if (note.lengths.at(i) == -1) {
            lengths.at(i) = -1;
        } else {
            lengths.at(i)
                = note.lengths.at(i) / static_cast<double>(track.resolution());
        }
    }

    auto is_sp_note = false;
    for (const auto& phrase : track.sp_phrases()) {
        if (note.position >= phrase.position
            && note.position < phrase.position + phrase.length) {
            is_sp_note = true;
            break;
        }
    }

    return {beat, lengths, note.flags, is_sp_note};
}

bool is_in_disco_flips(const std::vector<DiscoFlip>& disco_flips, int position)
{
    return std::any_of(disco_flips.cbegin(), disco_flips.cend(),
                       [&](const auto& flip) {
                           return (flip.position <= position)
                               && (flip.position + flip.length >= position);
                       });
}

std::vector<DrawnNote> drawn_notes(const NoteTrack& track,
                                   const DrumSettings& drum_settings)
{
    std::vector<DrawnNote> notes;

    for (const auto& note : track.notes()) {
        if (note.is_skipped_kick(drum_settings)) {
            continue;
        }
        auto drawn_note = note_to_drawn_note(note, track);
        if (!drum_settings.pro_drums) {
            drawn_note.note_flags
                = static_cast<NoteFlags>(drawn_note.note_flags & ~FLAGS_CYMBAL);
        } else if (is_in_disco_flips(track.disco_flips(), note.position)) {
            if (note.lengths[DRUM_RED] != -1) {
                std::swap(drawn_note.lengths[DRUM_RED],
                          drawn_note.lengths[DRUM_YELLOW]);
                drawn_note.note_flags = static_cast<NoteFlags>(
                    drawn_note.note_flags | FLAGS_CYMBAL);
            } else if (note.lengths[DRUM_YELLOW] != 1
                       && (note.flags & FLAGS_CYMBAL) != 0U) {
                std::swap(drawn_note.lengths[DRUM_RED],
                          drawn_note.lengths[DRUM_YELLOW]);
                drawn_note.note_flags = static_cast<NoteFlags>(
                    drawn_note.note_flags & ~FLAGS_CYMBAL);
            }
        }
        notes.push_back(drawn_note);
    }

    return notes;
}

std::vector<DrawnRow> drawn_rows(const NoteTrack& track,
                                 const SyncTrack& sync_track)
{
    int max_pos = 0;
    for (const auto& note : track.notes()) {
        const auto length
            = *std::max_element(note.lengths.cbegin(), note.lengths.cend());
        const auto note_end = note.position + length;
        max_pos = std::max(max_pos, note_end);
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

// It is important that SpPhrase has a higher value than ActEnd. This is because
// in form_events we want to push the event for getting a phrase to the end of
// the activation if the activation is not an overlap.
enum class SpDrainEventType { Measure, ActStart, ActEnd, WhammyEnd, SpPhrase };

std::vector<std::tuple<Beat, SpDrainEventType>>
form_events(const std::vector<double>& measure_lines, const PointSet& points,
            const Path& path)
{
    std::vector<std::tuple<Beat, SpDrainEventType>> events;
    for (std::size_t i = 1; i < measure_lines.size(); ++i) {
        events.emplace_back(Beat {measure_lines[i]}, SpDrainEventType::Measure);
    }
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        if (!p->is_sp_granting_note) {
            continue;
        }
        auto position = p->position.beat;
        for (auto act : path.activations) {
            if (p > act.act_end) {
                position = std::max(position, act.sp_end);
            }
        }
        events.emplace_back(position, SpDrainEventType::SpPhrase);
    }
    for (auto act : path.activations) {
        if (act.whammy_end < act.sp_end) {
            events.emplace_back(act.whammy_end, SpDrainEventType::WhammyEnd);
        }
        events.emplace_back(act.sp_start, SpDrainEventType::ActStart);
        events.emplace_back(act.sp_end, SpDrainEventType::ActEnd);
    }
    std::sort(events.begin(), events.end());
    return events;
}

ImageBuilder build_with_engine_params(const NoteTrack& track,
                                      const SyncTrack& sync_track,
                                      const Settings& settings)
{
    return {track,
            sync_track,
            settings.difficulty,
            settings.drum_settings,
            settings.is_lefty_flip,
            settings.engine->overlaps()};
}

std::vector<std::tuple<double, double>>
relative_complement(std::vector<std::tuple<double, double>> parent_set,
                    const std::vector<std::tuple<double, double>>& excluded_set)
{
    std::vector<std::tuple<double, double>> result;
    auto p = parent_set.begin();
    auto q = excluded_set.cbegin();
    while (p < parent_set.end() && q < excluded_set.cend()) {
        if (std::get<0>(*q) < std::get<0>(*p)) {
            std::get<0>(*p) = std::max(std::get<0>(*p), std::get<1>(*q));
            ++q;
            continue;
        }
        if (std::get<0>(*q) > std::get<0>(*p)) {
            result.emplace_back(std::get<0>(*p),
                                std::min(std::get<1>(*p), std::get<0>(*q)));
        }
        ++p;
    }
    std::copy(p, parent_set.end(), std::back_inserter(result));
    return result;
}

Beat subtract_video_lag(Beat beat, Second video_lag,
                        const TimeConverter& converter)
{
    const auto seconds = converter.beats_to_seconds(beat) - video_lag;
    if (seconds.value() < 0.0) {
        return Beat {0.0};
    }
    return converter.seconds_to_beats(seconds);
}
}

void ImageBuilder::form_beat_lines(const SyncTrack& sync_track, int resolution)
{
    constexpr double HALF_BEAT = 0.5;

    for (const auto& row : m_rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length = get_beat_rate(sync_track, resolution, start);
            auto numer = get_numer(sync_track, resolution, start);
            auto denom = get_denom(sync_track, resolution, start);
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

bool ImageBuilder::is_neutralised_phrase(Beat note_pos, const Path& path)
{
    for (const auto& act : path.activations) {
        if (act.act_start->position.beat > note_pos) {
            return false;
        }
        if (act.act_end->position.beat >= note_pos) {
            return true;
        }
    }
    return false;
}

std::tuple<double, double>
ImageBuilder::sp_phrase_bounds(const StarPower& phrase, const NoteTrack& track,
                               const Path& path) const
{
    constexpr double MINIMUM_GREEN_RANGE_SIZE = 0.1;

    auto p = track.notes().cbegin();
    while (p->position < phrase.position) {
        ++p;
    }
    const auto start = p->position / static_cast<double>(track.resolution());
    if (!m_overlap_engine && is_neutralised_phrase(Beat {start}, path)) {
        return {-1, -1};
    }
    const auto phrase_end = phrase.position + phrase.length;
    auto end = start;
    while (p < track.notes().cend() && p->position < phrase_end) {
        auto max_length = 0;
        for (auto length : p->lengths) {
            max_length = std::max(max_length, length);
        }
        auto current_end = (p->position + max_length)
            / static_cast<double>(track.resolution());
        end = std::max(end, current_end);
        ++p;
    }
    end = std::max(end, start + MINIMUM_GREEN_RANGE_SIZE);
    return {start, end};
}

ImageBuilder::ImageBuilder(const NoteTrack& track, const SyncTrack& sync_track,
                           Difficulty difficulty,
                           const DrumSettings& drum_settings,
                           bool is_lefty_flip, bool is_overlap_engine)
    : m_track_type {track.track_type()}
    , m_difficulty {difficulty}
    , m_is_lefty_flip {is_lefty_flip}
    , m_rows {drawn_rows(track, sync_track)}
    , m_notes {drawn_notes(track, drum_settings)}
    , m_overlap_engine {is_overlap_engine}
{
    form_beat_lines(sync_track, track.resolution());
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

void ImageBuilder::add_drum_fills(const NoteTrack& track)
{
    const auto resolution = static_cast<double>(track.resolution());
    for (auto fill : track.drum_fills()) {
        m_fill_ranges.emplace_back(fill.position / resolution,
                                   (fill.position + fill.length) / resolution);
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

    auto base_value_iter = m_base_values.begin();
    auto meas_iter = std::next(m_measure_lines.cbegin());
    auto score_value_iter = m_score_values.begin();
    for (auto p = points.cbegin(); p < points.cend(); ++p) {
        const auto adjusted_p_pos
            = subtract_video_lag(p->position.beat, points.video_lag(),
                                 converter)
                  .value();
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
                   && *meas_iter <= subtract_video_lag(p->position.beat,
                                                       points.video_lag(),
                                                       converter)
                                        .value()) {
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

void ImageBuilder::add_song_header(const SongGlobalData& global_data, int speed)
{
    constexpr int DEFAULT_SPEED = 100;
    m_song_name = global_data.name();
    if (speed != DEFAULT_SPEED) {
        const auto speedup_string = " (" + std::to_string(speed) + "%)";
        m_song_name += speedup_string;
    }
    m_artist = global_data.artist();
    m_charter = global_data.charter();
}

void ImageBuilder::add_sp_acts(const PointSet& points,
                               const TimeConverter& converter, const Path& path)
{
    std::vector<std::tuple<double, double>> no_whammy_ranges;

    const auto shifted_beat = [&](auto beat) {
        return subtract_video_lag(beat, points.video_lag(), converter);
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
        blue_end = std::min(shifted_beat(blue_end), Beat {m_rows.back().end});
        m_blue_ranges.emplace_back(shifted_beat(blue_start).value(),
                                   blue_end.value());
        if (act.sp_start > act.act_start->position.beat) {
            m_red_ranges.emplace_back(
                shifted_beat(act.act_start->position.beat).value(),
                shifted_beat(act.sp_start).value());
        }
        if (act.sp_end < act.act_end->position.beat) {
            m_red_ranges.emplace_back(
                shifted_beat(act.sp_end).value(),
                shifted_beat(act.act_end->position.beat).value());
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

    if (!m_overlap_engine) {
        m_yellow_ranges = relative_complement(m_yellow_ranges, m_blue_ranges);
        m_green_ranges = relative_complement(m_green_ranges, m_blue_ranges);
    }
}

void ImageBuilder::add_sp_percent_values(const SpData& sp_data,
                                         const TimeConverter& converter,
                                         const PointSet& points,
                                         const Path& path)
{
    constexpr double SP_PHRASE_AMOUNT = 0.25;

    m_sp_percent_values.clear();
    m_sp_percent_values.reserve(m_measure_lines.size() - 1);

    const auto events = form_events(m_measure_lines, points, path);
    auto is_sp_active = false;
    Beat whammy_end {std::numeric_limits<double>::infinity()};
    Beat position {0.0};

    double total_sp = 0.0;
    for (auto [event_pos, event_type] : events) {
        if (is_sp_active) {
            Position start_pos {position,
                                converter.beats_to_measures(position)};
            Position end_pos {event_pos,
                              converter.beats_to_measures(event_pos)};
            Position whammy_pos {Beat {0.0}, Measure {0.0}};
            if (m_overlap_engine) {
                whammy_pos
                    = {whammy_end, converter.beats_to_measures(whammy_end)};
            }
            total_sp = sp_data.propagate_sp_over_whammy_min(
                start_pos, end_pos, total_sp, whammy_pos);
        } else if (whammy_end > position) {
            total_sp += sp_data.available_whammy(
                position, std::min(event_pos, whammy_end));
        }
        switch (event_type) {
        case SpDrainEventType::ActStart:
            is_sp_active = true;
            break;
        case SpDrainEventType::ActEnd:
            is_sp_active = false;
            whammy_end = Beat {std::numeric_limits<double>::infinity()};
            total_sp = 0.0;
            break;
        case SpDrainEventType::SpPhrase:
            total_sp += SP_PHRASE_AMOUNT;
            break;
        case SpDrainEventType::Measure:
            m_sp_percent_values.push_back(std::clamp(total_sp, 0.0, 1.0));
            break;
        case SpDrainEventType::WhammyEnd:
            whammy_end = event_pos;
            break;
        }
        position = event_pos;
        total_sp = std::clamp(total_sp, 0.0, 1.0);
    }

    assert(m_sp_percent_values.size() == m_measure_lines.size() - 1); // NOLINT
}

void ImageBuilder::add_sp_phrases(const NoteTrack& track,
                                  const std::vector<int>& unison_phrases,
                                  const Path& path)
{
    for (const auto& phrase : track.sp_phrases()) {
        const auto range = sp_phrase_bounds(phrase, track, path);
        if (std::get<0>(range) == -1) {
            continue;
        }
        m_green_ranges.push_back(range);
        if (std::find(unison_phrases.cbegin(), unison_phrases.cend(),
                      phrase.position)
            != unison_phrases.cend()) {
            m_unison_ranges.push_back(range);
        }
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

ImageBuilder make_builder(const Song& song, const NoteTrack& track,
                          const Settings& settings,
                          const std::function<void(const char*)>& write,
                          const std::atomic<bool>* terminate)
{
    auto new_track = track;
    if (song.global_data().is_from_midi()) {
        new_track = track.trim_sustains();
    }
    new_track = new_track.snap_chords(settings.engine->snap_gap());
    if (track.track_type() == TrackType::Drums) {
        if (!settings.engine->is_rock_band()
            && new_track.drum_fills().empty()) {
            new_track.generate_drum_fills({song.global_data().sync_track(),
                                           new_track.resolution(),
                                           *settings.engine,
                                           {}});
        }
        if (!settings.drum_settings.enable_dynamics) {
            new_track.disable_dynamics();
        }
    }
    const auto sync_track
        = song.global_data().sync_track().speedup(settings.speed);

    auto builder = build_with_engine_params(new_track, sync_track, settings);
    builder.add_song_header(song.global_data(), settings.speed);

    if (track.track_type() == TrackType::Drums) {
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

    const auto unison_positions = (settings.engine->has_unison_bonuses())
        ? song.unison_phrase_positions()
        : std::vector<int> {};

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
                                         song.global_data().od_beats(),
                                         unison_positions};
    Path path;

    if (!settings.blank) {
        const auto is_rb_drums = track.track_type() == TrackType::Drums
            && settings.engine->is_rock_band();
        if (is_rb_drums) {
            write("Optimisation disabled for Rock Band drums, planned for a "
                  "future release");
            builder.add_sp_phrases(new_track, unison_positions, path);
        } else {
            write("Optimising, please wait...");
            const Optimiser optimiser {&processed_track, terminate,
                                       settings.speed,
                                       squeeze_settings.whammy_delay};
            path = optimiser.optimal_path();
            write(processed_track.path_summary(path).c_str());
            builder.add_sp_phrases(new_track, unison_positions, path);
            builder.add_sp_acts(processed_track.points(),
                                processed_track.converter(), path);
            builder.activation_opacity() = settings.opacity;
        }
    } else {
        builder.add_sp_phrases(new_track, unison_positions, path);
    }

    builder.add_measure_values(processed_track.points(),
                               processed_track.converter(), path);
    if (settings.blank || !settings.engine->overlaps()) {
        builder.add_sp_values(processed_track.sp_data(), *settings.engine);
    } else {
        builder.add_sp_percent_values(processed_track.sp_data(),
                                      processed_track.converter(),
                                      processed_track.points(), path);
    }
    builder.set_total_score(processed_track.points(), solos, path);
    if (settings.engine->has_bres() && new_track.bre().has_value()) {
        builder.add_bre(*(new_track.bre()), new_track.resolution(),
                        processed_track.converter());
    }

    return builder;
}
