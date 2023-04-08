/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2023 Raymond Wright
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

#include "songparts.hpp"

namespace {
Note combined_note(std::vector<Note>::const_iterator begin,
                   std::vector<Note>::const_iterator end)
{
    Note note = *begin;
    ++begin;
    while (begin < end) {
        for (auto i = 0; i < 7; ++i) {
            if (begin->lengths[i] != -1) {
                note.lengths[i] = begin->lengths[i];
            }
        }
        ++begin;
    }
    return note;
}
}

int Note::open_index() const
{
    if (flags & FLAGS_FIVE_FRET_GUITAR) {
        return FIVE_FRET_OPEN;
    }
    if (flags & FLAGS_SIX_FRET_GUITAR) {
        return SIX_FRET_OPEN;
    }
    return -1;
}

int Note::colours() const
{
    auto colour_flags = 0;
    for (auto i = 0U; i < lengths.size(); ++i) {
        if (lengths[i] != -1) {
            colour_flags |= 1 << i;
        }
    }
    return colour_flags;
}

void Note::merge_non_opens_into_open()
{
    const auto index = open_index();
    const auto open_length = lengths[index];
    if (index == -1 || open_length == -1) {
        return;
    }
    for (auto i = 0; i < 7; ++i) {
        if (i != index && lengths[i] == open_length) {
            lengths[i] = -1;
        }
    }
}

void Note::disable_dynamics()
{
    flags = static_cast<NoteFlags>(flags & ~(FLAGS_GHOST | FLAGS_ACCENT));
}

bool Note::is_kick_note() const
{
    return (flags & FLAGS_DRUMS)
        && (lengths[DRUM_KICK] != -1 || lengths[DRUM_DOUBLE_KICK] != -1);
}

bool Note::is_skipped_kick(const DrumSettings& settings) const
{
    if (!is_kick_note()) {
        return false;
    }
    if (lengths[DRUM_KICK] != -1) {
        return settings.disable_kick;
    }
    return !settings.enable_double_kick;
}

void NoteTrack::compute_base_score_ticks()
{
    constexpr int BASE_SUSTAIN_DENSITY = 25;

    auto total_ticks = 0;
    for (const auto& note : m_notes) {
        std::vector<int> constituent_lengths;
        for (auto length : note.lengths) {
            if (length != -1) {
                constituent_lengths.push_back(length);
            }
        }
        std::sort(constituent_lengths.begin(), constituent_lengths.end());
        if (constituent_lengths.front() == constituent_lengths.back()) {
            total_ticks += constituent_lengths.front();
        } else {
            for (auto length : constituent_lengths) {
                total_ticks += length;
            }
        }
    }

    m_base_score_ticks = (total_ticks * BASE_SUSTAIN_DENSITY + m_resolution - 1)
        / m_resolution;
}

void NoteTrack::merge_same_time_notes()
{
    if (m_track_type == TrackType::Drums) {
        return;
    }

    std::vector<Note> notes;
    for (auto p = m_notes.cbegin(); p < m_notes.cend();) {
        auto q = p;
        while (q < m_notes.cend() && p->position == q->position) {
            ++q;
        }
        notes.push_back(combined_note(p, q));
        p = q;
    }
    m_notes = std::move(notes);
}

NoteTrack::NoteTrack(std::vector<Note> notes,
                     const std::vector<StarPower>& sp_phrases,
                     std::vector<Solo> solos, std::vector<DrumFill> drum_fills,
                     std::vector<DiscoFlip> disco_flips,
                     std::optional<BigRockEnding> bre, TrackType track_type,
                     int resolution)
    : m_drum_fills {std::move(drum_fills)}
    , m_disco_flips {std::move(disco_flips)}
    , m_bre {bre}
    , m_track_type {track_type}
    , m_resolution {resolution}
{
    if (m_resolution <= 0) {
        throw ParseError("Resolution non-positive");
    }

    std::stable_sort(notes.begin(), notes.end(),
                     [](const auto& lhs, const auto& rhs) {
                         return lhs.position < rhs.position;
                     });

    if (!notes.empty()) {
        auto prev_note = notes.cbegin();
        for (auto p = notes.cbegin() + 1; p < notes.cend(); ++p) {
            if (p->position != prev_note->position
                || p->colours() != prev_note->colours()) {
                m_notes.push_back(*prev_note);
            }
            prev_note = p;
        }
        m_notes.push_back(*prev_note);
    }

    std::vector<int> sp_starts;
    std::vector<int> sp_ends;
    sp_starts.reserve(sp_phrases.size());
    sp_ends.reserve(sp_phrases.size());

    for (const auto& phrase : sp_phrases) {
        sp_starts.push_back(phrase.position);
        sp_ends.push_back(phrase.position + phrase.length);
    }

    std::sort(sp_starts.begin(), sp_starts.end());
    std::sort(sp_ends.begin(), sp_ends.end());

    std::vector<StarPower> new_sp_phrases;
    new_sp_phrases.reserve(sp_phrases.size());
    for (auto i = 0U; i < sp_phrases.size(); ++i) {
        auto start = sp_starts[i];
        if (i > 0) {
            start = std::max(sp_starts[i], sp_ends[i - 1]);
        }
        const auto length = sp_ends[i] - start;
        new_sp_phrases.push_back({start, length});
    }

    for (const auto& phrase : new_sp_phrases) {
        const auto first_note = std::lower_bound(
            m_notes.cbegin(), m_notes.cend(), phrase.position,
            [](const auto& lhs, const auto& rhs) {
                return lhs.position < rhs;
            });
        if ((first_note != m_notes.cend())
            && (first_note->position < (phrase.position + phrase.length))) {
            m_sp_phrases.push_back(phrase);
        }
    }

    std::stable_sort(
        solos.begin(), solos.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.start < rhs.start; });
    m_solos = std::move(solos);

    merge_same_time_notes();
    compute_base_score_ticks();

    // We handle open note merging at the end because in v23 the removed
    // notes still affect the base score.
    for (auto& note : m_notes) {
        note.merge_non_opens_into_open();
    }
}

void NoteTrack::generate_drum_fills(const TimeConverter& converter)
{
    if (m_notes.empty()) {
        return;
    }

    std::vector<std::tuple<Second, int>> note_times;
    for (const auto& n : m_notes) {
        const auto seconds = converter.beats_to_seconds(
            Beat(n.position / static_cast<double>(m_resolution)));
        note_times.push_back({seconds, n.position});
    }
    const auto final_note_s = converter.beats_to_seconds(
        Beat(m_notes.back().position / static_cast<double>(m_resolution)));
    const auto measure_bound
        = converter.seconds_to_measures(final_note_s + Second(0.25));
    Measure m {1.0};
    while (m <= measure_bound) {
        const auto fill_seconds = converter.measures_to_seconds(m);
        const auto measure_ticks = static_cast<int>(
            m_resolution * converter.measures_to_beats(m).value());
        bool exists_close_note = false;
        int close_note_position = 0;
        for (const auto& [s, pos] : note_times) {
            const auto s_diff = s - fill_seconds;
            if (s_diff > Second(0.25)) {
                break;
            }
            if (s_diff < Second(-0.25)) {
                continue;
            }
            if (!exists_close_note) {
                exists_close_note = true;
                close_note_position = pos;
            } else if (std::abs(measure_ticks - pos)
                       <= std::abs(measure_ticks - close_note_position)) {
                close_note_position = pos;
            }
        }
        if (!exists_close_note) {
            m += Measure(1.0);
            continue;
        }
        const auto m_seconds = converter.measures_to_seconds(m);
        const auto prev_m_seconds
            = converter.measures_to_seconds(m - Measure(1.0));
        const auto mid_m_seconds = (m_seconds + prev_m_seconds) * 0.5;
        const auto fill_start = static_cast<int>(
            m_resolution * converter.seconds_to_beats(mid_m_seconds).value());
        m_drum_fills.push_back(
            DrumFill {fill_start, measure_ticks - fill_start});
        m += Measure(4.0);
    }
}

void NoteTrack::disable_dynamics()
{
    for (auto& n : m_notes) {
        n.disable_dynamics();
    }
}

std::vector<Solo> NoteTrack::solos(const DrumSettings& drum_settings) const
{
    if (m_track_type != TrackType::Drums) {
        return m_solos;
    }
    auto solos = m_solos;
    auto p = m_notes.cbegin();
    auto q = solos.begin();
    while (p < m_notes.cend() && q < solos.end()) {
        if (p->position < q->start) {
            ++p;
            continue;
        }
        if (p->position > q->end) {
            ++q;
            continue;
        }
        if (p->is_skipped_kick(drum_settings)) {
            q->value -= 100;
        }
        ++p;
    }
    auto it = std::remove_if(solos.begin(), solos.end(),
                             [](auto solo) { return solo.value == 0; });
    solos.erase(it, solos.end());
    return solos;
}

int NoteTrack::base_score(DrumSettings drum_settings) const
{
    constexpr int BASE_NOTE_VALUE = 50;

    auto note_count = 0;
    for (const auto& note : m_notes) {
        if (note.is_skipped_kick(drum_settings)) {
            continue;
        }
        for (auto l : note.lengths) {
            if (l != -1) {
                ++note_count;
            }
        }
    }

    return BASE_NOTE_VALUE * note_count + m_base_score_ticks;
}

NoteTrack NoteTrack::trim_sustains() const
{
    constexpr int DEFAULT_RESOLUTION = 192;
    constexpr int DEFAULT_SUST_CUTOFF = 64;

    auto trimmed_track = *this;
    const auto sust_cutoff
        = (DEFAULT_SUST_CUTOFF * m_resolution) / DEFAULT_RESOLUTION;

    for (auto& note : trimmed_track.m_notes) {
        for (auto& length : note.lengths) {
            if (length != -1 && length <= sust_cutoff) {
                length = 0;
            }
        }
    }

    trimmed_track.compute_base_score_ticks();

    return trimmed_track;
}

NoteTrack NoteTrack::snap_chords(int snap_gap) const
{
    auto new_track = *this;
    auto& new_notes = new_track.m_notes;
    for (auto i = 1U; i < m_notes.size(); ++i) {
        if (new_notes[i].position - new_notes[i - 1].position <= snap_gap) {
            new_notes[i].position = new_notes[i - 1].position;
        }
    }
    return new_track;
}