set(LIBS "${PROJECT_SOURCE_DIR}/libs")
set(SRC "${PROJECT_SOURCE_DIR}/src")

add_library(catch_main STATIC catch_main.cpp)
target_include_directories(catch_main PRIVATE "${LIBS}")
set_cpp_standard(catch_main)
set_warnings(catch_main)

function(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_include_directories(${TESTNAME} PRIVATE "${LIBS}")
    target_link_libraries(${TESTNAME} PRIVATE catch_main)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_cpp_standard(${TESTNAME})
    set_warnings(${TESTNAME})
endfunction()

package_add_test(chart_unittest chart_unittest.cpp "${SRC}/chart.cpp")
target_include_directories(chart_unittest PRIVATE "${SRC}")

package_add_test(image_unittest image_unittest.cpp "${SRC}/chart.cpp" "${SRC}/image.cpp")
target_include_directories(image_unittest PRIVATE "${SRC}")

package_add_test(optimiser_unittest optimiser_unittest.cpp "${SRC}/chart.cpp" "${SRC}/optimiser.cpp" "${SRC}/sp.cpp" "${SRC}/time.cpp")
target_include_directories(optimiser_unittest PRIVATE "${SRC}")

package_add_test(sp_unittest sp_unittest.cpp "${SRC}/chart.cpp" "${SRC}/sp.cpp" "${SRC}/time.cpp")
target_include_directories(sp_unittest PRIVATE "${SRC}")

package_add_test(time_unittest time_unittest.cpp "${SRC}/chart.cpp" "${SRC}/time.cpp")
target_include_directories(time_unittest PRIVATE "${SRC}")
