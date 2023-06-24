#include <algorithm>
#include <array>
#include <set>

#include "parserutil.hpp"

bool SightRead::Detail::is_six_fret_instrument(SightRead::Instrument instrument)
{
    constexpr std::array SIX_FRET_INSTRUMENTS {
        SightRead::Instrument::GHLGuitar, SightRead::Instrument::GHLBass,
        SightRead::Instrument::GHLRhythm, SightRead::Instrument::GHLGuitarCoop};
    return std::find(SIX_FRET_INSTRUMENTS.cbegin(), SIX_FRET_INSTRUMENTS.cend(),
                     instrument)
        != SIX_FRET_INSTRUMENTS.cend();
}

std::vector<std::tuple<SightRead::Tick, SightRead::Tick>>
SightRead::Detail::combine_solo_events(const std::vector<int>& on_events,
                                       const std::vector<int>& off_events)
{
    std::vector<std::tuple<SightRead::Tick, SightRead::Tick>> ranges;

    auto on_iter = on_events.cbegin();
    auto off_iter = off_events.cbegin();

    while (on_iter < on_events.cend() && off_iter < off_events.cend()) {
        if (*on_iter >= *off_iter) {
            ++off_iter;
            continue;
        }
        ranges.emplace_back(*on_iter, *off_iter);
        while (on_iter < on_events.cend() && *on_iter < *off_iter) {
            ++on_iter;
        }
    }

    return ranges;
}

std::vector<SightRead::Solo>
SightRead::Detail::form_solo_vector(const std::vector<int>& solo_on_events,
                                    const std::vector<int>& solo_off_events,
                                    const std::vector<SightRead::Note>& notes,
                                    SightRead::TrackType track_type,
                                    bool is_midi)
{
    constexpr int SOLO_NOTE_VALUE = 100;

    std::vector<SightRead::Solo> solos;

    for (auto [start, end] :
         combine_solo_events(solo_on_events, solo_off_events)) {
        std::set<SightRead::Tick> positions_in_solo;
        auto note_count = 0;
        for (const auto& note : notes) {
            if ((note.position >= start && note.position < end)
                || (note.position == end && !is_midi)) {
                positions_in_solo.insert(note.position);
                ++note_count;
            }
        }
        if (positions_in_solo.empty()) {
            continue;
        }
        if (track_type != SightRead::TrackType::Drums) {
            note_count = static_cast<int>(positions_in_solo.size());
        }
        solos.push_back({start, end, SOLO_NOTE_VALUE * note_count});
    }

    return solos;
}
