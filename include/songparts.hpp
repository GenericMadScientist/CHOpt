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

#ifndef CHOPT_SONGPARTS_HPP
#define CHOPT_SONGPARTS_HPP

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "settings.hpp"
#include "timeconverter.hpp"

enum class NoteColour { Green, Red, Yellow, Blue, Orange, Open };

enum class GHLNoteColour {
    WhiteLow,
    WhiteMid,
    WhiteHigh,
    BlackLow,
    BlackMid,
    BlackHigh,
    Open
};

enum class DrumNoteColour {
    Red,
    Yellow,
    Blue,
    Green,
    YellowCymbal,
    BlueCymbal,
    GreenCymbal,
    RedGhost,
    YellowGhost,
    BlueGhost,
    GreenGhost,
    YellowCymbalGhost,
    BlueCymbalGhost,
    GreenCymbalGhost,
    RedAccent,
    YellowAccent,
    BlueAccent,
    GreenAccent,
    YellowCymbalAccent,
    BlueCymbalAccent,
    GreenCymbalAccent,
    Kick,
    DoubleKick
};

constexpr DrumNoteColour strip_dynamics(DrumNoteColour colour)
{
    switch (colour) {
    case DrumNoteColour::Red:
    case DrumNoteColour::RedGhost:
    case DrumNoteColour::RedAccent:
        return DrumNoteColour::Red;
    case DrumNoteColour::Yellow:
    case DrumNoteColour::YellowGhost:
    case DrumNoteColour::YellowAccent:
        return DrumNoteColour::Yellow;
    case DrumNoteColour::Blue:
    case DrumNoteColour::BlueGhost:
    case DrumNoteColour::BlueAccent:
        return DrumNoteColour::Blue;
    case DrumNoteColour::Green:
    case DrumNoteColour::GreenGhost:
    case DrumNoteColour::GreenAccent:
        return DrumNoteColour::Green;
    case DrumNoteColour::YellowCymbal:
    case DrumNoteColour::YellowCymbalGhost:
    case DrumNoteColour::YellowCymbalAccent:
        return DrumNoteColour::YellowCymbal;
    case DrumNoteColour::BlueCymbal:
    case DrumNoteColour::BlueCymbalGhost:
    case DrumNoteColour::BlueCymbalAccent:
        return DrumNoteColour::BlueCymbal;
    case DrumNoteColour::GreenCymbal:
    case DrumNoteColour::GreenCymbalGhost:
    case DrumNoteColour::GreenCymbalAccent:
        return DrumNoteColour::GreenCymbal;
    case DrumNoteColour::Kick:
        return DrumNoteColour::Kick;
    case DrumNoteColour::DoubleKick:
        return DrumNoteColour::DoubleKick;
    }

    throw std::invalid_argument("Invalid DrumNoteColour");
}

template <typename T> struct Note {
    int position {0};
    int length {0};
    T colour = {};
};

struct StarPower {
    int position;
    int length;
};

constexpr bool operator==(const StarPower& lhs, const StarPower& rhs)
{
    return std::tie(lhs.position, lhs.length)
        == std::tie(rhs.position, rhs.length);
}

struct Solo {
    int start;
    int end;
    int value;
};

struct DrumFill {
    int position;
    int length;
};

struct DiscoFlip {
    int position;
    int length;
};

struct BigRockEnding {
    int start;
    int end;
};

// Invariants:
// notes() will always return a vector of sorted notes.
// notes() will not return a vector with two notes of the same colour with the
// same position.
// resolution() will be greater than zero.
// sp_phrases() will always return a vector of sorted SP phrases.
// sp_phrases() will only return phrases with a note in their range.
// sp_phrases() will return non-overlapping phrases.
// solos() will always return a vector of sorted solos.
template <typename T> class NoteTrack {
private:
    std::vector<Note<T>> m_notes;
    std::vector<StarPower> m_sp_phrases;
    std::vector<Solo> m_solos;
    std::vector<DrumFill> m_drum_fills;
    std::vector<DiscoFlip> m_disco_flips;
    std::optional<BigRockEnding> m_bre;
    int m_resolution;
    int m_base_score;

    int compute_base_score()
    {
        constexpr auto BASE_NOTE_VALUE = 50;
        constexpr auto BASE_SUSTAIN_DENSITY = 25;

        auto base_score = static_cast<int>(BASE_NOTE_VALUE * m_notes.size());
        auto total_ticks = 0;
        auto current_pos = -1;
        auto first_note_length = 0;
        auto current_total_ticks = 0;
        auto is_disjoint_chord = false;
        for (const auto& note : m_notes) {
            if (note.position != current_pos) {
                if (is_disjoint_chord) {
                    total_ticks += current_total_ticks;
                } else {
                    total_ticks += first_note_length;
                }
                first_note_length = note.length;
                current_total_ticks = note.length;
                current_pos = note.position;
                is_disjoint_chord = false;
            } else {
                if (note.length != first_note_length) {
                    is_disjoint_chord = true;
                }
                current_total_ticks += note.length;
            }
        }
        if (is_disjoint_chord) {
            total_ticks += current_total_ticks;
        } else {
            total_ticks += first_note_length;
        }

        base_score += (total_ticks * BASE_SUSTAIN_DENSITY + m_resolution - 1)
            / m_resolution;
        return base_score;
    }

public:
    NoteTrack(std::vector<Note<T>> notes,
              const std::vector<StarPower>& sp_phrases, std::vector<Solo> solos,
              std::vector<DrumFill> drum_fills,
              std::vector<DiscoFlip> disco_flips,
              std::optional<BigRockEnding> bre, int resolution)
        : m_drum_fills {std::move(drum_fills)}
        , m_disco_flips {std::move(disco_flips)}
        , m_bre {bre}
        , m_resolution {resolution}
    {
        if (m_resolution <= 0) {
            throw ParseError("Resolution non-positive");
        }

        std::stable_sort(notes.begin(), notes.end(),
                         [](const auto& lhs, const auto& rhs) {
                             return std::tie(lhs.position, lhs.colour)
                                 < std::tie(rhs.position, rhs.colour);
                         });

        if (!notes.empty()) {
            auto prev_note = notes.cbegin();
            for (auto p = notes.cbegin() + 1; p < notes.cend(); ++p) {
                if (p->position != prev_note->position
                    || p->colour != prev_note->colour) {
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

        std::stable_sort(solos.begin(), solos.end(),
                         [](const auto& lhs, const auto& rhs) {
                             return lhs.start < rhs.start;
                         });
        m_solos = std::move(solos);

        m_base_score = compute_base_score();

        // We handle open note merging at the end because in v23 the removed
        // notes still affect the base score.
        std::vector<Note<T>> merged_notes;
        if constexpr (std::is_same_v<T, NoteColour>) {
            for (auto p = m_notes.cbegin(); p < m_notes.cend();) {
                auto q = std::next(p);
                while (q < m_notes.cend() && p->position == q->position) {
                    q = std::next(q);
                }
                for (auto r = p; r < q; ++r) {
                    if (r->colour == NoteColour::Open) {
                        merged_notes.push_back(*r);
                        continue;
                    }
                    bool discarded = false;
                    for (auto s = p; s < q; ++s) {
                        if (s == r) {
                            continue;
                        }
                        if (s->colour != NoteColour::Open) {
                            continue;
                        }
                        if (s->length == r->length) {
                            discarded = true;
                            break;
                        }
                    }
                    if (!discarded) {
                        merged_notes.push_back(*r);
                    }
                }
                p = q;
            }
        } else if constexpr (std::is_same_v<T, GHLNoteColour>) {
            for (auto p = m_notes.cbegin(); p < m_notes.cend();) {
                auto q = std::next(p);
                while (q < m_notes.cend() && p->position == q->position) {
                    q = std::next(q);
                }
                for (auto r = p; r < q; ++r) {
                    if (r->colour == GHLNoteColour::Open) {
                        merged_notes.push_back(*r);
                        continue;
                    }
                    bool discarded = false;
                    for (auto s = p; s < q; ++s) {
                        if (s == r) {
                            continue;
                        }
                        if (s->colour != GHLNoteColour::Open) {
                            continue;
                        }
                        if (s->length == r->length) {
                            discarded = true;
                            break;
                        }
                    }
                    if (!discarded) {
                        merged_notes.push_back(*r);
                    }
                }
                p = q;
            }
        } else {
            merged_notes = m_notes;
        }
        m_notes = std::move(merged_notes);
    }

    void generate_drum_fills(const TimeConverter& converter)
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
                m_resolution
                * converter.seconds_to_beats(mid_m_seconds).value());
            m_drum_fills.push_back(
                DrumFill {fill_start, measure_ticks - fill_start});
            m += Measure(4.0);
        }
    }

    void disable_dynamics()
    {
        if constexpr (std::is_same_v<T, DrumNoteColour>) {
            for (auto& n : m_notes) {
                n.colour = strip_dynamics(n.colour);
            }
        }
    }

    [[nodiscard]] const std::vector<Note<T>>& notes() const { return m_notes; }
    [[nodiscard]] const std::vector<StarPower>& sp_phrases() const
    {
        return m_sp_phrases;
    }
    [[nodiscard]] std::vector<Solo>
    solos(const DrumSettings& drum_settings) const
    {
        if constexpr (std::is_same_v<T, DrumNoteColour>) {
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
                if (p->colour == DrumNoteColour::DoubleKick
                    && !drum_settings.enable_double_kick) {
                    q->value -= 100;
                } else if (p->colour == DrumNoteColour::Kick
                           && drum_settings.disable_kick) {
                    q->value -= 100;
                }
                ++p;
            }
            auto it = std::remove_if(solos.begin(), solos.end(),
                                     [](auto solo) { return solo.value == 0; });
            solos.erase(it, solos.end());
            return solos;
        } else {
            return m_solos;
        }
    }
    [[nodiscard]] const std::vector<DrumFill>& drum_fills() const
    {
        return m_drum_fills;
    }
    [[nodiscard]] const std::vector<DiscoFlip>& disco_flips() const
    {
        return m_disco_flips;
    }
    [[nodiscard]] std::optional<BigRockEnding> bre() const { return m_bre; }
    [[nodiscard]] int resolution() const { return m_resolution; }
    [[nodiscard]] int base_score() const { return m_base_score; }
    [[nodiscard]] NoteTrack<T> trim_sustains(int speed) const
    {
        constexpr int DEFAULT_RESOLUTION = 192;
        constexpr int DEFAULT_SPEED = 100;
        constexpr int DEFAULT_SUST_CUTOFF = 64;

        auto trimmed_track = *this;
        auto sust_cutoff = (DEFAULT_SUST_CUTOFF * m_resolution * speed)
            / (DEFAULT_SPEED * DEFAULT_RESOLUTION);

        for (auto& note : trimmed_track.m_notes) {
            if (note.length <= sust_cutoff) {
                note.length = 0;
            }
        }

        trimmed_track.m_base_score = trimmed_track.compute_base_score();

        // We need to do this because for speeds below 100%, the sustains are
        // trimmed the same as 100% speed but the base score can be higher.
        if (speed < DEFAULT_SPEED) {
            sust_cutoff
                = (DEFAULT_SUST_CUTOFF * m_resolution) / DEFAULT_RESOLUTION;

            for (auto& note : trimmed_track.m_notes) {
                if (note.length <= sust_cutoff) {
                    note.length = 0;
                }
            }
        }

        return trimmed_track;
    }
    [[nodiscard]] NoteTrack<T> snap_chords(int snap_gap) const
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
};

#endif
