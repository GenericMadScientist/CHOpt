# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# revision: 2
# Modified from https://github.com/CMakePorts/CMakeFindPackages/FindCairo.cmake
#
# .rst:
# FindPango
# ---------
#
# Locate Pango library
#
# This module defines
#
# :: Pango_FOUND          - system has the Pango library
#    Pango_INCLUDE_DIRS   - the Pango include directory, and those of dependencies
#    Pango_LIBRARIES      - The libraries needed to use Pango
#    Pango_VERSION        - This is set to $major.$minor.$revision (eg. 0.9.8)
#    Pango_VERSION_STRING - This is set to $major.$minor.$revision (eg. 0.9.8)
#
# Authors: Copyright (c)           Eric Wing
#          Copyright (c)           Alexander Neundorf
#          Copyright (c) 2008      Joshua L. Blocher  <verbalshadow at gmail dot com>
#          Copyright (c) 2012      Dmitry Baryshnikov <polimax at mail dot ru>
#          Copyright (c) 2013-2017 Mikhail Paulyshka  <me at mixaill dot tk>

if(Pango_FIND_QUIETLY)
  set(_FIND_GLIB_ARG QUIET)
  set(_FIND_HarfBuzz_ARG QUIET)
endif()
find_package(GLIB ${_FIND_GLIB_ARG} COMPONENTS gobject)
find_package(HarfBuzz ${_FIND_HarfBuzz_ARG})

if(NOT WIN32)
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(_Pango pango)

    set(Pango_VERSION ${_Pango_VERSION})
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\1" num
                         "${Pango_VERSION}")
    math(EXPR Pango_VERSION_MAJOR "${num}")
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\2" num
                         "${Pango_VERSION}")
    math(EXPR Pango_VERSION_MINOR "${num}")
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\3" num
                         "${Pango_VERSION}")
    math(EXPR Pango_VERSION_MICRO "${num}")
  endif(PKG_CONFIG_FOUND)
endif(NOT WIN32)

set(_Pango_ROOT_HINTS_AND_PATHS
    HINTS
    $ENV{Pango}
    $ENV{Pango_DIR}
    ${CMAKE_FIND_ROOT_PATH}
    ${Pango_ROOT_DIR}
    PATHS
    ${CMAKE_FIND_ROOT_PATH}
    $ENV{Pango}/src
    /usr
    /usr/local)

find_path(
  Pango_INCLUDE_DIR
  NAMES "pango/pango.h"
  HINTS ${_Pango_INCLUDEDIR} ${_Pango_ROOT_HINTS_AND_PATHS}
  PATH_SUFFIXES include "include/pango-1.0")

if(NOT Pango_LIBRARY)
  find_library(
    Pango_LIBRARY_RELEASE
    NAMES pango-1.0 pango-static
    HINTS ${Pango_RELEASE_LIBDIR}
    PATH_SUFFIXES "lib" "local/lib"
    NO_DEFAULT_PATH)
  find_library(
    Pango_LIBRARY_RELEASE
    NAMES pango-1.0 pango-static
    HINTS ${_Pango__LIBDIR} ${_Pango_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES "lib" "local/lib")
  find_library(
    PangoCairo_LIBRARY_RELEASE
    NAMES pangocairo-1.0 pangocairo-static
    HINTS ${Pango_RELEASE_LIBDIR}
    PATH_SUFFIXES "lib" "local/lib"
    NO_DEFAULT_PATH)
  find_library(
    PangoCairo_LIBRARY_RELEASE
    NAMES pangocairo-1.0 pangocairo-static
    HINTS ${_Pango__LIBDIR} ${_Pango_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES "lib" "local/lib")

  find_library(
    Pango_LIBRARY_DEBUG
    NAMES pango-1.0 pango-staticd
    HINTS ${Pango_DEBUG_LIBDIR}
    PATH_SUFFIXES "lib" "local/lib"
    NO_DEFAULT_PATH)
  find_library(
    Pango_LIBRARY_DEBUG
    NAMES pango-1.0 pango-staticd
    HINTS ${_Pango_LIBDIR} ${_Pango_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES "lib" "local/lib")
  find_library(
    PangoCairo_LIBRARY_DEBUG
    NAMES pangocairo-1.0 pangocairo-staticd
    HINTS ${Pango_DEBUG_LIBDIR}
    PATH_SUFFIXES "lib" "local/lib"
    NO_DEFAULT_PATH)
  find_library(
    PangoCairo_LIBRARY_DEBUG
    NAMES pangocairo-1.0 pangocairo-staticd
    HINTS ${_Pango_LIBDIR} ${_Pango_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES "lib" "local/lib")

  include(SelectLibraryConfigurations)
  select_library_configurations(Pango)
  select_library_configurations(PangoCairo)
endif()
set(Pango_LIBRARIES ${Pango_LIBRARY} ${PangoCairo_LIBRARY} ${GLIB_LIBRARIES} ${GLIB_GOBJECT_LIBRARIES} ${HarfBuzz_LIBRARIES})
set(Pango_INCLUDE_DIRS ${Pango_INCLUDE_DIR} ${GLIB_INCLUDE_DIRS} ${HarfBuzz_INCLUDE_DIRS})

if(NOT Pango_VERSION)
  if(EXISTS "${Pango_INCLUDE_DIR}/pango-version-macros.h")
    file(READ "${Pango_INCLUDE_DIR}/pango-version-macros.h"
         Pango_VERSION_CONTENT)

    string(REGEX MATCH "#define +Pango_VERSION_MAJOR +([0-9]+)" _dummy
                 "${Pango_VERSION_CONTENT}")
    set(Pango_VERSION_MAJOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define +Pango_VERSION_MINOR +([0-9]+)" _dummy
                 "${Pango_VERSION_CONTENT}")
    set(Pango_VERSION_MINOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define +Pango_VERSION_MICRO +([0-9]+)" _dummy
                 "${Pango_VERSION_CONTENT}")
    set(Pango_VERSION_MICRO "${CMAKE_MATCH_1}")

    set(Pango_VERSION
        "${Pango_VERSION_MAJOR}.${Pango_VERSION_MINOR}.${Pango_VERSION_MICRO}")
    set(Pango_VERSION_STRING Pango_VERSION)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Pango
  REQUIRED_VARS Pango_LIBRARIES Pango_INCLUDE_DIR
  VERSION_VAR Pango_VERSION_STRING)

mark_as_advanced(Pango_INCLUDE_DIRS Pango_LIBRARY Pango_LIBRARIES)
