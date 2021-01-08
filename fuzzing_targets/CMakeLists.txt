set(INCLUDE "${PROJECT_SOURCE_DIR}/include")
set(LIBS "${PROJECT_SOURCE_DIR}/libs")
set(SRC "${PROJECT_SOURCE_DIR}/src")

# Adds a new fuzz target
function(add_fuzz_target testname)
  add_executable(${testname} ${ARGN})
  target_include_directories(${testname} PRIVATE ${INCLUDE} ${LIBS})
  target_compile_options(${testname} PRIVATE -fsanitize=fuzzer)
  target_link_libraries(${testname} PRIVATE -fsanitize=fuzzer)
  set_cpp_standard(${testname})
  set_warnings(${testname})
  enable_sanitisers(${testname})
endfunction()

add_fuzz_target(chart_fuzzer chart_fuzzer.cpp "${SRC}/chart.cpp"
                "${SRC}/stringutil.cpp")

add_fuzz_target(midi_fuzzer midi_fuzzer.cpp "${SRC}/midi.cpp")
