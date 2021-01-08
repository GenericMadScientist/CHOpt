set(INCLUDE "${PROJECT_SOURCE_DIR}/include")
set(LIBS "${PROJECT_SOURCE_DIR}/libs")
set(SRC "${PROJECT_SOURCE_DIR}/src")

add_executable(chart_fuzzer chart_fuzzer.cpp "${SRC}/chart.cpp"
                            "${SRC}/stringutil.cpp")
target_include_directories(chart_fuzzer PRIVATE ${INCLUDE} ${LIBS})
target_compile_options(chart_fuzzer PRIVATE -fsanitize=fuzzer)
target_link_libraries(chart_fuzzer PRIVATE -fsanitize=fuzzer)
set_cpp_standard(chart_fuzzer)
set_warnings(chart_fuzzer)
enable_sanitisers(chart_fuzzer)
