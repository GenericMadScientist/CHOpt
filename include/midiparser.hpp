#ifndef SIGHTREAD_MIDIPARSER_HPP
#define SIGHTREAD_MIDIPARSER_HPP

#include <cstdint>
#include <set>
#include <span>
#include <string>

#include <sightread/detail/midi.hpp>
#include <sightread/hopothreshold.hpp>
#include <sightread/metadata.hpp>
#include <sightread/song.hpp>
#include <sightread/songparts.hpp>

namespace SightRead {
class MidiParser {
private:
    std::string m_song_name;
    std::string m_artist;
    std::string m_charter;
    SightRead::HopoThreshold m_hopo_threshold;
    std::set<SightRead::Instrument> m_permitted_instruments;
    bool m_permit_solos;

public:
    explicit MidiParser(const SightRead::Metadata& metadata);
    MidiParser& hopo_threshold(SightRead::HopoThreshold hopo_threshold);
    MidiParser&
    permit_instruments(std::set<SightRead::Instrument> permitted_instruments);
    MidiParser& parse_solos(bool permit_solos);
    SightRead::Song from_midi(const SightRead::Detail::Midi& midi) const;
    SightRead::Song parse(std::span<const std::uint8_t> data) const;
};
}

#endif
