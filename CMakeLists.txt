cmake_minimum_required(VERSION 3.30.0)

project(
  chopt
  VERSION 1.9.5
  DESCRIPTION "A program to generate optimal Star Power paths for Clone Hero"
  LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
include(cmake/Sanitisers.cmake)

# Require standard C++20
function(set_cpp_standard target)
  set_target_properties(
    ${target}
    PROPERTIES CXX_STANDARD 20
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
  Qt6 REQUIRED
  COMPONENTS Core
  OPTIONAL_COMPONENTS Gui Widgets)
find_package(
  Boost 1.77 REQUIRED
  COMPONENTS locale
  OPTIONAL_COMPONENTS unit_test_framework)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_subdirectory("extern/sightread")

include(CheckPIESupported)
check_pie_supported(OUTPUT_VARIABLE output LANGUAGES CXX)

if(NOT CMAKE_CXX_LINK_NO_PIE_SUPPORTED)
  message(VERBOSE "No PIE is not supported at link time: ${output}.\n"
    "PIE link options will not be passed to linker.")
endif()

add_executable(
  chopt
  src/main.cpp
  src/image.cpp
  src/imagebuilder.cpp
  src/ini.cpp
  src/optimiser.cpp
  src/points.cpp
  src/processed.cpp
  src/settings.cpp
  src/songfile.cpp
  src/sp.cpp
  src/sptimemap.cpp
  src/stringutil.cpp
  resources/chopt.exe.manifest
  resources/resources.qrc
  resources/resources.rc)
target_include_directories(
  chopt PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/libs" ${PNG_INCLUDE_DIRS})
target_link_libraries(chopt PRIVATE ${PNG_LIBRARIES} Boost::locale Qt6::Core Qt6::Gui sightread)

set_property(TARGET chopt PROPERTY POSITION_INDEPENDENT_CODE FALSE)
if(APPLE)
  set_property(TARGET chopt PROPERTY MACOSX_BUNDLE TRUE)
endif()
set_cpp_standard(chopt)
set_warnings(chopt)
enable_sanitisers(chopt)

option(BUILD_GUI "Build the GUI version" ON)

if(BUILD_GUI)
  add_executable(
    choptgui WIN32
    gui/json_settings.cpp
    gui/main.cpp
    gui/mainwindow.cpp
    gui/mainwindow.ui
    src/image.cpp
    src/imagebuilder.cpp
    src/ini.cpp
    src/optimiser.cpp
    src/points.cpp
    src/processed.cpp
    src/settings.cpp
    src/songfile.cpp
    src/sp.cpp
    src/sptimemap.cpp
    src/stringutil.cpp
    resources/choptgui.exe.manifest
    resources/resources.qrc
    resources/resources.rc)
  target_include_directories(
    choptgui PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/libs" ${PNG_INCLUDE_DIRS})
  target_link_libraries(choptgui PRIVATE ${PNG_LIBRARIES} Boost::locale Qt6::Widgets sightread)

  set_property(TARGET choptgui PROPERTY POSITION_INDEPENDENT_CODE FALSE)

  if(APPLE)
    set_property(TARGET choptgui PROPERTY MACOSX_BUNDLE TRUE)
  endif()

  set_cpp_standard(choptgui)
endif()

include(CTest)

option(PACKAGE_TESTS "Build the tests" ON)

if(PACKAGE_TESTS)
  enable_testing()
  add_executable(
    chopt_tests
    tests/test_main.cpp
    tests/imagebuilder_unittest.cpp
    tests/ini_unittest.cpp
    tests/optimiser_unittest.cpp
    tests/points_unittest.cpp
    tests/processed_unittest.cpp
    tests/sp_unittest.cpp
    tests/stringutil_unittest.cpp
    src/imagebuilder.cpp
    src/ini.cpp
    src/optimiser.cpp
    src/points.cpp
    src/processed.cpp
    src/settings.cpp
    src/sp.cpp
    src/sptimemap.cpp
    src/stringutil.cpp)

  target_include_directories(chopt_tests
    PRIVATE "${PROJECT_SOURCE_DIR}/include")
  target_link_libraries(chopt_tests PRIVATE Boost::locale Boost::unit_test_framework Qt6::Core sightread)
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
