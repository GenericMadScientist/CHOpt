cmake_minimum_required(VERSION 3.16.2)

project(chopt
    VERSION 0.1.0
    DESCRIPTION "A tool to generate optimal SP paths in Clone Hero"
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

add_executable(chopt src/main.cpp src/chart.cpp src/optimiser.cpp src/time.cpp)

set_cpp_standard(chopt)
set_warnings(chopt)

include(CTest)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
