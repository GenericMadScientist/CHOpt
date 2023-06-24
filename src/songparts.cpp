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
        for (auto i = 0U; i < note.lengths.size(); ++i) {
            const auto new_length = begin->lengths.at(i);
            if (new_length != SightRead::Tick {-1}) {
                note.lengths.at(i) = new_length;
            }
        }
        ++begin;
    }
    return note;
}

bool is_chord(const Note& note)
{
    auto count = 0;
    for (auto length : note.lengths) {
        if (length != SightRead::Tick {-1}) {
            ++count;
        }
    }
    return count >= 2;
}
}

std::set<Instrument> all_instruments()
{
    return {Instrument::Guitar,        Instrument::GuitarCoop,
            Instrument::Bass,          Instrument::Rhythm,
            Instrument::Keys,          Instrument::GHLGuitar,
            Instrument::GHLBass,       Instrument::GHLRhythm,
            Instrument::GHLGuitarCoop, Instrument::Drums};
}

int Note::open_index() const
{
    if ((flags & FLAGS_FIVE_FRET_GUITAR) != 0U) {
        return FIVE_FRET_OPEN;
    }
    if ((flags & FLAGS_SIX_FRET_GUITAR) != 0U) {
        return SIX_FRET_OPEN;
    }
    return -1;
}

int Note::colours() const
{
    auto colour_flags = 0;
    for (auto i = 0U; i < lengths.size(); ++i) {
        if (lengths.at(i) != SightRead::Tick {-1}) {
            colour_flags |= 1 << i;
        }
    }
    return colour_flags;
}

void Note::merge_non_opens_into_open()
{
    const auto index = open_index();
    if (index == -1) {
        return;
    }
    const auto open_length = lengths.at(index);
    if (open_length == SightRead::Tick {-1}) {
        return;
    }
    for (auto i = 0; i < static_cast<int>(lengths.size()); ++i) {
        if (i != index && lengths.at(i) == open_length) {
            lengths.at(i) = SightRead::Tick {-1};
        }
    }
}

void Note::disable_dynamics()
{
    flags = static_cast<NoteFlags>(flags & ~(FLAGS_GHOST | FLAGS_ACCENT));
}

bool Note::is_kick_note() const
{
    return ((flags & FLAGS_DRUMS) != 0U)
        && (lengths[DRUM_KICK] != SightRead::Tick {-1}
            || lengths[DRUM_DOUBLE_KICK] != SightRead::Tick {-1});
}

bool Note::is_skipped_kick(const SightRead::DrumSettings& settings) const
{
    if (!is_kick_note()) {
        return false;
    }
    if (lengths[DRUM_KICK] != SightRead::Tick {-1}) {
        return settings.disable_kick;
    }
    return !settings.enable_double_kick;
}

void NoteTrack::compute_base_score_ticks()
{
    constexpr int BASE_SUSTAIN_DENSITY = 25;

    SightRead::Tick total_ticks {0};
    for (const auto& note : m_notes) {
        std::vector<SightRead::Tick> constituent_lengths;
        for (auto length : note.lengths) {
            if (length != SightRead::Tick {-1}) {
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

    const auto resolution = m_global_data->resolution();
    m_base_score_ticks
        = (total_ticks.value() * BASE_SUSTAIN_DENSITY + resolution - 1)
        / resolution;
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

void NoteTrack::add_hopos(SightRead::Tick max_hopo_gap)
{
    if (m_track_type == TrackType::Drums) {
        return;
    }

    for (auto i = 0U; i < m_notes.size(); ++i) {
        if ((m_notes[i].flags & (FLAGS_TAP | FLAGS_FORCE_STRUM)) != 0U) {
            continue;
        }
        bool is_hopo = (m_notes[i].flags & FLAGS_FORCE_FLIP) != 0U;
        if (i != 0U) {
            const auto note_gap = m_notes[i].position - m_notes[i - 1].position;
            if (!is_chord(m_notes[i])
                && m_notes[i].colours() != m_notes[i - 1].colours()
                && note_gap <= max_hopo_gap) {
                is_hopo = !is_hopo;
            }
        }
        if ((m_notes[i].flags & FLAGS_FORCE_HOPO) != 0U) {
            is_hopo = true;
        }
        if (is_hopo) {
            m_notes[i].flags
                = static_cast<NoteFlags>(m_notes[i].flags | FLAGS_HOPO);
        }
    }
}

NoteTrack::NoteTrack(std::vector<Note> notes,
                     const std::vector<StarPower>& sp_phrases,
                     TrackType track_type,
                     std::shared_ptr<SongGlobalData> global_data,
                     SightRead::Tick max_hopo_gap)
    : m_track_type {track_type}
    , m_global_data {std::move(global_data)}
    , m_base_score_ticks {0}
{
    if (m_global_data == nullptr) {
        throw std::runtime_error("Non-null global data required");
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

    std::vector<SightRead::Tick> sp_starts;
    std::vector<SightRead::Tick> sp_ends;
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

    merge_same_time_notes();
    compute_base_score_ticks();

    // We handle open note merging at the end because in v23 the removed
    // notes still affect the base score.
    for (auto& note : m_notes) {
        note.merge_non_opens_into_open();
    }

    add_hopos(max_hopo_gap);
}

void NoteTrack::generate_drum_fills(const SightRead::TempoMap& tempo_map)
{
    const SightRead::Second FILL_DELAY {0.25};
    const SightRead::Measure FILL_GAP {4.0};

    if (m_notes.empty()) {
        return;
    }

    std::vector<std::tuple<SightRead::Second, SightRead::Tick>> note_times;
    for (const auto& n : m_notes) {
        const auto seconds = tempo_map.to_seconds(n.position);
        note_times.emplace_back(seconds, n.position);
    }
    const auto final_note_s = tempo_map.to_seconds(m_notes.back().position);
    const auto measure_bound = tempo_map.to_measures(final_note_s + FILL_DELAY);
    SightRead::Measure m {1.0};
    while (m <= measure_bound) {
        const auto fill_seconds = tempo_map.to_seconds(m);
        const auto measure_ticks = tempo_map.to_ticks(tempo_map.to_beats(m));
        bool exists_close_note = false;
        SightRead::Tick close_note_position {0};
        for (const auto& [s, pos] : note_times) {
            const auto s_diff = s - fill_seconds;
            if (s_diff > FILL_DELAY) {
                break;
            }
            if (s_diff + FILL_DELAY < SightRead::Second {0}) {
                continue;
            }
            if (!exists_close_note) {
                exists_close_note = true;
                close_note_position = pos;
            } else if (std::abs((measure_ticks - pos).value()) <= std::abs(
                           (measure_ticks - close_note_position).value())) {
                close_note_position = pos;
            }
        }
        if (!exists_close_note) {
            m += SightRead::Measure(1.0);
            continue;
        }
        const auto m_seconds = tempo_map.to_seconds(m);
        const auto prev_m_seconds
            = tempo_map.to_seconds(m - SightRead::Measure(1.0));
        const auto mid_m_seconds = (m_seconds + prev_m_seconds) * 0.5;
        const auto fill_start = tempo_map.to_ticks(mid_m_seconds);
        m_drum_fills.push_back(
            DrumFill {fill_start, measure_ticks - fill_start});
        m += FILL_GAP;
    }
}

void NoteTrack::disable_dynamics()
{
    for (auto& n : m_notes) {
        n.disable_dynamics();
    }
}

std::vector<Solo>
NoteTrack::solos(const SightRead::DrumSettings& drum_settings) const
{
    constexpr int SOLO_NOTE_VALUE = 100;

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
            q->value -= SOLO_NOTE_VALUE;
        }
        ++p;
    }
    std::erase_if(solos, [](const auto& solo) { return solo.value == 0; });
    return solos;
}

void NoteTrack::solos(std::vector<Solo> solos)
{
    std::stable_sort(
        solos.begin(), solos.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.start < rhs.start; });
    m_solos = std::move(solos);
}

int NoteTrack::base_score(SightRead::DrumSettings drum_settings) const
{
    constexpr int BASE_NOTE_VALUE = 50;

    auto note_count = 0;
    for (const auto& note : m_notes) {
        if (note.is_skipped_kick(drum_settings)) {
            continue;
        }
        for (auto l : note.lengths) {
            if (l != SightRead::Tick {-1}) {
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
    const auto resolution = m_global_data->resolution();
    const SightRead::Tick sust_cutoff {(DEFAULT_SUST_CUTOFF * resolution)
                                       / DEFAULT_RESOLUTION};

    for (auto& note : trimmed_track.m_notes) {
        for (auto& length : note.lengths) {
            if (length != SightRead::Tick {-1} && length <= sust_cutoff) {
                length = SightRead::Tick {0};
            }
        }
    }

    trimmed_track.compute_base_score_ticks();

    return trimmed_track;
}

NoteTrack NoteTrack::snap_chords(SightRead::Tick snap_gap) const
{
    auto new_track = *this;
    auto& new_notes = new_track.m_notes;
    for (auto i = 1U; i < m_notes.size(); ++i) {
        if (new_notes[i].position - new_notes[i - 1].position <= snap_gap) {
            new_notes[i].position = new_notes[i - 1].position;
        }
    }
    new_track.merge_same_time_notes();
    return new_track;
}