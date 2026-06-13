/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2023, 2025, 2026 Raymond Wright
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
#include <iterator>
#include <ranges>

#include "sp.hpp"

namespace {
bool is_note_part_of_phrase(const std::vector<SightRead::StarPower>& phrases,
                            const SightRead::Note& note)
{
    const auto candidate_phrase = std::ranges::upper_bound(
        phrases, note.position, {},
        [](const auto& phrase) { return phrase.position + phrase.length; });
    return candidate_phrase != std::ranges::end(phrases)
        && note.position >= candidate_phrase->position;
}

double sp_deduction(SpPosition start, SpPosition end)
{
    constexpr double MEASURES_PER_BAR = 8.0;

    const auto meas_diff = end.sp_measure - start.sp_measure;
    return meas_diff.value() / MEASURES_PER_BAR;
}

struct ExtendedSustain {
    SightRead::Tick start;
    SightRead::Tick end;
    SightRead::Tick terminating_note_position;
};

class ExtendedSustainGroups {
private:
    std::vector<ExtendedSustain> m_sustains;

    static SightRead::Tick note_end(const SightRead::Note& note)
    {
        const auto max_length = *std::ranges::max_element(note.lengths);
        return note.position + max_length;
    }

    void append_sustain(std::optional<ExtendedSustain> sustain)
    {
        if (sustain.has_value()) {
            m_sustains.push_back(*sustain);
        }
    }

public:
    explicit ExtendedSustainGroups(const std::vector<SightRead::Note>& notes)
    {
        std::optional<ExtendedSustain> current_sustain;
        for (const auto& note : notes) {
            if (!current_sustain.has_value()
                || note.position >= current_sustain->end) {
                append_sustain(current_sustain);
                current_sustain = {.start = note.position,
                                   .end = note_end(note),
                                   .terminating_note_position = note.position};
            }

            const auto new_end = note_end(note);
            if (current_sustain.has_value() && new_end > current_sustain->end) {
                current_sustain->end = new_end;
                current_sustain->terminating_note_position = note.position;
            }
        }

        append_sustain(current_sustain);
    }

    [[nodiscard]] bool
    is_extended_sustain_ender(const SightRead::Note& note) const
    {
        auto sustain = std::ranges::lower_bound(
            m_sustains, note.position, {},
            [](const auto& sustain) { return sustain.start; });
        if (sustain != std::ranges::begin(m_sustains)
            && (sustain == std::ranges::end(m_sustains)
                || sustain->start > note.position)) {
            --sustain;
        }

        assert(sustain != std::ranges::end(m_sustains));
        return sustain->terminating_note_position == note.position;
    }
};

std::vector<SpSustain> sp_whammy_spans(const SightRead::NoteTrack& track,
                                       const PathingSettings& pathing_settings,
                                       const SpTimeMap& time_map)
{
    const auto& tempo_map = track.global_data().tempo_map();
    const ExtendedSustainGroups extended_sustains {track.notes()};
    std::vector<SpSustain> spans;
    for (auto note = track.notes().cbegin(); note < track.notes().cend();
         ++note) {
        if (!is_note_part_of_phrase(track.sp_phrases(), *note)) {
            continue;
        }

        auto early_gap = std::numeric_limits<double>::infinity();
        auto late_gap = std::numeric_limits<double>::infinity();
        const auto current_note_time
            = tempo_map.to_seconds(note->position).value();
        if (note != track.notes().cbegin()) {
            early_gap = current_note_time
                - tempo_map.to_seconds(std::prev(note)->position).value();
        }
        if (std::next(note) < track.notes().cend()) {
            late_gap = tempo_map.to_seconds(std::next(note)->position).value()
                - current_note_time;
        }
        std::set<SightRead::Tick> sustain_lengths;
        for (auto length : note->lengths) {
            if (length > SightRead::Tick {0}) {
                sustain_lengths.insert(length);
            }
        }
        for (auto length : std::views::reverse(sustain_lengths)) {
            SightRead::Second early_timing_window {0};
            if (pathing_settings.engine->has_early_whammy()) {
                early_timing_window
                    = SightRead::Second {pathing_settings.engine
                                             ->early_timing_window(early_gap,
                                                                   late_gap)}
                    * pathing_settings.early_whammy;
            }

            const auto whammy_start_beat = tempo_map.to_beats(
                tempo_map.to_seconds(note->position) - early_timing_window);
            const SpPosition whammy_start {
                .beat = whammy_start_beat,
                .sp_measure = time_map.to_sp_measures(whammy_start_beat)};
            const auto whammy_end_beat
                = tempo_map.to_beats(note->position + length);
            const SpPosition whammy_end {
                .beat = whammy_end_beat,
                .sp_measure = time_map.to_sp_measures(whammy_end_beat)};
            auto burst_position = whammy_end;
            if (pathing_settings.engine->has_whammy_bursts()) {
                burst_position.beat
                    -= SightRead::Beat {pathing_settings.engine->burst_size()};
                burst_position.beat
                    = std::max(burst_position.beat, whammy_start.beat);
                burst_position.sp_measure
                    = time_map.to_sp_measures(burst_position.beat);
            }

            spans.emplace_back(
                note->position, whammy_start, whammy_end, burst_position,
                length == *sustain_lengths.rbegin()
                    && extended_sustains.is_extended_sustain_ender(*note));
        }
    }

    return spans;
}
}

std::vector<SpData::BeatRate>
SpData::form_beat_rates(const SightRead::TempoMap& tempo_map,
                        const std::vector<SightRead::Tick>& od_beats,
                        const Engine& engine)
{
    constexpr double DEFAULT_BEAT_RATE = 4.0;

    std::vector<BeatRate> beat_rates;
    if (engine.sp_mode() == SpMode::Measure) {
        beat_rates.reserve(tempo_map.time_sigs().size());

        for (const auto& ts : tempo_map.time_sigs()) {
            const auto pos = tempo_map.to_beats(ts.position);
            const auto measure_rate
                = ts.numerator * DEFAULT_BEAT_RATE / ts.denominator;
            const auto drain_rate
                = engine.sp_gain_rate() - 1 / (MEASURES_PER_BAR * measure_rate);
            beat_rates.push_back(
                {.position = pos, .net_sp_gain_rate = drain_rate});
        }
    } else if (!od_beats.empty()) {
        beat_rates.reserve(od_beats.size() - 1);

        for (auto i = 0U; i < od_beats.size() - 1; ++i) {
            const auto pos = tempo_map.to_beats(od_beats.at(i));
            const auto next_marker = tempo_map.to_beats(od_beats.at(i + 1));
            const auto drain_rate = engine.sp_gain_rate()
                - 1 / (DEFAULT_BEATS_PER_BAR * (next_marker - pos).value());
            beat_rates.push_back(
                {.position = pos, .net_sp_gain_rate = drain_rate});
        }
    } else {
        beat_rates.push_back({.position = SightRead::Beat(0.0),
                              .net_sp_gain_rate = engine.sp_gain_rate()
                                  - 1 / DEFAULT_BEATS_PER_BAR});
    }

    return beat_rates;
}

SpData::SpData(const SightRead::NoteTrack& track,
               const SpDurationData& duration_data,
               const PathingSettings& pathing_settings)
    : m_time_map {duration_data.time_map}
    , m_gain_mode {pathing_settings.engine->sp_gain_mode()}
    , m_beat_rates {form_beat_rates(track.global_data().tempo_map(),
                                    duration_data.od_beats,
                                    *pathing_settings.engine)}
    , m_sp_gain_rate {pathing_settings.engine->sp_gain_rate()}
    , m_default_net_sp_gain_rate {m_sp_gain_rate - 1 / DEFAULT_BEATS_PER_BAR}
{
    m_sp_sustains = sp_whammy_spans(track, pathing_settings, m_time_map);
    for (auto& sustain : m_sp_sustains) {
        auto second_start = m_time_map.to_seconds(sustain.whammy_start.beat);
        second_start += pathing_settings.lazy_whammy;
        second_start += pathing_settings.video_lag;
        sustain.whammy_start
            = {.beat = m_time_map.to_beats(second_start),
               .sp_measure = m_time_map.to_sp_measures(second_start)};
        if (pathing_settings.engine->sustain_ticks_metric()
            == SustainTicksMetric::Fretbar) {
            const auto burst_size
                = m_time_map.to_seconds(SightRead::Fretbar {0.25});
            const auto whammy_end_seconds
                = m_time_map.to_seconds(sustain.whammy_end.beat) - burst_size;
            sustain.whammy_end.beat = m_time_map.to_beats(whammy_end_seconds);
            sustain.whammy_end.sp_measure
                = m_time_map.to_sp_measures(whammy_end_seconds);
            sustain.burst_position = sustain.whammy_end;
        }
    }

    const auto [first, last]
        = std::ranges::remove_if(m_sp_sustains, [](const auto& sustain) {
              return sustain.whammy_start.beat >= sustain.whammy_end.beat;
          });
    m_sp_sustains.erase(first, last);

    if (m_sp_sustains.empty()) {
        return;
    }

    const auto latest_ending_sp_sustain = std::ranges::max_element(
        m_sp_sustains, std::less {},
        [](const auto& sust) { return sust.whammy_end.beat; });
    m_last_whammy_point = latest_ending_sp_sustain->whammy_end.beat;
    auto p = m_sp_sustains.cbegin();
    for (auto pos = 0; pos < m_last_whammy_point.value(); ++pos) {
        p = std::find_if_not(p, m_sp_sustains.cend(), [=](const auto& x) {
            return x.whammy_end.beat.value() <= pos;
        });
        m_initial_guesses.push_back(p);
    }
}

std::vector<SpSustain>::const_iterator
SpData::first_sp_sustain_after(SightRead::Beat pos) const
{
    if (m_last_whammy_point <= pos) {
        return m_sp_sustains.cend();
    }
    const auto index = static_cast<std::size_t>(pos.value());
    const auto begin = (pos < SightRead::Beat(0.0))
        ? m_sp_sustains.cbegin()
        : m_initial_guesses.at(index);

    return std::find_if_not(begin, m_sp_sustains.cend(), [=](const auto& x) {
        return x.whammy_end.beat <= pos;
    });
}

SpData::WhammyPropagationState
SpData::initial_whammy_prop_state(SightRead::Beat start, SightRead::Beat end,
                                  double sp_bar_amount) const
{
    auto p
        = std::ranges::lower_bound(m_beat_rates, start, std::less {},
                                   [](const auto& ts) { return ts.position; });
    if (p != m_beat_rates.cbegin()) {
        --p;
    } else {
        const auto subrange_end = std::min(end, p->position);
        const auto sp_gain
            = (subrange_end - start).value() * m_default_net_sp_gain_rate;
        sp_bar_amount += sp_gain;
        sp_bar_amount = std::min(sp_bar_amount, 1.0);
        start = subrange_end;
    }
    return {.current_beat_rate = p,
            .current_position = start,
            .current_sp = sp_bar_amount};
}

double SpData::add_reserved_burst(double sp,
                                  SightRead::Beat& reserved_burst_size) const
{
    if (reserved_burst_size.value() > 0.0) {
        sp += reserved_burst_size.value() * m_sp_gain_rate;
        sp = std::min(sp, 1.0);
    }

    reserved_burst_size = SightRead::Beat {0.0};
    return sp;
}

double
SpData::propagate_sp_over_whammy_max(SpPosition start, SpPosition end,
                                     double sp,
                                     SightRead::Beat last_burst_position) const
{
    assert(start.beat <= end.beat);

    SightRead::Beat reserved_burst_size {0.0};
    SpPosition end_of_reserved_burst {.beat = SightRead::Beat {0.0},
                                      .sp_measure = SpMeasure {0.0}};
    for (auto p = first_sp_sustain_after(start.beat);
         (p != m_sp_sustains.cend()) && (p->whammy_start.beat < end.beat);
         ++p) {
        if (p->burst_position.beat <= last_burst_position) {
            continue;
        }
        if (p->whammy_start.beat > start.beat) {
            SpPosition sustain_start = p->whammy_start;
            const auto meas_diff = sustain_start.sp_measure - start.sp_measure;
            sp -= meas_diff.value() / MEASURES_PER_BAR;
            if (sp < 0.0) {
                sp = add_reserved_burst(sp, reserved_burst_size);
                if (sp < 0.0) {
                    return sp;
                }
            }
            start = sustain_start;
        }

        const auto sustain_end
            = p->releasable_for_burst ? p->burst_position : p->whammy_end;
        const auto range_end = std::min(end.beat, sustain_end.beat);

        sp = add_reserved_burst(sp, reserved_burst_size);
        if (start.beat < range_end) {
            sp = propagate_over_whammy_range(start.beat, range_end, sp);
        }
        if (sp < 0.0 || sustain_end.beat >= end.beat) {
            return sp;
        }
        start = sustain_end;
        reserved_burst_size = p->whammy_end.beat - sustain_end.beat;
        end_of_reserved_burst = p->whammy_end;
        last_burst_position = p->burst_position.beat;
    }

    if (reserved_burst_size.value() > 0.0
        && end_of_reserved_burst.sp_measure > start.sp_measure) {
        const auto meas_diff
            = end_of_reserved_burst.sp_measure - start.sp_measure;
        sp -= meas_diff.value() / MEASURES_PER_BAR;
        sp = add_reserved_burst(sp, reserved_burst_size);
        start = end_of_reserved_burst;
    }

    const auto meas_diff = end.sp_measure - start.sp_measure;
    sp -= meas_diff.value() / MEASURES_PER_BAR;

    return sp;
}

double
SpData::propagate_sp_over_whammy_min(SpPosition start, SpPosition end,
                                     double sp,
                                     SpPosition required_whammy_end) const
{
    assert(start.beat <= end.beat);
    if (required_whammy_end.beat > start.beat) {
        auto whammy_end = end;
        if (required_whammy_end.beat < end.beat) {
            whammy_end = required_whammy_end;
        }
        sp = propagate_sp_over_whammy_max(start, whammy_end, sp);
        start = required_whammy_end;
    }
    if (start.beat < end.beat) {
        const auto meas_diff = end.sp_measure - start.sp_measure;
        sp -= meas_diff.value() / MEASURES_PER_BAR;
    }

    sp = std::max(sp, 0.0);
    return sp;
}

double SpData::propagate_over_whammy_range(SightRead::Beat start,
                                           SightRead::Beat end,
                                           double sp_bar_amount) const
{
    assert(start <= end);

    auto state = initial_whammy_prop_state(start, end, sp_bar_amount);
    while (state.current_position < end) {
        auto subrange_end = end;
        if (std::next(state.current_beat_rate) != m_beat_rates.cend()) {
            subrange_end
                = std::min(end, std::next(state.current_beat_rate)->position);
        }
        state.current_sp += (subrange_end - state.current_position).value()
            * state.current_beat_rate->net_sp_gain_rate;
        if (state.current_sp < 0.0) {
            return -1.0;
        }
        state.current_sp = std::min(state.current_sp, 1.0);
        state.current_position = subrange_end;
        ++state.current_beat_rate;
    }

    return state.current_sp;
}

bool SpData::is_in_whammy_ranges(SightRead::Beat beat) const
{
    const auto p = first_sp_sustain_after(beat);
    if (p == m_sp_sustains.cend()) {
        return false;
    }
    return p->whammy_start.beat <= beat;
}

double SpData::sp_from_whammying_range(SightRead::Beat start,
                                       SightRead::Beat end) const
{
    if (start >= end) {
        return 0.0;
    }

    auto gain_interval = (end - start).value();
    if (m_gain_mode == SpGainMode::Fretbar) {
        gain_interval
            = (m_time_map.to_fretbars(end) - m_time_map.to_fretbars(start))
                  .value();
    }

    return gain_interval * m_sp_gain_rate;
}

double SpData::available_whammy(SightRead::Beat start, SightRead::Beat end,
                                SightRead::Tick note_pos) const
{
    double total_whammy {0.0};
    SightRead::Beat last_burst_position {
        -std::numeric_limits<double>::infinity()};

    for (auto p = first_sp_sustain_after(start);
         p < m_sp_sustains.cend() && p->whammy_start.beat < end
         && p->note_position < note_pos && start < end;
         ++p) {
        if (p->burst_position.beat <= last_burst_position) {
            continue;
        }
        const auto whammy_start = std::max(p->whammy_start.beat, start);
        const auto whammy_end = std::min(p->whammy_end.beat, end);
        total_whammy += sp_from_whammying_range(whammy_start, whammy_end);
        if (p->releasable_for_burst) {
            start = std::max(start, p->burst_position.beat);
        } else {
            start = std::max(start, p->whammy_end.beat);
        }
        last_burst_position = p->burst_position.beat;
    }

    return total_whammy;
}

SpPosition SpData::sp_drain_end_point(SpPosition start,
                                      double sp_bar_amount) const
{
    const auto end_meas
        = start.sp_measure + SpMeasure(sp_bar_amount * MEASURES_PER_BAR);
    const auto end_beat = m_time_map.to_beats(end_meas);
    return {.beat = end_beat, .sp_measure = end_meas};
}

SpPosition SpData::activation_end_point(SpPosition start, SpPosition end,
                                        double sp_bar_amount) const
{
    auto p = first_sp_sustain_after(start.beat);
    while ((p != m_sp_sustains.cend()) && (p->whammy_start.beat < end.beat)) {
        if (p->whammy_start.beat > start.beat) {
            SpPosition sustain_start = p->whammy_start;
            const auto sp_drop = sp_deduction(start, sustain_start);
            if (sp_bar_amount < sp_drop) {
                return sp_drain_end_point(start, sp_bar_amount);
            }
            sp_bar_amount -= sp_drop;
            start = sustain_start;
        }
        const auto range_end = std::min(end.beat, p->whammy_end.beat);
        const auto new_sp_bar_amount
            = propagate_over_whammy_range(start.beat, range_end, sp_bar_amount);
        if (new_sp_bar_amount < 0.0) {
            const auto end_beat = whammy_propagation_endpoint(
                start.beat, end.beat, sp_bar_amount);
            const auto end_meas = m_time_map.to_sp_measures(end_beat);
            return {.beat = end_beat, .sp_measure = end_meas};
        }
        sp_bar_amount = new_sp_bar_amount;
        if (p->whammy_end.beat >= end.beat) {
            return end;
        }
        start = p->whammy_end;
        ++p;
    }

    const auto sp_drop = sp_deduction(start, end);
    if (sp_bar_amount < sp_drop) {
        return sp_drain_end_point(start, sp_bar_amount);
    }
    return end;
}

// Return the point whammy runs out if all of the range [start, end) is
// whammied.
SightRead::Beat SpData::whammy_propagation_endpoint(SightRead::Beat start,
                                                    SightRead::Beat end,
                                                    double sp_bar_amount) const
{
    auto state = initial_whammy_prop_state(start, end, sp_bar_amount);
    while (state.current_position < end) {
        auto subrange_end = end;
        if (std::next(state.current_beat_rate) != m_beat_rates.cend()) {
            subrange_end
                = std::min(end, std::next(state.current_beat_rate)->position);
        }
        auto sp_gain = (subrange_end - state.current_position).value()
            * state.current_beat_rate->net_sp_gain_rate;
        if (state.current_sp + sp_gain < 0.0) {
            return state.current_position
                + SightRead::Beat(-state.current_sp
                                  / state.current_beat_rate->net_sp_gain_rate);
        }
        state.current_sp += sp_gain;
        state.current_sp = std::min(state.current_sp, 1.0);
        state.current_position = subrange_end;
        ++state.current_beat_rate;
    }

    return end;
}

SightRead::Beat SpData::next_whammy_point(SightRead::Beat pos) const
{
    const auto p = first_sp_sustain_after(pos);
    if (p == m_sp_sustains.cend()) {
        return SightRead::Beat {std::numeric_limits<double>::infinity()};
    }
    return std::max(p->whammy_start.beat, pos);
}
