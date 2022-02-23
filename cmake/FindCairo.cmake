# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# revision: 2
# See https://github.com/CMakePorts/CMakeFindPackages for updates
#
# .rst:
# FindCairo
# ---------
#
# Locate Cairo library
#
# This module defines
#
# :: Cairo_FOUND          - system has the Cairo library
#    Cairo_INCLUDE_DIR    - the Cairo include directory
#    Cairo_LIBRARIES      - The libraries needed to use Cairo
#    Cairo_VERSION        - This is set to $major.$minor.$revision (eg. 0.9.8)
#    Cairo_VERSION_STRING - This is set to $major.$minor.$revision (eg. 0.9.8)
#
# Authors: Copyright (c)           Eric Wing
#          Copyright (c)           Alexander Neundorf
#          Copyright (c) 2008      Joshua L. Blocher  <verbalshadow at gmail dot com>
#          Copyright (c) 2012      Dmitry Baryshnikov <polimax at mail dot ru>
#          Copyright (c) 2013-2017 Mikhail Paulyshka  <me at mixaill dot tk>

if(NOT WIN32)
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(_Cairo cairo)

    set(Cairo_VERSION ${_Cairo_VERSION})
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\1" num
                         "${Cairo_VERSION}")
    math(EXPR Cairo_VERSION_MAJOR "${num}")
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\2" num
                         "${Cairo_VERSION}")
    math(EXPR Cairo_VERSION_MINOR "${num}")
    string(REGEX REPLACE "([0-9]+).([0-9]+).([0-9]+)" "\\3" num
                         "${Cairo_VERSION}")
    math(EXPR Cairo_VERSION_MICRO "${num}")
  endif(PKG_CONFIG_FOUND)
endif(NOT WIN32)

set(_Cairo_ROOT_HINTS_AND_PATHS
    HINTS
    $ENV{Cairo}
    $ENV{Cairo_DIR}
    ${CMAKE_FIND_ROOT_PATH}
    ${Cairo_ROOT_DIR}
    PATHS
    ${CMAKE_FIND_ROOT_PATH}
    $ENV{Cairo}/src
    /usr
    /usr/local)

find_path(
  Cairo_INCLUDE_DIR
  NAMES cairo.h
  HINTS ${_Cairo_INCLUDEDIR} ${_Cairo_ROOT_HINTS_AND_PATHS}
  PATH_SUFFIXES include "include/cairo")

if(NOT Cairo_LIBRARY)
  find_library(
    Cairo_LIBRARY_RELEASE
    NAMES cairo cairo-static
    HINTS ${Cairo_RELEASE_LIBDIR}
    PATH_SUFFIXES "lib" "local/lib"
    NO_DEFAULT_PATH)
  find_library(
    Cairo_LIBRARY_RELEASE
    NAMES cairo cairo-static
    HINTS ${_Cairo_LIBDIR} ${_Cairo_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES "lib" "local/lib")

  find_library(
    Cairo_LIBRARY_RELEASE
    NAMES cairo cairo-staticd
    HINTS ${Cairo_DEBUG_LIBDIR}
    PATH_SUFFIXES "lib" "local/lib"
    NO_DEFAULT_PATH)
  find_library(
    Cairo_LIBRARY_DEBUG
    NAMES cairod cairo-staticd
    HINTS ${_Cairo_LIBDIR} ${_Cairo_ROOT_HINTS_AND_PATHS}
    PATH_SUFFIXES "lib" "local/lib")

  include(SelectLibraryConfigurations)
  select_library_configurations(Cairo)
endif()
set(Cairo_LIBRARIES ${Cairo_LIBRARY})

if(NOT Cairo_VERSION)
  if(EXISTS "${Cairo_INCLUDE_DIR}/cairo-version.h")
    file(READ "${Cairo_INCLUDE_DIR}/cairo-version.h" Cairo_VERSION_CONTENT)

    string(REGEX MATCH "#define +Cairo_VERSION_MAJOR +([0-9]+)" _dummy
                 "${Cairo_VERSION_CONTENT}")
    set(Cairo_VERSION_MAJOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define +Cairo_VERSION_MINOR +([0-9]+)" _dummy
                 "${Cairo_VERSION_CONTENT}")
    set(Cairo_VERSION_MINOR "${CMAKE_MATCH_1}")

    string(REGEX MATCH "#define +Cairo_VERSION_MICRO +([0-9]+)" _dummy
                 "${Cairo_VERSION_CONTENT}")
    set(Cairo_VERSION_MICRO "${CMAKE_MATCH_1}")

    set(Cairo_VERSION
        "${Cairo_VERSION_MAJOR}.${Cairo_VERSION_MINOR}.${Cairo_VERSION_MICRO}")
    set(Cairo_VERSION_STRING Cairo_VERSION)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Cairo
  REQUIRED_VARS Cairo_LIBRARIES Cairo_INCLUDE_DIR
  VERSION_VAR Cairo_VERSION_STRING)

mark_as_advanced(Cairo_INCLUDE_DIR Cairo_LIBRARY Cairo_LIBRARIES)
