/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include <algorithm>
#include <array>
#include <stdexcept>

#include <QByteArrayView>
#include <QChar>
#include <QString>
#include <QStringConverter>
#include <QStringDecoder>

#include <sightread/songparts.hpp>

#include "stringutil.hpp"

std::string_view break_off_newline(std::string_view& input)
{
    if (input.empty()) {
        throw SightRead::ParseError("No lines left");
    }

    const auto newline_location
        = std::min(input.find('\n'), input.find("\r\n"));
    if (newline_location == std::string_view::npos) {
        const auto line = input;
        input.remove_prefix(input.size());
        return line;
    }

    const auto line = input.substr(0, newline_location);
    input.remove_prefix(newline_location);
    input = skip_whitespace(input);
    return line;
}

std::string_view skip_whitespace(std::string_view input)
{
    const auto first_non_ws_location = input.find_first_not_of(" \f\n\r\t\v");
    input.remove_prefix(std::min(first_non_ws_location, input.size()));
    return input;
}

std::string to_ordinal(int ordinal)
{
    constexpr int TENS_MODULUS = 10;
    constexpr int HUNDREDS_MODULUS = 100;
    constexpr std::array<int, 3> EXCEPTIONAL_TEENS {11, 12, 13};

    if (ordinal < 0) {
        throw std::runtime_error("ordinal was negative");
    }
    if (std::find(EXCEPTIONAL_TEENS.cbegin(), EXCEPTIONAL_TEENS.cend(),
                  ordinal % HUNDREDS_MODULUS)
        != EXCEPTIONAL_TEENS.cend()) {
        return std::to_string(ordinal) + "th";
    }
    if (ordinal % TENS_MODULUS == 1) {
        return std::to_string(ordinal) + "st";
    }
    if (ordinal % TENS_MODULUS == 2) {
        return std::to_string(ordinal) + "nd";
    }
    if (ordinal % TENS_MODULUS == 3) {
        return std::to_string(ordinal) + "rd";
    }
    return std::to_string(ordinal) + "th";
}

std::string to_utf8_string(std::string_view input)
{
    const QByteArrayView byte_view {input.data(),
                                    static_cast<qsizetype>(input.size())};

    QStringDecoder to_utf8 {QStringDecoder::Utf8};
    QString str = to_utf8(byte_view);
    if (!to_utf8.hasError()) {
        return str.toStdString();
    }

    QStringDecoder to_latin_1 {QStringDecoder::Latin1};
    str = to_latin_1(byte_view);
    if (!to_latin_1.hasError() && !str.contains(QChar {0})) {
        return str.toStdString();
    }

    QStringDecoder to_utf16_le {QStringDecoder::Utf16LE};
    str = to_utf16_le(byte_view);
    if (!to_utf16_le.hasError()) {
        return str.toStdString();
    }

    throw std::runtime_error("Unable to determine string encoding");
}
