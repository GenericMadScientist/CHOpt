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

#ifndef CHOPT_JSON_SETTINGS_HPP
#define CHOPT_JSON_SETTINGS_HPP

#include <string_view>

struct JsonSettings {
    int squeeze;
    int early_whammy;
    int lazy_whammy;
    int whammy_delay;
    int video_lag;
};

JsonSettings load_saved_settings(std::string_view application_dir);
void save_settings(const JsonSettings& settings,
                   std::string_view application_dir);

#endif
