set(INCLUDE "${PROJECT_SOURCE_DIR}/include")
set(LIBS "${PROJECT_SOURCE_DIR}/libs")
set(SRC "${PROJECT_SOURCE_DIR}/src")

add_library(catch_main STATIC catch_main.cpp)
target_include_directories(catch_main PRIVATE ${LIBS})
set_property(
  TARGET catch_main PROPERTY MSVC_RUNTIME_LIBRARY
                             "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")
set_cpp_standard(catch_main)
set_warnings(catch_main)

# Create a new unittest, doing the required boilerplate
function(package_add_test testname)
  add_executable(${testname} ${ARGN})
  target_include_directories(${testname} PRIVATE ${INCLUDE} ${LIBS})
  target_link_libraries(${testname} PRIVATE catch_main nowide)
  add_test(NAME ${testname} COMMAND ${testname})
  set_property(
    TARGET ${testname} PROPERTY MSVC_RUNTIME_LIBRARY
                                "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")
  set_cpp_standard(${testname})
  set_warnings(${testname})
  enable_sanitisers(${testname})
endfunction()

package_add_test(chart_unittest chart_unittest.cpp "${SRC}/chart.cpp"
                 "${SRC}/stringutil.cpp")

package_add_test(
  imagebuilder_unittest
  imagebuilder_unittest.cpp
  "${SRC}/imagebuilder.cpp"
  "${SRC}/optimiser.cpp"
  "${SRC}/points.cpp"
  "${SRC}/processed.cpp"
  "${SRC}/songparts.cpp"
  "${SRC}/sp.cpp"
  "${SRC}/stringutil.cpp"
  "${SRC}/time.cpp")
target_include_directories(imagebuilder_unittest PRIVATE ${PNG_INCLUDE_DIRS})
target_link_libraries(imagebuilder_unittest PRIVATE ${PNG_LIBRARIES})

package_add_test(ini_unittest ini_unittest.cpp "${SRC}/ini.cpp"
                 "${SRC}/stringutil.cpp")

package_add_test(midi_unittest midi_unittest.cpp "${SRC}/midi.cpp")

package_add_test(
  optimiser_unittest
  optimiser_unittest.cpp
  "${SRC}/optimiser.cpp"
  "${SRC}/points.cpp"
  "${SRC}/processed.cpp"
  "${SRC}/songparts.cpp"
  "${SRC}/sp.cpp"
  "${SRC}/stringutil.cpp"
  "${SRC}/time.cpp")

package_add_test(
  points_unittest points_unittest.cpp "${SRC}/points.cpp"
  "${SRC}/songparts.cpp" "${SRC}/stringutil.cpp" "${SRC}/time.cpp")

package_add_test(
  processed_unittest
  processed_unittest.cpp
  "${SRC}/points.cpp"
  "${SRC}/processed.cpp"
  "${SRC}/songparts.cpp"
  "${SRC}/sp.cpp"
  "${SRC}/stringutil.cpp"
  "${SRC}/time.cpp")

package_add_test(
  song_unittest
  song_unittest.cpp
  "${SRC}/chart.cpp"
  "${SRC}/ini.cpp"
  "${SRC}/midi.cpp"
  "${SRC}/song.cpp"
  "${SRC}/songparts.cpp"
  "${SRC}/stringutil.cpp")

package_add_test(songparts_unittest songparts_unittest.cpp
                 "${SRC}/songparts.cpp")

package_add_test(sp_unittest sp_unittest.cpp "${SRC}/songparts.cpp"
                 "${SRC}/sp.cpp" "${SRC}/stringutil.cpp" "${SRC}/time.cpp")

package_add_test(stringutil_unittest stringutil_unittest.cpp
                 "${SRC}/stringutil.cpp")

package_add_test(time_unittest time_unittest.cpp "${SRC}/songparts.cpp"
                 "${SRC}/stringutil.cpp" "${SRC}/time.cpp")
