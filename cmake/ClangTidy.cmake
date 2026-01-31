option(UNCOPENER_ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" ON)

if(UNCOPENER_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE clang-tidy)

    if(CLANG_TIDY_EXECUTABLE)
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY_EXECUTABLE}
            -p=${CMAKE_BINARY_DIR}
        )
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXECUTABLE}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
else()
    message(STATUS "clang-tidy disabled")
endif()
