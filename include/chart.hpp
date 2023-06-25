#ifndef SIGHTREAD_DETAIL_CHART_HPP
#define SIGHTREAD_DETAIL_CHART_HPP

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace SightRead::Detail {
struct BpmEvent {
    int position;
    int bpm;
};

struct Event {
    int position;
    std::string data;
};

struct NoteEvent {
    int position;
    int fret;
    int length;
};

struct SpecialEvent {
    int position;
    int key;
    int length;
};

struct TimeSigEvent {
    int position;
    int numerator;
    int denominator;
};

struct ChartSection {
    std::string name;
    std::map<std::string, std::string> key_value_pairs;
    std::vector<BpmEvent> bpm_events;
    std::vector<Event> events;
    std::vector<NoteEvent> note_events;
    std::vector<SpecialEvent> special_events;
    std::vector<TimeSigEvent> ts_events;
};

struct Chart {
    std::vector<ChartSection> sections;
};

Chart parse_chart(std::string_view data);
}

#endif
