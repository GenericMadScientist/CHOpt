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

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <sstream>

#include "processed.hpp"
#include "stringutil.hpp"

namespace {
int bre_boost(const NoteTrack& track, const Engine& engine)
{
    constexpr int INITIAL_BRE_VALUE = 750;
    constexpr int BRE_VALUE_PER_SECOND = 500;

    if (!engine.has_bres() || !track.bre().has_value()) {
        return 0;
    }
    const auto& tempo_map = track.global_data().tempo_map();
    const auto seconds_start = tempo_map.to_seconds(track.bre()->start);
    const auto seconds_end = tempo_map.to_seconds(track.bre()->end);
    const auto seconds_gap = seconds_end - seconds_start;
    return static_cast<int>(INITIAL_BRE_VALUE
                            + BRE_VALUE_PER_SECOND * seconds_gap.value());
}
}

SpBar ProcessedSong::sp_from_phrases(PointPtr begin, PointPtr end) const
{
    SpBar sp_bar {0.0, 0.0};
    for (auto p = m_points.next_sp_granting_note(begin); p < end;
         p = m_points.next_sp_granting_note(std::next(p))) {
        sp_bar.add_phrase();
        if (p->is_unison_sp_granting_note) {
            sp_bar.add_phrase();
        }
    }

    return sp_bar;
}

ProcessedSong::ProcessedSong(const NoteTrack& track, const TempoMap& tempo_map,
                             const SqueezeSettings& squeeze_settings,
                             const DrumSettings& drum_settings,
                             const Engine& engine,
                             const std::vector<Tick>& od_beats,
                             const std::vector<Tick>& unison_phrases)
    : m_tempo_map {tempo_map}
    , m_converter {tempo_map, engine, od_beats}
    , m_points {track,          tempo_map,        m_converter,
                unison_phrases, squeeze_settings, drum_settings,
                engine}
    , m_sp_data {track, tempo_map, od_beats, squeeze_settings, engine}
    , m_total_bre_boost {bre_boost(track, engine)}
    , m_base_score {track.base_score(drum_settings)}
    , m_ignore_average_multiplier {engine.ignore_average_multiplier()}
    , m_is_drums {track.track_type() == TrackType::Drums}
    , m_overlaps {engine.overlaps()}
{
    const auto solos = track.solos(drum_settings);
    m_total_solo_boost = std::accumulate(
        solos.cbegin(), solos.cend(), 0,
        [](const auto x, const auto& y) { return x + y.value; });
}

SpBar ProcessedSong::total_available_sp(Beat start, PointPtr first_point,
                                        PointPtr act_start,
                                        Beat required_whammy_end) const
{
    auto sp_bar = sp_from_phrases(first_point, act_start);

    if (start >= required_whammy_end) {
        sp_bar.max()
            += m_sp_data.available_whammy(start, act_start->position.beat);
        sp_bar.max() = std::min(sp_bar.max(), 1.0);
    } else if (required_whammy_end >= act_start->position.beat) {
        sp_bar.min()
            += m_sp_data.available_whammy(start, act_start->position.beat);
        sp_bar.min() = std::min(sp_bar.min(), 1.0);
        sp_bar.max() = sp_bar.min();
    } else {
        sp_bar.min() += m_sp_data.available_whammy(start, required_whammy_end);
        sp_bar.min() = std::min(sp_bar.min(), 1.0);
        sp_bar.max() = sp_bar.min();
        sp_bar.max() += m_sp_data.available_whammy(required_whammy_end,
                                                   act_start->position.beat);
        sp_bar.max() = std::min(sp_bar.max(), 1.0);
    }

    return sp_bar;
}

std::tuple<SpBar, Position> ProcessedSong::total_available_sp_with_earliest_pos(
    Beat start, PointPtr first_point, PointPtr act_start,
    Position earliest_potential_pos) const
{
    const Beat BEAT_EPSILON {0.0001};

    auto sp_bar = sp_from_phrases(first_point, act_start);

    sp_bar.max() += m_sp_data.available_whammy(
        start, earliest_potential_pos.beat, act_start->position.beat);
    sp_bar.max() = std::min(sp_bar.max(), 1.0);

    if (sp_bar.full_enough_to_activate()) {
        return {sp_bar, earliest_potential_pos};
    }

    const auto extra_sp_required = 0.5 - sp_bar.max();
    auto first_beat = earliest_potential_pos.beat;
    auto last_beat = act_start->position.beat;
    if (m_sp_data.available_whammy(first_beat, last_beat,
                                   act_start->position.beat)
        < extra_sp_required) {
        return {sp_bar, earliest_potential_pos};
    }

    while (last_beat - first_beat > BEAT_EPSILON) {
        const auto mid_beat = (first_beat + last_beat) * 0.5;
        if (m_sp_data.available_whammy(earliest_potential_pos.beat, mid_beat,
                                       act_start->position.beat)
            < extra_sp_required) {
            first_beat = mid_beat;
        } else {
            last_beat = mid_beat;
        }
    }

    sp_bar.max() += m_sp_data.available_whammy(
        earliest_potential_pos.beat, last_beat, act_start->position.beat);
    sp_bar.max() = std::min(sp_bar.max(), 1.0);

    return {sp_bar,
            Position {last_beat, m_converter.beats_to_measures(last_beat)}};
}

Position ProcessedSong::adjusted_hit_window_start(PointPtr point,
                                                  double squeeze) const
{
    assert((0.0 <= squeeze) && (squeeze <= 1.0)); // NOLINT

    if (squeeze == 1.0) {
        return point->hit_window_start;
    }

    auto start = m_tempo_map.to_seconds(point->hit_window_start.beat);
    auto mid = m_tempo_map.to_seconds(point->position.beat);
    auto adj_start_s = start + (mid - start) * (1.0 - squeeze);
    auto adj_start_b = m_tempo_map.to_beats(adj_start_s);
    auto adj_start_m = m_converter.beats_to_measures(adj_start_b);

    return {adj_start_b, adj_start_m};
}

Position ProcessedSong::adjusted_hit_window_end(PointPtr point,
                                                double squeeze) const
{
    assert((0.0 <= squeeze) && (squeeze <= 1.0)); // NOLINT

    if (squeeze == 1.0) {
        return point->hit_window_end;
    }

    auto mid = m_tempo_map.to_seconds(point->position.beat);
    auto end = m_tempo_map.to_seconds(point->hit_window_end.beat);
    auto adj_end_s = mid + (end - mid) * squeeze;
    auto adj_end_b = m_tempo_map.to_beats(adj_end_s);
    auto adj_end_m = m_converter.beats_to_measures(adj_end_b);

    return {adj_end_b, adj_end_m};
}

class SpStatus {
private:
    Position m_position;
    double m_sp;
    bool m_overlap_engine;
    static constexpr double MEASURES_PER_BAR = 8.0;

public:
    SpStatus(Position position, double sp, bool overlap_engine)
        : m_position {position}
        , m_sp {sp}
        , m_overlap_engine {overlap_engine}
    {
    }

    [[nodiscard]] Position position() const { return m_position; }
    [[nodiscard]] double sp() const { return m_sp; }

    void add_phrase()
    {
        constexpr double SP_PHRASE_AMOUNT = 0.25;

        m_sp += SP_PHRASE_AMOUNT;
        m_sp = std::min(m_sp, 1.0);
    }

    void advance_whammy_max(Position end_position, const SpData& sp_data,
                            bool does_overlap)
    {
        if (does_overlap) {
            m_sp = sp_data.propagate_sp_over_whammy_max(m_position,
                                                        end_position, m_sp);
        } else {
            m_sp -= (end_position.measure - m_position.measure).value()
                / MEASURES_PER_BAR;
        }
        m_position = end_position;
    }

    void update_early_end(Position sp_note_start, const SpData& sp_data,
                          Position required_whammy_end)
    {
        if (!m_overlap_engine) {
            required_whammy_end = {Beat {0.0}, Measure {0.0}};
        }
        m_sp = sp_data.propagate_sp_over_whammy_min(m_position, sp_note_start,
                                                    m_sp, required_whammy_end);
        if (sp_note_start.beat > m_position.beat) {
            m_position = sp_note_start;
        }
    }

    void update_late_end(Position sp_note_start, Position sp_note_end,
                         const SpData& sp_data, bool does_overlap)
    {
        if (sp_note_start.beat < m_position.beat) {
            sp_note_start = m_position;
        }

        advance_whammy_max(sp_note_start, sp_data, does_overlap);
        if (m_sp < 0.0) {
            return;
        }
        // We might run out of SP between sp_note_start and sp_note_end. In this
        // case we just hit the note as early as possible.
        if (does_overlap) {
            const auto new_sp = sp_data.propagate_sp_over_whammy_max(
                sp_note_start, sp_note_end, m_sp);
            if (new_sp >= 0.0) {
                m_sp = new_sp;
                m_position = sp_note_end;
            }
        }
    }
};

ActResult
ProcessedSong::is_candidate_valid(const ActivationCandidate& activation,
                                  double squeeze,
                                  Position required_whammy_end) const
{
    static constexpr double MEASURES_PER_BAR = 8.0;
    static constexpr double MINIMUM_SP_AMOUNT = 0.5;
    const Position null_position {Beat(0.0), Measure(0.0)};

    if (!activation.sp_bar.full_enough_to_activate()) {
        return {null_position, ActValidity::insufficient_sp};
    }

    auto ending_pos = adjusted_hit_window_start(activation.act_end, squeeze);
    if (ending_pos.beat < activation.earliest_activation_point.beat) {
        ending_pos = activation.earliest_activation_point;
    }

    auto late_end_position
        = adjusted_hit_window_end(activation.act_start, squeeze);
    // This conditional can be taken if, for example, the first and last point
    // are the same.
    if (late_end_position.beat > ending_pos.beat) {
        late_end_position = ending_pos;
    }
    auto late_end_sp = activation.sp_bar.max();
    late_end_sp
        += m_sp_data.available_whammy(activation.earliest_activation_point.beat,
                                      activation.act_start->position.beat);
    late_end_sp = std::min(late_end_sp, 1.0);

    SpStatus status_for_early_end {
        activation.earliest_activation_point,
        std::max(activation.sp_bar.min(), MINIMUM_SP_AMOUNT), m_overlaps};
    SpStatus status_for_late_end {late_end_position, late_end_sp, m_overlaps};

    for (auto p = m_points.next_sp_granting_note(activation.act_start);
         p < activation.act_end;
         p = m_points.next_sp_granting_note(std::next(p))) {
        auto p_start = adjusted_hit_window_start(p, squeeze);
        if (p_start.beat < activation.earliest_activation_point.beat) {
            p_start = activation.earliest_activation_point;
        }
        auto p_end = adjusted_hit_window_end(p, squeeze);
        if (p_end.beat > ending_pos.beat) {
            p_end = ending_pos;
        }
        status_for_late_end.update_late_end(p_start, p_end, m_sp_data,
                                            m_overlaps);
        if (status_for_late_end.sp() < 0.0) {
            return {null_position, ActValidity::insufficient_sp};
        }
        status_for_early_end.update_early_end(p_start, m_sp_data,
                                              required_whammy_end);
        if (m_overlaps) {
            status_for_early_end.add_phrase();
            status_for_late_end.add_phrase();
            if (p->is_unison_sp_granting_note) {
                status_for_early_end.add_phrase();
                status_for_late_end.add_phrase();
            }
        }
    }

    status_for_late_end.advance_whammy_max(ending_pos, m_sp_data, m_overlaps);
    if (status_for_late_end.sp() < 0.0) {
        return {null_position, ActValidity::insufficient_sp};
    }

    status_for_early_end.update_early_end(ending_pos, m_sp_data,
                                          required_whammy_end);
    if (m_overlaps && activation.act_end->is_sp_granting_note) {
        status_for_early_end.add_phrase();
        if (activation.act_end->is_unison_sp_granting_note) {
            status_for_early_end.add_phrase();
        }
    }
    const auto end_meas = status_for_early_end.position().measure
        + Measure(status_for_early_end.sp() * MEASURES_PER_BAR);

    const auto next_point = std::next(activation.act_end);
    if (next_point != m_points.cend()
        && end_meas >= adjusted_hit_window_end(next_point, squeeze).measure) {
        return {null_position, ActValidity::surplus_sp};
    }

    const auto end_beat = m_converter.measures_to_beats(end_meas);
    return {{end_beat, end_meas}, ActValidity::success};
}

void ProcessedSong::append_activation(std::stringstream& stream,
                                      const Activation& activation,
                                      const std::string& act_summary) const
{
    stream << '\n' << act_summary.substr(0, act_summary.find('-')) << ": ";
    if (act_summary[0] == '0') {
        stream << "See image";
        return;
    }
    const auto act_start = activation.act_start;
    auto previous_sp_note = std::prev(act_start);
    while (!previous_sp_note->is_sp_granting_note) {
        --previous_sp_note;
    }
    const auto count
        = std::count_if(std::next(previous_sp_note), std::next(act_start),
                        [](const auto& p) { return !p.is_hold_point; });
    if (act_start->is_hold_point) {
        auto starting_note = act_start;
        while (starting_note->is_hold_point) {
            --starting_note;
        }
        const auto beat_gap
            = act_start->position.beat - starting_note->position.beat;
        if (count > 0) {
            stream << beat_gap.value() << " beats after ";
        } else {
            stream << "After " << beat_gap.value() << " beats";
        }
    }
    if (count > 1) {
        auto previous_note = act_start;
        while (previous_note->is_hold_point) {
            --previous_note;
        }
        const auto colour = m_points.colour_set(previous_note);
        auto same_colour_count = 1;
        for (auto p = std::next(previous_sp_note); p < previous_note; ++p) {
            if (p->is_hold_point) {
                continue;
            }
            if (m_points.colour_set(p) == colour) {
                ++same_colour_count;
            }
        }
        stream << to_ordinal(static_cast<int>(same_colour_count)) << ' '
               << colour;
    } else if (count == 1) {
        stream << "NN";
    }
    const auto act_end = activation.act_end;
    if (!act_end->is_hold_point) {
        stream << " (" << m_points.colour_set(act_end) << ")";
    }
}

std::vector<std::string> ProcessedSong::act_summaries(const Path& path) const
{
    using namespace std::literals::string_literals;

    std::vector<std::string> activation_summaries;
    auto start_point = m_points.cbegin();
    for (const auto& act : path.activations) {
        const auto sp_before
            = std::count_if(start_point, act.act_start, [](const auto& p) {
                  return p.is_sp_granting_note;
              });
        const auto sp_during = std::count_if(
            act.act_start, std::next(act.act_end),
            [](const auto& p) { return p.is_sp_granting_note; });
        auto summary = std::to_string(sp_before);
        if (sp_during != 0) {
            if (m_overlaps) {
                summary += "(+"s + std::to_string(sp_during) + ')';
            } else {
                summary += "-S"s + std::to_string(sp_during);
            }
        }
        activation_summaries.push_back(summary);
        start_point = std::next(act.act_end);
    }

    const auto spare_sp
        = std::count_if(start_point, m_points.cend(),
                        [](const auto& p) { return p.is_sp_granting_note; });
    if (spare_sp != 0) {
        activation_summaries.push_back(std::string("ES")
                                       + std::to_string(spare_sp));
    }

    return activation_summaries;
}

std::vector<std::string>
ProcessedSong::drum_act_summaries(const Path& path) const
{
    std::vector<std::string> activation_summaries;
    auto start_point = m_points.cbegin();
    for (const auto& act : path.activations) {
        assert(act.act_start->fill_start.has_value()); // NOLINT
        int sp_count = 0;
        while (sp_count < 2) {
            if (start_point->is_sp_granting_note) {
                ++sp_count;
            }
            ++start_point;
        }
        const auto early_fill_point
            = m_tempo_map.to_seconds(
                  std::prev(start_point)->hit_window_start.beat)
            + Second(2.0);
        const auto late_fill_point
            = m_tempo_map.to_seconds(
                  std::prev(start_point)->hit_window_end.beat)
            + Second(2.0);
        const auto skipped_fills
            = std::count_if(start_point, act.act_start, [&](const auto& p) {
                  return p.fill_start.has_value()
                      && *p.fill_start >= early_fill_point;
              });
        if (skipped_fills == 0
            && late_fill_point > *act.act_start->fill_start) {
            activation_summaries.emplace_back("0(E)");
        } else if (skipped_fills > 0) {
            while (!start_point->fill_start.has_value()) {
                ++start_point;
            }
            if (late_fill_point > *start_point->fill_start
                && early_fill_point < *start_point->fill_start) {
                activation_summaries.push_back(std::to_string(skipped_fills - 1)
                                               + "(L)");
            } else {
                activation_summaries.push_back(std::to_string(skipped_fills));
            }
        } else {
            activation_summaries.push_back(std::to_string(skipped_fills));
        }
        start_point = std::next(act.act_end);
    }

    return activation_summaries;
}

std::string ProcessedSong::path_summary(const Path& path) const
{
    constexpr double AVG_MULT_PRECISION = 1000.0;

    // We use std::stringstream instead of std::string for better formating of
    // floats (average multiplier and mid-sustain activation positions).
    std::stringstream stream;
    stream << "Path: ";

    const auto activation_summaries
        = m_is_drums ? drum_act_summaries(path) : act_summaries(path);
    if (activation_summaries.empty()) {
        stream << "None";
    } else {
        stream << activation_summaries[0];
        for (std::size_t i = 1; i < activation_summaries.size(); ++i) {
            stream << '-' << activation_summaries[i];
        }
    }

    auto no_sp_score = std::accumulate(
        m_points.cbegin(), m_points.cend(), 0,
        [](const auto x, const auto& y) { return x + y.value; });
    no_sp_score += m_total_solo_boost;
    no_sp_score += m_total_bre_boost;
    stream << "\nNo SP score: " << no_sp_score;

    const auto total_score = no_sp_score + path.score_boost;
    stream << "\nTotal score: " << total_score;

    if (!m_ignore_average_multiplier) {
        double avg_mult = 0;
        if (m_base_score != 0) {
            auto int_avg_mult = 0;
            auto stars_score = total_score - m_total_solo_boost;
            for (auto i = 0; i < 4; ++i) {
                int_avg_mult *= 10; // NOLINT
                int_avg_mult += (stars_score / m_base_score);
                stars_score %= m_base_score;
                stars_score *= 10; // NOLINT
            }
            avg_mult = static_cast<double>(int_avg_mult) / AVG_MULT_PRECISION;
        }
        stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
        stream << std::setprecision(3);
        stream << "\nAverage multiplier: " << avg_mult << 'x';
    }

    if (!m_is_drums) {
        stream << std::setprecision(2);
        for (std::size_t i = 0; i < path.activations.size(); ++i) {
            append_activation(stream, path.activations[i],
                              activation_summaries[i]);
        }
    }

    return stream.str();
}
