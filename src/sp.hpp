/*
 * chopt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020 Raymond Wright
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

#ifndef CHOPT_SP_HPP
#define CHOPT_SP_HPP

// Represents the minimum and maximum SP possible at a given time.
class SpBar {
private:
    double m_min;
    double m_max;

public:
    SpBar(double min, double max)
        : m_min {min}
        , m_max {max}
    {
    }

    double& min() { return m_min; }
    double& max() { return m_max; }
    [[nodiscard]] double min() const { return m_min; }
    [[nodiscard]] double max() const { return m_max; }

    friend bool operator==(const SpBar& lhs, const SpBar& rhs)
    {
        return std::tie(lhs.m_min, lhs.m_max) == std::tie(rhs.m_min, rhs.m_max);
    }

    void add_phrase()
    {
        constexpr double SP_PHRASE_AMOUNT = 0.25;

        m_min += SP_PHRASE_AMOUNT;
        m_max += SP_PHRASE_AMOUNT;
        m_min = std::min(m_min, 1.0);
        m_max = std::min(m_max, 1.0);
    }

    [[nodiscard]] bool full_enough_to_activate() const
    {
        constexpr double MINIMUM_SP_AMOUNT = 0.5;
        return m_max >= MINIMUM_SP_AMOUNT;
    }
};

#endif
