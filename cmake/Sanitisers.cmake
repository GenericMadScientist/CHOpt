# Enable the selected sanitisers; only supports GCC and Clang
function(enable_sanitisers target)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                             ".*Clang")
    set(sanitisers "")

    option(ENABLE_SANITISER_ADDRESS "Enable address sanitiser" OFF)
    if(ENABLE_SANITISER_ADDRESS)
      list(APPEND sanitisers "address")
    endif()

    option(ENABLE_SANITISER_LEAK "Enable leak sanitiser" OFF)
    if(ENABLE_SANITISER_LEAK)
      list(APPEND sanitisers "leak")
    endif()

    option(ENABLE_SANITISER_UNDEFINED_BEHAVIOR
           "Enable undefined behavior sanitiser" OFF)
    if(ENABLE_SANITISER_UNDEFINED_BEHAVIOR)
      list(APPEND sanitisers "undefined")
    endif()

    option(ENABLE_SANITISER_THREAD "Enable thread sanitiser" OFF)
    if(ENABLE_SANITISER_THREAD)
      if("address" IN_LIST sanitisers OR "leak" IN_LIST sanitisers)
        message(WARNING "TSan does not work with ASan or LSan enabled")
      else()
        list(APPEND sanitisers "thread")
      endif()
    endif()

    option(ENABLE_SANITISER_MEMORY "Enable memory sanitiser" OFF)
    if(ENABLE_SANITISER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      if("address" IN_LIST sanitisers
         OR "thread" IN_LIST sanitisers
         OR "leak" IN_LIST sanitisers)
        message(WARNING "MSan does not work with ASan, LSan, or TSan enabled")
      else()
        list(APPEND sanitisers "memory")
      endif()
    endif()

    list(JOIN sanitisers "," list_of_sanitisers)
  endif()

  if(list_of_sanitisers)
    if(NOT "${list_of_sanitisers}" STREQUAL "")
      target_compile_options(${target} PRIVATE -fsanitize=${list_of_sanitisers})
      target_link_libraries(${target} PRIVATE -fsanitize=${list_of_sanitisers})
    endif()
  endif()
endfunction()
