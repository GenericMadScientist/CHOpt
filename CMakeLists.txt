cmake_minimum_required(VERSION 3.15.0)

cmake_policy(SET CMP0091 NEW)

project(
  chopt
  VERSION 1.4.1
  DESCRIPTION "A program to generate optimal Star Power paths for Clone Hero"
  LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
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
            -Wno-c++98-compat
            -Wno-unknown-warning-option>
            $<$<CXX_COMPILER_ID:MSVC>:
            /W4
            /WX>)
endfunction()

find_package(PNG REQUIRED)
find_package(
  Boost 1.76 REQUIRED
  COMPONENTS nowide program_options
  OPTIONAL_COMPONENTS json unit_test_framework)
find_package(Cairo REQUIRED)

cmake_policy(SET CMP0083 NEW)
include(CheckPIESupported)
check_pie_supported(OUTPUT_VARIABLE output LANGUAGES CXX)
if(NOT CMAKE_CXX_LINK_NO_PIE_SUPPORTED)
  message(VERBOSE "No PIE is not supported at link time: ${output}.\n"
          "PIE link options will not be passed to linker.")
endif()

add_executable(
  chopt
  src/main.cpp
  src/chart.cpp
  src/image.cpp
  src/imagebuilder.cpp
  src/ini.cpp
  src/midi.cpp
  src/optimiser.cpp
  src/points.cpp
  src/processed.cpp
  src/settings.cpp
  src/song.cpp
  src/sp.cpp
  src/stringutil.cpp
  src/timeconverter.cpp
  resources/chopt.exe.manifest
  resources/resources.rc)
target_include_directories(
  chopt PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/libs"
                ${PNG_INCLUDE_DIRS} ${Cairo_INCLUDE_DIR})
target_link_directories(chopt PRIVATE ${Boost_LIBRARY_DIRS})
target_link_libraries(chopt PRIVATE Boost::nowide Boost::program_options
                                    ${PNG_LIBRARIES} ${Cairo_LIBRARIES})

set_property(TARGET chopt PROPERTY POSITION_INDEPENDENT_CODE FALSE)
set_cpp_standard(chopt)
set_warnings(chopt)
enable_sanitisers(chopt)

option(BUILD_GUI "Build the GUI version" ON)
if(BUILD_GUI)
  if(NOT Boost_json_FOUND)
    message(FATAL_ERROR "GUI version needs Boost::JSON in order to be built")
  endif()

  find_package(
    Qt6
    COMPONENTS Widgets
    REQUIRED)

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTORCC ON)
  set(CMAKE_AUTOUIC ON)
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  add_executable(
    choptgui WIN32
    gui/json_settings.cpp
    gui/main.cpp
    gui/mainwindow.cpp
    gui/mainwindow.ui
    src/chart.cpp
    src/image.cpp
    src/imagebuilder.cpp
    src/ini.cpp
    src/midi.cpp
    src/optimiser.cpp
    src/points.cpp
    src/processed.cpp
    src/song.cpp
    src/sp.cpp
    src/stringutil.cpp
    src/timeconverter.cpp
    resources/choptgui.exe.manifest
    resources/resources.qrc
    resources/resources.rc)
  target_include_directories(
    choptgui
    PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/libs"
            ${PNG_INCLUDE_DIRS} ${Cairo_INCLUDE_DIR})
  target_link_directories(choptgui PRIVATE ${Boost_LIBRARY_DIRS})
  target_link_libraries(
    choptgui PRIVATE Boost::json Boost::nowide ${PNG_LIBRARIES}
                     ${Cairo_LIBRARIES} Qt6::Widgets)

  set_property(TARGET choptgui PROPERTY POSITION_INDEPENDENT_CODE FALSE)
  if(APPLE)
    set_property(TARGET choptgui PROPERTY MACOSX_BUNDLE TRUE)
  endif()
  set_cpp_standard(choptgui)
endif()

include(CTest)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
  if(NOT Boost_unit_test_framework_FOUND)
    message(FATAL_ERROR "Tests need Boost::Test in order to be built")
  endif()

  enable_testing()
  add_executable(
    chopt_tests
    tests/test_main.cpp
    tests/chart_unittest.cpp
    tests/imagebuilder_unittest.cpp
    tests/ini_unittest.cpp
    tests/midi_unittest.cpp
    tests/optimiser_unittest.cpp
    tests/points_unittest.cpp
    tests/processed_unittest.cpp
    tests/song_unittest.cpp
    tests/songparts_unittest.cpp
    tests/sp_unittest.cpp
    tests/stringutil_unittest.cpp
    tests/time_unittest.cpp
    tests/timeconverter_unittest.cpp
    src/chart.cpp
    src/imagebuilder.cpp
    src/ini.cpp
    src/midi.cpp
    src/optimiser.cpp
    src/points.cpp
    src/processed.cpp
    src/song.cpp
    src/sp.cpp
    src/stringutil.cpp
    src/timeconverter.cpp)

  target_include_directories(chopt_tests
                             PRIVATE "${PROJECT_SOURCE_DIR}/include")
  target_link_directories(chopt_tests PRIVATE ${Boost_LIBRARY_DIRS})
  target_link_libraries(chopt_tests PRIVATE Boost::nowide
                                            Boost::unit_test_framework)
  add_test(NAME chopt_tests COMMAND chopt_tests)
  set_cpp_standard(chopt_tests)
  set_warnings(chopt_tests)
  enable_sanitisers(chopt_tests)
endif()

option(BUILD_FUZZ_TARGETS "Build the fuzzing targets" OFF)
if(BUILD_FUZZ_TARGETS)
  add_subdirectory(fuzzing_targets)
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
    if(BUILD_GUI)
      set_property(TARGET choptgui PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
  else()
    message(WARNING "LTO is not supported: ${output}")
  endif()
endif()
