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

// This set behaves like two hash sets. One contains permanent elements, the
// second contains temporary elements that can be cleared as desired. The use of
// this data structure is to keep track of already attained activation ends in
// the optimiser to avoid recalculating activations that are completely
// redundant. The temporary elements are needed for end points that can be
// discounted only until another SP phrase is acquired. For instance, the first
// act of the GH:VH Expert guitar chart of Somebody Get Me A Doctor requires the
// player to stop whammying before getting half bar, and waiting until the
// second phrase is acquired before activating.
//
// This class is seen as a pair of subsets of a range [a, b). The key
// optimisation that makes this class useful is that for the optimiser's uses,
// the subset is typically of the form [a, c) and a very low number of extra
// elements after c. Thus by storing c, we can dramatically reduce the size of
// the contained hash sets.
template <typename T> class ActivationEndSet {
private:
    T m_start;
    T m_end;
    T m_min_absent_element;
    boost::unordered_flat_set<T> m_abnormal_elements;
    boost::unordered_flat_set<T> m_temporary_abnormal_elements;

public:
    ActivationEndSet(T start, T end)
        : m_start {start}
        , m_end {end}
        , m_min_absent_element {start}
    {
        assert(start <= end);
    }

    [[nodiscard]] bool contains(T element) const
    {
        if (m_start > element || m_end <= element) {
            return false;
        }
        if (element < m_min_absent_element) {
            return true;
        }
        return m_abnormal_elements.contains(element)
            || m_temporary_abnormal_elements.contains(element);
    }

    [[nodiscard]] T lowest_absent_element() const
    {
        for (auto element = m_min_absent_element; element < m_end; ++element) {
            if (!m_temporary_abnormal_elements.contains(element)) {
                return element;
            }
        }

        return m_end;
    }

    [[nodiscard]] T next_absent_element(T element) const
    {
        for (element = std::max(element + 1, m_min_absent_element);
             element < m_end; ++element) {
            if (!m_abnormal_elements.contains(element)
                && !m_temporary_abnormal_elements.contains(element)) {
                return element;
            }
        }

        return m_end;
    }

    void add(T element)
    {
        assert(m_start <= element);
        assert(element < m_end);
        if (m_min_absent_element == element) {
            ++m_min_absent_element;
            while (m_abnormal_elements.contains(m_min_absent_element)) {
                m_abnormal_elements.erase(m_min_absent_element);
                ++m_min_absent_element;
            }
        } else {
            m_abnormal_elements.insert(element);
        }
    }

    void add_temporary_element(T element)
    {
        assert(m_start <= element);
        assert(element < m_end);

        m_temporary_abnormal_elements.insert(element);
    }

    void clear_temporary_elements() { m_temporary_abnormal_elements.clear(); }
};

#endif
