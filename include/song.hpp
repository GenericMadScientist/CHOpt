#ifndef SIGHTREAD_SONG_HPP
#define SIGHTREAD_SONG_HPP

#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <sightread/songparts.hpp>
#include <sightread/time.hpp>

namespace SightRead {
class Song {
private:
    std::shared_ptr<SightRead::SongGlobalData> m_global_data
        = std::make_shared<SightRead::SongGlobalData>();
    std::map<std::tuple<SightRead::Instrument, SightRead::Difficulty>,
             SightRead::NoteTrack>
        m_tracks;

public:
    Song() = default;
    void add_note_track(SightRead::Instrument instrument,
                        SightRead::Difficulty difficulty,
                        SightRead::NoteTrack note_track);
    [[nodiscard]] SightRead::SongGlobalData& global_data()
    {
        return *m_global_data;
    }
    [[nodiscard]] const SightRead::SongGlobalData& global_data() const
    {
        return *m_global_data;
    }
    [[nodiscard]] const std::shared_ptr<SightRead::SongGlobalData>&
    global_data_ptr()
    {
        return m_global_data;
    }
    [[nodiscard]] std::vector<SightRead::Instrument> instruments() const;
    [[nodiscard]] std::vector<SightRead::Difficulty>
    difficulties(SightRead::Instrument instrument) const;
    [[nodiscard]] const SightRead::NoteTrack&
    track(SightRead::Instrument instrument,
          SightRead::Difficulty difficulty) const;
    [[nodiscard]] std::vector<SightRead::Tick> unison_phrase_positions() const;
    void speedup(int speed);
};
}

#endif
