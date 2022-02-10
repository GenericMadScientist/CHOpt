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

#include <filesystem>

#include <boost/json.hpp>
#include <boost/nowide/fstream.hpp>

#include "json_settings.hpp"

std::filesystem::path settings_path(std::string_view application_dir)
{
    return std::filesystem::path(application_dir) / "settings.json";
}

static int read_value(const boost::json::object& settings, const char* name,
                      int min_value, int max_value, int default_value)
{
    const auto* it = settings.find(name);
    if (it != settings.cend() && it->value().is_int64()) {
        const auto value = it->value().get_int64();
        if (value >= min_value && value <= max_value) {
            return static_cast<int>(value);
        }
    }
    return default_value;
}

JsonSettings load_saved_settings(std::string_view application_dir)
{
    constexpr int MAX_LINE_EDIT_INT = 999999999;
    constexpr int MAX_PERCENT = 100;
    constexpr int MAX_VIDEO_LAG = 200;
    constexpr int MIN_VIDEO_LAG = -200;

    JsonSettings settings {};
    settings.squeeze = MAX_PERCENT;
    settings.early_whammy = MAX_PERCENT;
    settings.lazy_whammy = 0;
    settings.whammy_delay = 0;
    settings.video_lag = 0;

    const auto path = settings_path(application_dir);
    boost::nowide::ifstream settings_file {path};
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
    settings.squeeze = read_value(obj, "squeeze", 0, MAX_PERCENT, MAX_PERCENT);
    settings.early_whammy
        = read_value(obj, "early_whammy", 0, MAX_PERCENT, MAX_PERCENT);
    settings.lazy_whammy
        = read_value(obj, "lazy_whammy", 0, MAX_LINE_EDIT_INT, 0);
    settings.whammy_delay
        = read_value(obj, "whammy_delay", 0, MAX_LINE_EDIT_INT, 0);
    settings.video_lag
        = read_value(obj, "video_lag", MIN_VIDEO_LAG, MAX_VIDEO_LAG, 0);

    return settings;
}

void save_settings(const JsonSettings& settings,
                   std::string_view application_dir)
{
    const boost::json::object obj = {{"squeeze", settings.squeeze},
                                     {"early_whammy", settings.early_whammy},
                                     {"lazy_whammy", settings.lazy_whammy},
                                     {"whammy_delay", settings.whammy_delay},
                                     {"video_lag", settings.video_lag}};
    const auto path = settings_path(application_dir);
    boost::nowide::ofstream settings_file {path};
    settings_file << obj;
}
