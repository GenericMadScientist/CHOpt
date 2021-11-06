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

#include "catch.hpp"

#include "songparts.hpp"

template <typename T>
static bool operator==(const Note<T>& lhs, const Note<T>& rhs)
{
    return std::tie(lhs.position, lhs.length, lhs.colour)
        == std::tie(rhs.position, rhs.length, rhs.colour);
}

static bool operator==(const Solo& lhs, const Solo& rhs)
{
    return std::tie(lhs.start, lhs.end, lhs.value)
        == std::tie(rhs.start, rhs.end, rhs.value);
}

static bool operator==(const DrumFill& lhs, const DrumFill& rhs)
{
    return std::tie(lhs.position, lhs.length)
        == std::tie(rhs.position, rhs.length);
}

TEST_CASE("NoteTrack ctor maintains invariants")
{
    SECTION("Notes are sorted")
    {
        std::vector<Note<NoteColour>> notes {{768}, {384}};
        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
        std::vector<Note<NoteColour>> sorted_notes {{384}, {768}};

        REQUIRE(track.notes() == sorted_notes);
    }

    SECTION("Notes of the same colour and position are merged")
    {
        std::vector<Note<NoteColour>> notes {{768, 0}, {768, 768}};
        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
        std::vector<Note<NoteColour>> required_notes {{768, 768}};

        REQUIRE(track.notes() == required_notes);

        std::vector<Note<NoteColour>> second_notes {{768, 768}, {768, 0}};
        NoteTrack<NoteColour> second_track {second_notes, {}, {}, {},
                                            {},           {}, 192};
        std::vector<Note<NoteColour>> second_required_notes {{768, 0}};

        REQUIRE(second_track.notes() == second_required_notes);
    }

    SECTION("Notes of different colours are dealt with separately")
    {
        std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green},
                                             {768, 0, NoteColour::Red},
                                             {768, 768, NoteColour::Green}};
        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
        std::vector<Note<NoteColour>> required_notes {
            {768, 768, NoteColour::Green}, {768, 0, NoteColour::Red}};

        REQUIRE(track.notes() == required_notes);
    }

    SECTION("Open notes and non-open notes of same pos and length are merged")
    {
        std::vector<Note<NoteColour>> notes {{768, 0, NoteColour::Green},
                                             {768, 1, NoteColour::Red},
                                             {768, 0, NoteColour::Open}};
        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};
        std::vector<Note<NoteColour>> required_notes {
            {768, 1, NoteColour::Red}, {768, 0, NoteColour::Open}};

        REQUIRE(track.notes() == required_notes);
    }

    SECTION("Resolution is positive")
    {
        std::vector<Note<NoteColour>> notes {{768}};
        REQUIRE_THROWS_AS(
            [&] {
                return NoteTrack<NoteColour>(notes, {}, {}, {}, {}, {}, 0);
            }(),
            ParseError);
    }

    SECTION("Empty SP phrases are culled")
    {
        std::vector<Note<NoteColour>> notes {{768}};
        std::vector<StarPower> phrases {{0, 100}, {700, 100}, {1000, 100}};
        NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
        std::vector<StarPower> required_phrases {{700, 100}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases are sorted")
    {
        std::vector<Note<NoteColour>> notes {{768}, {1000}};
        std::vector<StarPower> phrases {{1000, 1}, {768, 1}};
        NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
        std::vector<StarPower> required_phrases {{768, 1}, {1000, 1}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("SP phrases do not overlap")
    {
        std::vector<Note<NoteColour>> notes {{768}, {1000}, {1500}};
        std::vector<StarPower> phrases {{768, 1000}, {900, 150}};
        NoteTrack<NoteColour> track {notes, phrases, {}, {}, {}, {}, 192};
        std::vector<StarPower> required_phrases {{768, 282}, {1050, 718}};

        REQUIRE(track.sp_phrases() == required_phrases);
    }

    SECTION("Solos are sorted")
    {
        std::vector<Note<NoteColour>> notes {{0}, {768}};
        std::vector<Solo> solos {{768, 868, 100}, {0, 100, 100}};
        NoteTrack<NoteColour> track {notes, {}, solos, {}, {}, {}, 192};
        std::vector<Solo> required_solos {{0, 100, 100}, {768, 868, 100}};

        REQUIRE(track.solos(DrumSettings::default_settings())
                == required_solos);
    }
}

TEST_CASE("Solos do take into account drum settings")
{
    std::vector<Note<DrumNoteColour>> notes {
        {0, 0, DrumNoteColour::Red},
        {0, 0, DrumNoteColour::DoubleKick},
        {192, 0, DrumNoteColour::DoubleKick}};
    std::vector<Solo> solos {{0, 1, 200}, {192, 193, 100}};
    NoteTrack<DrumNoteColour> track {notes, {}, solos, {}, {}, {}, 192};
    std::vector<Solo> required_solos {{0, 1, 100}};

    REQUIRE(track.solos({false, false, true}) == required_solos);
}

TEST_CASE("Automatic drum activation zone generation is correct")
{
    SECTION("Automatic zones are created")
    {
        std::vector<Note<DrumNoteColour>> notes {
            {768}, {1536}, {2304}, {3072}, {3840}};
        TimeConverter converter {{}, 192, ChDrumEngine(), {}};

        NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
        std::vector<DrumFill> fills {{384, 384}, {3456, 384}};

        track.generate_drum_fills(converter);

        REQUIRE(track.drum_fills() == fills);
    }

    SECTION("Automatic zones have 250ms of leniency")
    {
        std::vector<Note<DrumNoteColour>> notes {
            {672}, {3936}, {6815}, {10081}};
        TimeConverter converter {{}, 192, ChDrumEngine(), {}};

        NoteTrack<DrumNoteColour> track {notes, {}, {}, {}, {}, {}, 192};
        std::vector<DrumFill> fills {{384, 288}, {3456, 480}};

        track.generate_drum_fills(converter);

        REQUIRE(track.drum_fills() == fills);
    }
}

TEST_CASE("Base score for average multiplier is correct")
{
    SECTION("Base score is correct for songs without sustains")
    {
        std::vector<Note<NoteColour>> notes {
            {192}, {384}, {384, 0, NoteColour::Red}};

        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

        REQUIRE(track.base_score() == 150);
    }

    SECTION("Base score is correct for songs with sustains")
    {
        std::vector<Note<NoteColour>> notes_one {{192, 192}};
        std::vector<Note<NoteColour>> notes_two {{192, 92}};
        std::vector<Note<NoteColour>> notes_three {{192, 93}};

        NoteTrack<NoteColour> track_one {notes_one, {}, {}, {}, {}, {}, 192};
        NoteTrack<NoteColour> track_two {notes_two, {}, {}, {}, {}, {}, 192};
        NoteTrack<NoteColour> track_three {notes_three, {}, {}, {},
                                           {},          {}, 192};

        REQUIRE(track_one.base_score() == 75);
        REQUIRE(track_two.base_score() == 62);
        REQUIRE(track_three.base_score() == 63);
    }

    SECTION("Base score is correct for songs with chord sustains")
    {
        std::vector<Note<NoteColour>> notes {{192, 192},
                                             {192, 192, NoteColour::Red}};

        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

        REQUIRE(track.base_score() == 125);
    }

    SECTION("Base score is correct for other resolutions")
    {
        std::vector<Note<NoteColour>> notes {{192, 192}};

        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 480};

        REQUIRE(track.base_score() == 60);
    }

    SECTION("Fractional ticks from multiple holds are added together correctly")
    {
        std::vector<Note<NoteColour>> notes {{0, 100}, {192, 100}};

        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

        REQUIRE(track.base_score() == 127);
    }

    SECTION("Disjoint chords are handled correctly")
    {
        std::vector<Note<NoteColour>> notes {
            {0, 384}, {0, 384, NoteColour::Red}, {0, 192, NoteColour::Yellow}};

        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

        REQUIRE(track.base_score() == 275);
    }

    SECTION("Base score is correctly handled with open note merging")
    {
        std::vector<Note<NoteColour>> notes {{0, 0}, {0, 0, NoteColour::Open}};

        NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 192};

        REQUIRE(track.base_score() == 100);
    }
}

TEST_CASE("trim_sustains is correct")
{
    const std::vector<Note<NoteColour>> notes {{0, 65}, {200, 70}, {400, 140}};
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 200};

    SECTION("100% speed")
    {
        auto new_track = track.trim_sustains(100);
        const auto& new_notes = new_track.notes();

        REQUIRE(new_notes[0].length == 0);
        REQUIRE(new_notes[1].length == 70);
        REQUIRE(new_notes[2].length == 140);
        REQUIRE(new_track.base_score() == 177);
    }

    SECTION("50% speed")
    {
        auto new_track = track.trim_sustains(50);
        const auto& new_notes = new_track.notes();

        REQUIRE(new_notes[0].length == 0);
        REQUIRE(new_notes[1].length == 70);
        REQUIRE(new_notes[2].length == 140);
        REQUIRE(new_track.base_score() == 185);
    }

    SECTION("200% speed")
    {
        auto new_track = track.trim_sustains(200);
        const auto& new_notes = new_track.notes();

        REQUIRE(new_notes[0].length == 0);
        REQUIRE(new_notes[1].length == 0);
        REQUIRE(new_notes[2].length == 140);
        REQUIRE(new_track.base_score() == 168);
    }
}

TEST_CASE("snap_chords is correct")
{
    const std::vector<Note<NoteColour>> notes {{0, 0, NoteColour::Green},
                                               {5, 0, NoteColour::Red}};
    const NoteTrack<NoteColour> track {notes, {}, {}, {}, {}, {}, 480};

    SECTION("No snapping")
    {
        auto new_track = track.snap_chords(0);
        const auto& new_notes = new_track.notes();

        REQUIRE(new_notes[0].position == 0);
        REQUIRE(new_notes[1].position == 5);
    }

    SECTION("HMX GH snapping")
    {
        auto new_track = track.snap_chords(10);
        const auto& new_notes = new_track.notes();

        REQUIRE(new_notes[0].position == 0);
        REQUIRE(new_notes[1].position == 0);
    }
}
