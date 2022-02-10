/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2022 Raymond Wright
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

#include <boost/json.hpp>
#include <boost/nowide/fstream.hpp>

#include "json_settings.hpp"

JsonSettings load_saved_settings()
{
    JsonSettings settings;
    settings.squeeze = 100;
    settings.early_whammy = 100;
    settings.lazy_whammy = 0;
    settings.whammy_delay = 0;
    settings.video_lag = 0;

    boost::nowide::ifstream settings_file {"settings.json"};
    if (!settings_file.is_open()) {
        return settings;
    }

    boost::json::error_code ec;
    const boost::json::value jv = boost::json::parse(
        std::string {std::istreambuf_iterator<char>(settings_file),
                     std::istreambuf_iterator<char>()},
        ec);
    if (ec || !jv.is_object()) {
        return settings;
    }

    const auto& obj = jv.get_object();
    auto it = obj.find("squeeze");
    if (it != obj.cend() && it->value().is_int64()) {
        const auto squeeze = it->value().get_int64();
        if (squeeze >= 0 && squeeze <= 100) {
            settings.squeeze = squeeze;
        }
    }
    it = obj.find("early_whammy");
    if (it != obj.cend() && it->value().is_int64()) {
        const auto early_whammy = it->value().get_int64();
        if (early_whammy >= 0 && early_whammy <= 100) {
            settings.early_whammy = early_whammy;
        }
    }
    it = obj.find("lazy_whammy");
    if (it != obj.cend() && it->value().is_int64()) {
        const auto lazy_whammy = it->value().get_int64();
        if (lazy_whammy >= 0 && lazy_whammy <= 999999999) {
            settings.lazy_whammy = lazy_whammy;
        }
    }
    it = obj.find("whammy_delay");
    if (it != obj.cend() && it->value().is_int64()) {
        const auto whammy_delay = it->value().get_int64();
        if (whammy_delay >= 0 && whammy_delay <= 999999999) {
            settings.whammy_delay = whammy_delay;
        }
    }
    it = obj.find("video_lag");
    if (it != obj.cend() && it->value().is_int64()) {
        const auto video_lag = it->value().get_int64();
        if (video_lag >= -200 && video_lag <= 200) {
            settings.video_lag = video_lag;
        }
    }

    return settings;
}

void save_settings(const JsonSettings& settings)
{
    const boost::json::object obj = {{"squeeze", settings.squeeze},
                                     {"early_whammy", settings.early_whammy},
                                     {"lazy_whammy", settings.lazy_whammy},
                                     {"whammy_delay", settings.whammy_delay},
                                     {"video_lag", settings.video_lag}};
    boost::nowide::ofstream settings_file {"settings.json"};
    settings_file << obj;
}
