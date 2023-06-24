#ifndef SIGHTREAD_SONGPARTS_HPP
#define SIGHTREAD_SONGPARTS_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <sightread/drumsettings.hpp>
#include <sightread/tempomap.hpp>
#include <sightread/time.hpp>

namespace SightRead {
enum class Difficulty { Easy = 0, Medium = 1, Hard = 2, Expert = 3 };

enum class Instrument {
    Guitar,
    GuitarCoop,
    Bass,
    Rhythm,
    Keys,
    GHLGuitar,
    GHLBass,
    GHLRhythm,
    GHLGuitarCoop,
    Drums
};

std::set<Instrument> all_instruments();

enum class TrackType { FiveFret, SixFret, Drums };

enum NoteFlags : std::uint32_t {
    FLAGS_NONE = 0,
    FLAGS_CYMBAL = 1U << 0,
    FLAGS_GHOST = 1U << 1,
    FLAGS_ACCENT = 1U << 2,
    FLAGS_HOPO = 1U << 3,
    FLAGS_TAP = 1U << 4,
    FLAGS_FORCE_FLIP = 1U << 5,
    FLAGS_FORCE_HOPO = 1U << 6,
    FLAGS_FORCE_STRUM = 1U << 7,
    FLAGS_DRUMS = 1U << 29,
    FLAGS_SIX_FRET_GUITAR = 1U << 30,
    FLAGS_FIVE_FRET_GUITAR = 1U << 31
};

enum FiveFretNotes {
    FIVE_FRET_GREEN = 0,
    FIVE_FRET_RED = 1,
    FIVE_FRET_YELLOW = 2,
    FIVE_FRET_BLUE = 3,
    FIVE_FRET_ORANGE = 4,
    FIVE_FRET_OPEN = 5
};

enum SixFretNotes {
    SIX_FRET_WHITE_LOW = 0,
    SIX_FRET_WHITE_MID = 1,
    SIX_FRET_WHITE_HIGH = 2,
    SIX_FRET_BLACK_LOW = 3,
    SIX_FRET_BLACK_MID = 4,
    SIX_FRET_BLACK_HIGH = 5,
    SIX_FRET_OPEN = 6
};

enum DrumNotes {
    DRUM_RED = 0,
    DRUM_YELLOW = 1,
    DRUM_BLUE = 2,
    DRUM_GREEN = 3,
    DRUM_KICK = 4,
    DRUM_DOUBLE_KICK = 5
};

struct Note {
private:
    int open_index() const;

public:
    SightRead::Tick position {0};
    std::array<SightRead::Tick, 7> lengths {
        {SightRead::Tick {-1}, SightRead::Tick {-1}, SightRead::Tick {-1},
         SightRead::Tick {-1}, SightRead::Tick {-1}, SightRead::Tick {-1},
         SightRead::Tick {-1}}};
    NoteFlags flags {0};

    [[nodiscard]] int colours() const;
    void merge_non_opens_into_open();
    void disable_dynamics();
    [[nodiscard]] bool is_kick_note() const;
    [[nodiscard]] bool
    is_skipped_kick(const SightRead::DrumSettings& settings) const;
};

struct StarPower {
    SightRead::Tick position;
    SightRead::Tick length;
};

struct Solo {
    SightRead::Tick start;
    SightRead::Tick end;
    int value;
};

struct DrumFill {
    SightRead::Tick position;
    SightRead::Tick length;
};

struct DiscoFlip {
    SightRead::Tick position;
    SightRead::Tick length;
};

struct BigRockEnding {
    SightRead::Tick start;
    SightRead::Tick end;
};

// Invariants:
// resolution() > 0.
class SongGlobalData {
private:
    static constexpr int DEFAULT_RESOLUTION = 192;

    bool m_is_from_midi = false;
    int m_resolution = DEFAULT_RESOLUTION;
    std::string m_name;
    std::string m_artist;
    std::string m_charter;
    SightRead::TempoMap m_tempo_map;
    std::vector<SightRead::Tick> m_od_beats;

public:
    SongGlobalData() = default;

    [[nodiscard]] bool is_from_midi() const { return m_is_from_midi; }
    [[nodiscard]] int resolution() const { return m_resolution; }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::string& artist() const { return m_artist; }
    [[nodiscard]] const std::string& charter() const { return m_charter; }
    [[nodiscard]] SightRead::TempoMap& tempo_map() { return m_tempo_map; }
    [[nodiscard]] const SightRead::TempoMap& tempo_map() const
    {
        return m_tempo_map;
    }
    [[nodiscard]] const std::vector<SightRead::Tick>& od_beats() const
    {
        return m_od_beats;
    }

    void is_from_midi(bool value) { m_is_from_midi = value; }
    void resolution(int value)
    {
        if (value <= 0) {
            throw SightRead::ParseError("Resolution non-positive");
        }
        m_resolution = value;
    }
    void name(std::string value) { m_name = std::move(value); }
    void artist(std::string value) { m_artist = std::move(value); }
    void charter(std::string value) { m_charter = std::move(value); }
    void tempo_map(SightRead::TempoMap value)
    {
        m_tempo_map = std::move(value);
    }
    void od_beats(std::vector<SightRead::Tick> value)
    {
        m_od_beats = std::move(value);
    }
};

class NoteTrack {
private:
    std::vector<Note> m_notes;
    std::vector<StarPower> m_sp_phrases;
    std::vector<Solo> m_solos;
    std::vector<DrumFill> m_drum_fills;
    std::vector<DiscoFlip> m_disco_flips;
    std::optional<BigRockEnding> m_bre;
    TrackType m_track_type;
    std::shared_ptr<SongGlobalData> m_global_data;
    int m_base_score_ticks;

    void compute_base_score_ticks();
    void merge_same_time_notes();
    void add_hopos(SightRead::Tick max_hopo_gap);

public:
    NoteTrack(std::vector<Note> notes, const std::vector<StarPower>& sp_phrases,
              TrackType track_type, std::shared_ptr<SongGlobalData> global_data,
              SightRead::Tick max_hopo_gap = SightRead::Tick {65});
    void generate_drum_fills(const SightRead::TempoMap& tempo_map);
    void disable_dynamics();
    [[nodiscard]] const std::vector<Note>& notes() const { return m_notes; }
    [[nodiscard]] const std::vector<StarPower>& sp_phrases() const
    {
        return m_sp_phrases;
    }

    [[nodiscard]] std::vector<Solo>
    solos(const SightRead::DrumSettings& drum_settings) const;
    void solos(std::vector<Solo> solos);

    [[nodiscard]] const std::vector<DrumFill>& drum_fills() const
    {
        return m_drum_fills;
    }
    void drum_fills(std::vector<DrumFill> drum_fills)
    {
        m_drum_fills = std::move(drum_fills);
    }

    [[nodiscard]] const std::vector<DiscoFlip>& disco_flips() const
    {
        return m_disco_flips;
    }
    void disco_flips(std::vector<DiscoFlip> disco_flips)
    {
        m_disco_flips = std::move(disco_flips);
    }

    [[nodiscard]] std::optional<BigRockEnding> bre() const { return m_bre; }
    void bre(std::optional<BigRockEnding> bre) { m_bre = std::move(bre); }

    [[nodiscard]] TrackType track_type() const { return m_track_type; }
    [[nodiscard]] const SongGlobalData& global_data() const
    {
        return *m_global_data;
    }
    [[nodiscard]] int
    base_score(SightRead::DrumSettings drum_settings
               = SightRead::DrumSettings::default_settings()) const;
    [[nodiscard]] NoteTrack trim_sustains() const;
    [[nodiscard]] NoteTrack snap_chords(SightRead::Tick snap_gap) const;
};
}

#endif
