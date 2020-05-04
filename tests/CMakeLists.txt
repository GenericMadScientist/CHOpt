set(INCLUDE "${PROJECT_SOURCE_DIR}/include")
set(LIBS "${PROJECT_SOURCE_DIR}/libs")
set(SRC "${PROJECT_SOURCE_DIR}/src")

add_library(catch_main STATIC catch_main.cpp)
target_include_directories(catch_main PRIVATE ${LIBS})
set_cpp_standard(catch_main)
set_warnings(catch_main)

function(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_include_directories(${TESTNAME} PRIVATE ${INCLUDE} ${LIBS})
    target_link_libraries(${TESTNAME} PRIVATE catch_main)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_cpp_standard(${TESTNAME})
    set_warnings(${TESTNAME})
endfunction()

package_add_test(chart_unittest chart_unittest.cpp "${SRC}/chart.cpp")

package_add_test(image_unittest image_unittest.cpp "${SRC}/chart.cpp" "${SRC}/image.cpp" "${SRC}/points.cpp" "${SRC}/sp.cpp" "${SRC}/time.cpp")

package_add_test(optimiser_unittest optimiser_unittest.cpp "${SRC}/chart.cpp" "${SRC}/optimiser.cpp" "${SRC}/points.cpp" "${SRC}/processed.cpp" "${SRC}/sp.cpp" "${SRC}/time.cpp")

package_add_test(points_unittest points_unittest.cpp "${SRC}/chart.cpp" "${SRC}/points.cpp" "${SRC}/time.cpp")

package_add_test(processed_unittest processed_unittest.cpp "${SRC}/chart.cpp" "${SRC}/points.cpp" "${SRC}/processed.cpp" "${SRC}/sp.cpp" "${SRC}/time.cpp")

package_add_test(sp_unittest sp_unittest.cpp "${SRC}/chart.cpp" "${SRC}/sp.cpp" "${SRC}/time.cpp")

package_add_test(time_unittest time_unittest.cpp "${SRC}/chart.cpp" "${SRC}/time.cpp")
