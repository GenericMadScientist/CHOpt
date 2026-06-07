/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2026 Raymond Wright
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

#ifndef CHOPT_ACTIVATIONENDSET_HPP
#define CHOPT_ACTIVATIONENDSET_HPP

#include <cassert>

#include <boost/unordered/unordered_flat_set.hpp>

// The idea is this is like a std::set<PointPtr>, but is add-only and
// takes advantage of the fact that we often tend to add all elements
// before a certain point.
template <typename T> class ActivationEndSet {
private:
    T m_start;
    T m_end;
    T m_min_absent_ptr;
    boost::unordered_flat_set<T> m_abnormal_elements;

public:
    ActivationEndSet(T start, T end)
        : m_start {start}
        , m_end {end}
        , m_min_absent_ptr {start}
    {
        assert(start <= end);
    }

    [[nodiscard]] bool contains(T element) const
    {
        if (m_start > element || m_end <= element) {
            return false;
        }
        if (element < m_min_absent_ptr) {
            return true;
        }
        return m_abnormal_elements.contains(element);
    }

    [[nodiscard]] T lowest_absent_element() const { return m_min_absent_ptr; }

    [[nodiscard]] T next_absent_element(T element) const
    {
        while (++element < m_end) {
            if (!m_abnormal_elements.contains(element)) {
                return element;
            }
        }

        return m_end;
    }

    void add(T element)
    {
        assert(m_start <= element);
        assert(element < m_end);
        if (m_min_absent_ptr == element) {
            ++m_min_absent_ptr;
            while (m_abnormal_elements.contains(m_min_absent_ptr)) {
                m_abnormal_elements.erase(m_min_absent_ptr);
                ++m_min_absent_ptr;
            }
        } else {
            m_abnormal_elements.insert(element);
        }
    }
};

#endif
