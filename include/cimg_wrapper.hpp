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

// The entire purpose of this header is to include CImg with warnings
// disabled. On GCC and Clang this can be dealt with by using -Isystem, and the
// MSVC equivalent seems to be to use /external:I (see
// https://devblogs.microsoft.com/cppblog/broken-warnings-theory/). However,
// clang-cl seems to not have this available yet so this hack will have to do.

#ifndef CHOPT_CIMG_WRAPPER_HPP
#define CHOPT_CIMG_WRAPPER_HPP

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#elif defined(__GNUG__)
#pragma GCC system_header
#elif defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#define cimg_display 0 // NOLINT
#include "CImg.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
