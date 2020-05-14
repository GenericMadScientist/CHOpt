cmake_minimum_required(VERSION 3.15.0)

project(chopt
    VERSION 0.3.2
    DESCRIPTION "A tool to generate optimal SP paths for Clone Hero"
    LANGUAGES CXX)

# Require standard C++17
function(set_cpp_standard TARGET)
    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endfunction()

# Set warning flags
function(set_warnings TARGET)
    target_compile_options(${TARGET} PRIVATE
        $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>: -Wall -Wextra -Werror -Wno-c++98-compat>
        $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX>
    )
endfunction()

find_package(PNG REQUIRED)

add_executable(chopt src/main.cpp src/chart.cpp src/image.cpp src/optimiser.cpp src/points.cpp src/processed.cpp src/settings.cpp src/sp.cpp src/time.cpp)
target_include_directories(chopt PRIVATE "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/libs")

set_cpp_standard(chopt)
set_warnings(chopt)

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
    check_ipo_supported(RESULT supported OUTPUT output LANGUAGES CXX)
    if(supported)
        set_property(TARGET chopt PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO is not supported: ${output}")
    endif()
endif()
