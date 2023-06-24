#ifndef SIGHTREAD_DETAIL_PARSERUTIL_HPP
#define SIGHTREAD_DETAIL_PARSERUTIL_HPP

#include <tuple>
#include <vector>

#include <sightread/songparts.hpp>
#include <sightread/time.hpp>

namespace SightRead {
namespace Detail {
    bool is_six_fret_instrument(SightRead::Instrument instrument);

    // Takes a sequence of points where some note type/event is turned on, and a
    // sequence where said type is turned off, and returns a tuple of intervals
    // where the event is on.
    std::vector<std::tuple<SightRead::Tick, SightRead::Tick>>
    combine_solo_events(const std::vector<int>& on_events,
                        const std::vector<int>& off_events);

    std::vector<SightRead::Solo>
    form_solo_vector(const std::vector<int>& solo_on_events,
                     const std::vector<int>& solo_off_events,
                     const std::vector<SightRead::Note>& notes,
                     SightRead::TrackType track_type, bool is_midi);
}
}

#endif
