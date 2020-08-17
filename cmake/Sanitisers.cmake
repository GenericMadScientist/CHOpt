function(enable_sanitisers TARGET)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(SANITISERS "")

        option(ENABLE_SANITISER_ADDRESS "Enable address sanitiser" OFF)
        if(ENABLE_SANITISER_ADDRESS)
            list(APPEND SANITISERS "address")
        endif()

        option(ENABLE_SANITISER_LEAK "Enable leak sanitiser" OFF)
        if(ENABLE_SANITISER_LEAK)
            list(APPEND SANITISERS "leak")
        endif()

        option(ENABLE_SANITISER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitiser" OFF)
        if(ENABLE_SANITISER_UNDEFINED_BEHAVIOR)
            list(APPEND SANITISERS "undefined")
        endif()

        option(ENABLE_SANITISER_THREAD "Enable thread sanitiser" OFF)
        if(ENABLE_SANITISER_THREAD)
            if("address" IN_LIST SANITISERS OR "leak" IN_LIST SANITISERS)
                message(WARNING "Thread sanitiser does not work with Address or Leak sanitiser enabled")
            else()
                list(APPEND SANITISERS "thread")
            endif()
        endif()

        option(ENABLE_SANITISER_MEMORY "Enable memory sanitiser" OFF)
        if(ENABLE_SANITISER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            if("address" IN_LIST SANITISERS OR "thread" IN_LIST SANITISERS OR "leak" IN_LIST SANITISERS)
                message(WARNING "Memory sanitiser does not work with Address, Thread, or Leak sanitiser enabled")
            else()
                list(APPEND SANITISERS "memory")
            endif()
        endif()

        list(
            JOIN
            SANITISERS
            ","
            LIST_OF_SANITISERS)
    endif()

    if(LIST_OF_SANITISERS)
        if(NOT "${LIST_OF_SANITISERS}" STREQUAL "")
            target_compile_options(${TARGET} PRIVATE -fsanitize=${LIST_OF_SANITISERS})
            target_link_libraries(${TARGET} PRIVATE -fsanitize=${LIST_OF_SANITISERS})
        endif()
    endif()
endfunction()
