option(UNCOPENER_ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" ON)

if(UNCOPENER_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE clang-tidy)

    if(CLANG_TIDY_EXECUTABLE)
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY_EXECUTABLE}
            -p=${CMAKE_BINARY_DIR}
            --header-filter=${CMAKE_SOURCE_DIR}/src/.*
        )
        # Skip clang-tidy for auto-generated files (CMake 3.27+)
        if(POLICY CMP0143)
            cmake_policy(SET CMP0143 NEW)
        endif()
        set(CMAKE_CXX_CLANG_TIDY_EXPORT_FIXES_DIR "${CMAKE_BINARY_DIR}/clang-tidy-fixes")
        message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXECUTABLE}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
else()
    message(STATUS "clang-tidy disabled")
endif()

# Function to disable clang-tidy for a target (use for test targets with autogen)
function(target_disable_clang_tidy target)
    set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "")
endfunction()
