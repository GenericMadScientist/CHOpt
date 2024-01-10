/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2022, 2024 Raymond Wright
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

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "json_settings.hpp"

namespace {
struct IntRange {
    int min;
    int max;
};

int read_value(const QJsonObject& settings, const QString& name, IntRange range,
               int default_value)
{
    const auto value = settings.value(name).toInt(default_value);
    if (value >= range.min && value <= range.max) {
        return value;
    }
    return default_value;
}

bool read_json_bool(const QJsonObject& settings, const QString& name,
                    bool default_value)
{
    return settings.value(name).toBool(default_value);
}

QString settings_path(std::string_view application_dir)
{
    const auto path = std::filesystem::path(application_dir) / "settings.json";
    return QString::fromStdString(path.string());
}
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
    settings.is_lefty_flip = false;

    QFile settings_file {settings_path(application_dir)};
    if (!settings_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return settings;
    }

    const auto settings_file_bytes = settings_file.readAll();
    const auto jv = QJsonDocument::fromJson(settings_file_bytes);
    if (jv.isNull() || !jv.isObject()) {
        return settings;
    }

    const auto obj = jv.object();
    settings.squeeze
        = read_value(obj, "squeeze", {0, MAX_PERCENT}, MAX_PERCENT);
    settings.early_whammy
        = read_value(obj, "early_whammy", {0, MAX_PERCENT}, MAX_PERCENT);
    settings.lazy_whammy
        = read_value(obj, "lazy_whammy", {0, MAX_LINE_EDIT_INT}, 0);
    settings.whammy_delay
        = read_value(obj, "whammy_delay", {0, MAX_LINE_EDIT_INT}, 0);
    settings.video_lag
        = read_value(obj, "video_lag", {MIN_VIDEO_LAG, MAX_VIDEO_LAG}, 0);
    settings.is_lefty_flip = read_json_bool(obj, "lefty_flip", false);

    return settings;
}

void save_settings(const JsonSettings& settings,
                   std::string_view application_dir)
{
    const QJsonObject obj = {{"squeeze", settings.squeeze},
                             {"early_whammy", settings.early_whammy},
                             {"lazy_whammy", settings.lazy_whammy},
                             {"whammy_delay", settings.whammy_delay},
                             {"video_lag", settings.video_lag},
                             {"lefty_flip", settings.is_lefty_flip}};
    QFile settings_file {settings_path(application_dir)};
    if (settings_file.open(QIODevice::WriteOnly)) {
        settings_file.write(QJsonDocument(obj).toJson());
    }
}
