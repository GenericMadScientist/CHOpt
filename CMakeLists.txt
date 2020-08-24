cmake_minimum_required(VERSION 3.15.0)

project(
  chopt
  VERSION 0.6.2
  DESCRIPTION "A tool to generate optimal SP paths for Clone Hero"
  LANGUAGES CXX)

include(cmake/Sanitisers.cmake)

# Require standard C++17
function(set_cpp_standard target)
  set_target_properties(
    ${target}
    PROPERTIES CXX_STANDARD 17
               CXX_STANDARD_REQUIRED ON
               CXX_EXTENSIONS OFF)
endfunction()

# Set warning flags
function(set_warnings target)
  target_compile_options(
    ${target}
    PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:
            -Wall
            -Wextra
            -Werror
            -Wno-c++98-compat>
            $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /WX>)
endfunction()

find_package(PNG REQUIRED)

include(FetchContent)

FetchContent_Declare(
  Nowide
  GIT_REPOSITORY "https://github.com/boostorg/nowide"
  GIT_TAG "02f40f0b5f5686627fcddae93ff88ca399db4766")

FetchContent_MakeAvailable(Nowide)

add_executable(
  chopt
  src/main.cpp
  src/chart.cpp
  src/image.cpp
  src/midi.cpp
  src/optimiser.cpp
  src/points.cpp
  src/processed.cpp
  src/settings.cpp
  src/song.cpp
  src/sp.cpp
  src/time.cpp)
target_include_directories(
  chopt PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/libs"
                ${PNG_INCLUDE_DIRS})
target_link_libraries(chopt PRIVATE nowide ${PNG_LIBRARIES})

set_cpp_standard(chopt)
set_warnings(chopt)
enable_sanitisers(chopt)

option(BUILD_GUI "Build the GUI version" ON)
if(BUILD_GUI)
  find_package(
    Qt5
    COMPONENTS Widgets
    REQUIRED)

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  add_executable(
    choptgui WIN32
    gui/main.cpp
    gui/mainwindow.cpp
    gui/mainwindow.ui
    src/chart.cpp
    src/image.cpp
    src/midi.cpp
    src/optimiser.cpp
    src/points.cpp
    src/processed.cpp
    src/song.cpp
    src/sp.cpp
    src/time.cpp)
  target_include_directories(
    choptgui PRIVATE "${PROJECT_SOURCE_DIR}/include"
                     "${PROJECT_SOURCE_DIR}/libs" ${PNG_INCLUDE_DIRS})
  target_link_libraries(choptgui PRIVATE nowide ${PNG_LIBRARIES} Qt5::Widgets)

  set_cpp_standard(choptgui)
endif()

include(CTest)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

option(ENABLE_LTO "Enable Link Time Optimisation" OFF)
if(ENABLE_LTO)
  cmake_policy(SET CMP0069 NEW)

  include(CheckIPOSupported)
  check_ipo_supported(
    RESULT supported
    OUTPUT output
    LANGUAGES CXX)
  if(supported)
    set_property(TARGET chopt PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(WARNING "LTO is not supported: ${output}")
  endif()
endif()
