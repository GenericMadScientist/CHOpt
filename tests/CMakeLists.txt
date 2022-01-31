set(INCLUDE "${PROJECT_SOURCE_DIR}/include")
set(LIBS "${PROJECT_SOURCE_DIR}/libs")
set(SRC "${PROJECT_SOURCE_DIR}/src")

add_library(catch_main STATIC catch_main.cpp)
target_include_directories(catch_main PRIVATE ${LIBS})
set_cpp_standard(catch_main)
set_warnings(catch_main)

# Create a new unittest, doing the required boilerplate
function(package_add_test testname)
  add_executable(${testname} ${ARGN})
  target_include_directories(${testname} PRIVATE ${INCLUDE} ${LIBS})
  target_link_directories(${testname} PRIVATE ${Boost_LIBRARY_DIRS})
  target_link_libraries(${testname} PRIVATE catch_main Boost::nowide)
  add_test(NAME ${testname} COMMAND ${testname})
  set_cpp_standard(${testname})
  set_warnings(${testname})
  enable_sanitisers(${testname})
endfunction()

package_add_test(
  optimiser_unittest
  optimiser_unittest.cpp
  "${SRC}/optimiser.cpp"
  "${SRC}/points.cpp"
  "${SRC}/processed.cpp"
  "${SRC}/sp.cpp"
  "${SRC}/stringutil.cpp"
  "${SRC}/timeconverter.cpp")

package_add_test(
  processed_unittest
  processed_unittest.cpp
  "${SRC}/points.cpp"
  "${SRC}/processed.cpp"
  "${SRC}/sp.cpp"
  "${SRC}/stringutil.cpp"
  "${SRC}/timeconverter.cpp")

add_executable(
  chopt_tests
  test_main.cpp
  chart_unittest.cpp
  imagebuilder_unittest.cpp
  ini_unittest.cpp
  midi_unittest.cpp
  points_unittest.cpp
  song_unittest.cpp
  songparts_unittest.cpp
  sp_unittest.cpp
  stringutil_unittest.cpp
  time_unittest.cpp
  timeconverter_unittest.cpp
  "${SRC}/chart.cpp"
  "${SRC}/imagebuilder.cpp"
  "${SRC}/ini.cpp"
  "${SRC}/midi.cpp"
  "${SRC}/optimiser.cpp"
  "${SRC}/points.cpp"
  "${SRC}/processed.cpp"
  "${SRC}/song.cpp"
  "${SRC}/sp.cpp"
  "${SRC}/stringutil.cpp"
  "${SRC}/timeconverter.cpp")
target_include_directories(chopt_tests PRIVATE ${INCLUDE} ${LIBS})
target_link_directories(chopt_tests PRIVATE ${Boost_LIBRARY_DIRS})
target_link_libraries(chopt_tests PRIVATE Boost::nowide
                                          Boost::unit_test_framework)
add_test(NAME chopt_tests COMMAND chopt_tests)
set_cpp_standard(chopt_tests)
set_warnings(chopt_tests)
enable_sanitisers(chopt_tests)
