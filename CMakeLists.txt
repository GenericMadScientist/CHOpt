cmake_minimum_required(VERSION 3.16.3)

project(chopt
    VERSION 0.1.0
    DESCRIPTION "A tool to generate optimal SP paths in Clone Hero"
    LANGUAGES CXX)

add_executable(chopt src/main.cpp src/chart.cpp)

# Require standard C++17
set_target_properties(chopt PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Set warning flags
target_compile_options(chopt PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>: -Wall -Wextra -Werror -Wno-c++98-compat>
    $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX>
)