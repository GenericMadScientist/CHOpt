add_library(catch_main STATIC catch_main.cpp)

function(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} PRIVATE catch_main)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_cpp_standard(${TESTNAME})
    set_warnings(${TESTNAME})
endfunction()

set(SRC "${PROJECT_SOURCE_DIR}/src")

package_add_test(chart_unittest chart_unittest.cpp "${SRC}/chart.cpp")
target_include_directories(chart_unittest PRIVATE "${SRC}")