/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023, 2024, 2025 Raymond Wright
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
#include <iterator>
#include <stdexcept>

#include "imagebuilder.hpp"
#include "optimiser.hpp"

constexpr int MAX_BEATS_PER_LINE = 16;

namespace {
double get_beat_rate(const SightRead::TempoMap& tempo_map, SightRead::Beat beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        tempo_map.time_sigs().cbegin(), tempo_map.time_sigs().cend(),
        [=](const auto& x) { return tempo_map.to_beats(x.position) > beat; });
    if (ts == tempo_map.time_sigs().cbegin()) {
        return BASE_BEAT_RATE;
    }
    --ts;
    return BASE_BEAT_RATE * ts->numerator / ts->denominator;
}

int get_numer(const SightRead::TempoMap& tempo_map, SightRead::Beat beat)
{
    constexpr int BASE_NUMERATOR = 4;

    auto ts = std::find_if(
        tempo_map.time_sigs().cbegin(), tempo_map.time_sigs().cend(),
        [=](const auto& x) { return tempo_map.to_beats(x.position) > beat; });
    if (ts == tempo_map.time_sigs().cbegin()) {
        return BASE_NUMERATOR;
    }
    --ts;
    return ts->numerator;
}

double get_denom(const SightRead::TempoMap& tempo_map, SightRead::Beat beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        tempo_map.time_sigs().cbegin(), tempo_map.time_sigs().cend(),
        [=](const auto& x) { return tempo_map.to_beats(x.position) > beat; });
    if (ts == tempo_map.time_sigs().cbegin()) {
        return 1.0;
    }
    --ts;
    return BASE_BEAT_RATE / ts->denominator;
}

DrawnNote note_to_drawn_note(const SightRead::Note& note,
                             const SightRead::NoteTrack& track)
{
    constexpr auto COLOURS_SIZE = 7;
    const auto& tempo_map = track.global_data().tempo_map();
    const auto beat = tempo_map.to_beats(note.position);

    std::array<double, COLOURS_SIZE> lengths {};
    for (auto i = 0; i < COLOURS_SIZE; ++i) {
        if (note.lengths.at(i) == SightRead::Tick {-1}) {
            lengths.at(i) = -1;
        } else {
            lengths.at(i) = tempo_map.to_beats(note.lengths.at(i)).value();
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

    return {beat.value(), lengths, note.flags, is_sp_note};
}

bool is_in_disco_flips(const std::vector<SightRead::DiscoFlip>& disco_flips,
                       SightRead::Tick position)
{
    return std::any_of(disco_flips.cbegin(), disco_flips.cend(),
                       [&](const auto& flip) {
                           return (flip.position <= position)
                               && (flip.position + flip.length >= position);
                       });
}

std::vector<DrawnNote> drawn_notes(const SightRead::NoteTrack& track,
                                   const SightRead::DrumSettings& drum_settings)
{
    std::vector<DrawnNote> notes;

    for (const auto& note : track.notes()) {
        if (note.is_skipped_kick(drum_settings)) {
            continue;
        }
        auto drawn_note = note_to_drawn_note(note, track);
        if (!drum_settings.pro_drums) {
            drawn_note.note_flags = static_cast<SightRead::NoteFlags>(
                drawn_note.note_flags & ~SightRead::FLAGS_CYMBAL);
        } else if (is_in_disco_flips(track.disco_flips(), note.position)) {
            if (note.lengths[SightRead::DRUM_RED] != SightRead::Tick {-1}) {
                std::swap(drawn_note.lengths[SightRead::DRUM_RED],
                          drawn_note.lengths[SightRead::DRUM_YELLOW]);
                drawn_note.note_flags = static_cast<SightRead::NoteFlags>(
                    drawn_note.note_flags | SightRead::FLAGS_CYMBAL);
            } else if (note.lengths[SightRead::DRUM_YELLOW]
                           != SightRead::Tick {-1}
                       && (note.flags & SightRead::FLAGS_CYMBAL) != 0U) {
                std::swap(drawn_note.lengths[SightRead::DRUM_RED],
                          drawn_note.lengths[SightRead::DRUM_YELLOW]);
                drawn_note.note_flags = static_cast<SightRead::NoteFlags>(
                    drawn_note.note_flags & ~SightRead::FLAGS_CYMBAL);
            }
        }
        notes.push_back(drawn_note);
    }

    return notes;
}

std::vector<DrawnRow> drawn_rows(const SightRead::NoteTrack& track)
{
    SightRead::Tick max_pos {0};
    for (const auto& note : track.notes()) {
        const auto length
            = *std::max_element(note.lengths.cbegin(), note.lengths.cend());
        const auto note_end = note.position + length;
        max_pos = std::max(max_pos, note_end);
    }

    const auto& tempo_map = track.global_data().tempo_map();
    const auto max_beat = tempo_map.to_beats(max_pos).value();
    auto current_beat = 0.0;
    std::vector<DrawnRow> rows;

    while (current_beat <= max_beat) {
        auto row_length = 0.0;
        while (true) {
            auto contribution = get_beat_rate(
                tempo_map, SightRead::Beat {current_beat + row_length});
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
enum class SpDrainEventType : std::uint8_t {
    Measure,
    ActStart,
    ActEnd,
    WhammyEnd,
    SpPhrase
};

std::vector<std::tuple<SightRead::Beat, SpDrainEventType>>
form_events(const std::vector<double>& measure_lines, const PointSet& points,
            const Path& path)
{
    std::vector<std::tuple<SightRead::Beat, SpDrainEventType>> events;
    for (std::size_t i = 1; i < measure_lines.size(); ++i) {
        events.emplace_back(SightRead::Beat {measure_lines[i]},
                            SpDrainEventType::Measure);
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

ImageBuilder build_with_engine_params(const SightRead::NoteTrack& track,
                                      const Settings& settings)
{
    return {track, settings.difficulty, settings.pathing_settings.drum_settings,
            settings.is_lefty_flip,
            settings.pathing_settings.engine->overlaps()};
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

SightRead::Beat subtract_video_lag(SightRead::Beat beat,
                                   SightRead::Second video_lag,
                                   const SightRead::TempoMap& tempo_map)
{
    const auto seconds = tempo_map.to_seconds(beat) - video_lag;
    if (seconds.value() < 0.0) {
        return SightRead::Beat {0.0};
    }
    return tempo_map.to_beats(seconds);
}

void apply_drum_settings(SightRead::NoteTrack& track,
                         const SightRead::Song& song,
                         const PathingSettings& pathing_settings)
{
    if (!pathing_settings.engine->is_rock_band()
        && track.drum_fills().empty()) {
        track.generate_drum_fills(song.global_data().tempo_map());
    }
    if (!pathing_settings.drum_settings.enable_dynamics) {
        track.disable_dynamics();
    }
    if (!pathing_settings.drum_settings.pro_drums) {
        track.disable_cymbals();
    }
}
}

void ImageBuilder::form_beat_lines(const SightRead::TempoMap& tempo_map)
{
    constexpr double HALF_BEAT = 0.5;

    for (const auto& row : m_rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length
                = get_beat_rate(tempo_map, SightRead::Beat {start});
            auto numer = get_numer(tempo_map, SightRead::Beat {start});
            auto denom = get_denom(tempo_map, SightRead::Beat {start});
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

bool ImageBuilder::is_neutralised_phrase(SightRead::Beat note_pos,
                                         const Path& path)
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
ImageBuilder::sp_phrase_bounds(const SightRead::StarPower& phrase,
                               const SightRead::NoteTrack& track,
                               const Path& path) const
{
    constexpr double MINIMUM_GREEN_RANGE_SIZE = 0.1;

    auto p = track.notes().cbegin();
    while (p->position < phrase.position) {
        ++p;
    }
    const auto& tempo_map = track.global_data().tempo_map();
    const auto start = tempo_map.to_beats(p->position).value();
    if (!m_overlap_engine
        && is_neutralised_phrase(SightRead::Beat {start}, path)) {
        return {-1, -1};
    }
    const auto phrase_end = phrase.position + phrase.length;
    auto end = start;
    while (p < track.notes().cend() && p->position < phrase_end) {
        SightRead::Tick max_length {0};
        for (auto length : p->lengths) {
            max_length = std::max(max_length, length);
        }
        auto current_end = tempo_map.to_beats(p->position + max_length).value();
        end = std::max(end, current_end);
        ++p;
    }
    end = std::max(end, start + MINIMUM_GREEN_RANGE_SIZE);
    return {start, end};
}

ImageBuilder::ImageBuilder(const SightRead::NoteTrack& track,
                           SightRead::Difficulty difficulty,
                           const SightRead::DrumSettings& drum_settings,
                           bool is_lefty_flip, bool is_overlap_engine)
    : m_track_type {track.track_type()}
    , m_difficulty {difficulty}
    , m_is_lefty_flip {is_lefty_flip}
    , m_rows {drawn_rows(track)}
    , m_notes {drawn_notes(track, drum_settings)}
    , m_overlap_engine {is_overlap_engine}
{
    form_beat_lines(track.global_data().tempo_map());
}

void ImageBuilder::add_bpms(const SightRead::TempoMap& tempo_map)
{
    constexpr double MS_PER_SECOND = 1000.0;

    m_bpms.clear();

    for (const auto& bpm : tempo_map.bpms()) {
        const auto pos = tempo_map.to_beats(bpm.position).value();
        const auto tempo = static_cast<double>(bpm.bpm) / MS_PER_SECOND;
        if (pos < m_rows.back().end) {
            m_bpms.emplace_back(pos, tempo);
        }
    }
}

void ImageBuilder::add_bre(const SightRead::BigRockEnding& bre,
                           const SightRead::TempoMap& tempo_map)
{
    const auto seconds_start = tempo_map.to_seconds(bre.start);
    const auto seconds_end = tempo_map.to_seconds(bre.end);
    const auto seconds_gap = seconds_end - seconds_start;
    const auto bre_value = static_cast<int>(750 + 500 * seconds_gap.value());

    m_total_score += bre_value;
    m_score_values.back() += bre_value;

    m_bre_ranges.emplace_back(tempo_map.to_beats(bre.start).value(),
                              tempo_map.to_beats(bre.end).value());
}

void ImageBuilder::add_drum_fills(const SightRead::NoteTrack& track)
{
    const auto& tempo_map = track.global_data().tempo_map();
    for (auto fill : track.drum_fills()) {
        m_fill_ranges.emplace_back(
            tempo_map.to_beats(fill.position).value(),
            tempo_map.to_beats(fill.position + fill.length).value());
    }
}

void ImageBuilder::add_measure_values(const PointSet& points,
                                      const SightRead::TempoMap& tempo_map,
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
                                 tempo_map)
                  .value();
        while (meas_iter != m_measure_lines.cend()
               && (std::next(meas_iter) != m_measure_lines.cend())
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
                                                       tempo_map)
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

void ImageBuilder::add_practice_sections(
    const std::vector<SightRead::PracticeSection>& practice_sections,
    const SightRead::TempoMap& tempo_map)
{
    for (const auto& section : practice_sections) {
        const auto pos = tempo_map.to_beats(section.start).value();
        m_practice_sections.emplace_back(pos, section.name);
    }
}

void ImageBuilder::add_solo_sections(const std::vector<SightRead::Solo>& solos,
                                     const SightRead::TempoMap& tempo_map)
{
    for (const auto& solo : solos) {
        const auto start = tempo_map.to_beats(solo.start).value();
        const auto end = tempo_map.to_beats(solo.end).value();
        m_solo_ranges.emplace_back(start, end);
    }
}

void ImageBuilder::add_song_header(const SightRead::SongGlobalData& global_data)
{
    m_song_name = global_data.name();
    m_artist = global_data.artist();
    m_charter = global_data.charter();
}

void ImageBuilder::add_sp_acts(const PointSet& points,
                               const SightRead::TempoMap& tempo_map,
                               const Path& path)
{
    std::vector<std::tuple<double, double>> no_whammy_ranges;

    const auto shifted_beat = [&](auto beat) {
        return subtract_video_lag(beat, points.video_lag(), tempo_map);
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
        blue_end = std::min(shifted_beat(blue_end),
                            SightRead::Beat {m_rows.back().end});
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
                                         const SpTimeMap& time_map,
                                         const PointSet& points,
                                         const Path& path)
{
    constexpr double SP_PHRASE_AMOUNT = 0.25;

    m_sp_percent_values.clear();
    m_sp_percent_values.reserve(m_measure_lines.size() - 1);

    const auto events = form_events(m_measure_lines, points, path);
    auto is_sp_active = false;
    SightRead::Beat whammy_end {std::numeric_limits<double>::infinity()};
    SightRead::Beat position {0.0};

    double total_sp = 0.0;
    for (auto [event_pos, event_type] : events) {
        if (is_sp_active) {
            SpPosition start_pos {position, time_map.to_sp_measures(position)};
            SpPosition end_pos {event_pos, time_map.to_sp_measures(event_pos)};
            SpPosition whammy_pos {SightRead::Beat {0.0}, SpMeasure {0.0}};
            if (m_overlap_engine) {
                whammy_pos = {whammy_end, time_map.to_sp_measures(whammy_end)};
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
            whammy_end
                = SightRead::Beat {std::numeric_limits<double>::infinity()};
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

void ImageBuilder::add_sp_phrases(
    const SightRead::NoteTrack& track,
    const std::vector<SightRead::Tick>& unison_phrases, const Path& path)
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

    if (engine.sp_gain_rate() == 0.0) {
        std::fill(m_sp_values.begin(), m_sp_values.end(), 0);
        return;
    }

    for (std::size_t i = 0; i < m_measure_lines.size() - 1; ++i) {
        SightRead::Beat start {m_measure_lines[i]};
        SightRead::Beat end {std::numeric_limits<double>::infinity()};
        if (i < m_measure_lines.size() - 1) {
            end = SightRead::Beat {m_measure_lines[i + 1]};
        }
        m_sp_values[i]
            = sp_data.available_whammy(start, end) / engine.sp_gain_rate();
    }
}

void ImageBuilder::add_time_sigs(const SightRead::TempoMap& tempo_map)
{
    for (const auto& ts : tempo_map.time_sigs()) {
        const auto pos = tempo_map.to_beats(ts.position).value();
        const auto num = ts.numerator;
        const auto denom = ts.denominator;
        if (pos < m_rows.back().end) {
            m_time_sigs.emplace_back(pos, num, denom);
        }
    }
}

void ImageBuilder::set_total_score(const PointSet& points,
                                   const std::vector<SightRead::Solo>& solos,
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

ImageBuilder make_builder(SightRead::Song& song,
                          const SightRead::NoteTrack& track,
                          const Settings& settings,
                          const std::function<void(const char*)>& write,
                          const std::atomic<bool>* terminate)
{
    auto new_track = track;
    if (song.global_data().is_from_midi()) {
        new_track = track.trim_sustains();
    }
    new_track
        = new_track.snap_chords(settings.pathing_settings.engine->snap_gap());
    if (track.track_type() == SightRead::TrackType::Drums) {
        apply_drum_settings(new_track, song, settings.pathing_settings);
    }
    song.speedup(settings.speed);
    const auto& tempo_map = song.global_data().tempo_map();
    const SpTimeMap time_map {tempo_map,
                              settings.pathing_settings.engine->sp_mode()};

    auto builder = build_with_engine_params(new_track, settings);
    builder.add_song_header(song.global_data());
    builder.add_practice_sections(song.global_data().practice_sections(),
                                  tempo_map);

    if (track.track_type() == SightRead::TrackType::Drums) {
        builder.add_drum_fills(new_track);
    }

    if (settings.draw_bpms) {
        builder.add_bpms(tempo_map);
    }

    const auto solos = new_track.solos(settings.pathing_settings.drum_settings);
    if (settings.draw_solos) {
        builder.add_solo_sections(solos, tempo_map);
    }

    if (settings.draw_time_sigs) {
        builder.add_time_sigs(tempo_map);
    }

    const auto unison_positions
        = (settings.pathing_settings.engine->has_unison_bonuses())
        ? song.unison_phrase_positions()
        : std::vector<SightRead::Tick> {};

    const ProcessedSong processed_track {
        new_track,
        {time_map, song.global_data().od_beats(), unison_positions},
        settings.pathing_settings};
    Path path;

    if (!settings.blank) {
        const auto is_rb_drums
            = track.track_type() == SightRead::TrackType::Drums
            && settings.pathing_settings.engine->is_rock_band();
        if (is_rb_drums) {
            write("Optimisation disabled for Rock Band drums, planned for a "
                  "future release");
            builder.add_sp_phrases(new_track, unison_positions, path);
        } else {
            write("Optimising, please wait...");
            const Optimiser optimiser {&processed_track, terminate,
                                       settings.speed,
                                       settings.pathing_settings.whammy_delay};
            path = optimiser.optimal_path();
            write(processed_track.path_summary(path).c_str());
            builder.add_sp_phrases(new_track, unison_positions, path);
            builder.add_sp_acts(processed_track.points(), tempo_map, path);
            builder.activation_opacity() = settings.opacity;
        }
    } else {
        builder.add_sp_phrases(new_track, unison_positions, path);
    }

    builder.add_measure_values(processed_track.points(), tempo_map, path);
    if (settings.blank || !settings.pathing_settings.engine->overlaps()) {
        builder.add_sp_values(processed_track.sp_data(),
                              *settings.pathing_settings.engine);
    } else {
        builder.add_sp_percent_values(processed_track.sp_data(), time_map,
                                      processed_track.points(), path);
    }
    builder.set_total_score(processed_track.points(), solos, path);
    if (settings.pathing_settings.engine->has_bres()
        && new_track.bre().has_value()) {
        const auto bre = new_track.bre();
        if (bre.has_value()) {
            builder.add_bre(*bre, tempo_map);
        }
    }

    return builder;
}
