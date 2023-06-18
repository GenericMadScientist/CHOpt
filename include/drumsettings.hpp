#ifndef SIGHTREAD_DRUMSETTINGS_HPP
#define SIGHTREAD_DRUMSETTINGS_HPP

namespace SightRead {
struct DrumSettings {
    bool enable_double_kick;
    bool disable_kick;
    bool pro_drums;
    bool enable_dynamics;

    static DrumSettings default_settings()
    {
        return {true, false, true, false};
    }
};
}

#endif
