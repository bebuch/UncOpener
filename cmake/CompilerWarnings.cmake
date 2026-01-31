function(set_project_warnings target_name)
    set(CLANG_GCC_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wunused
    )

    set(GCC_WARNINGS
        ${CLANG_GCC_WARNINGS}
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
    )

    set(CLANG_WARNINGS
        ${CLANG_GCC_WARNINGS}
    )

    set(MSVC_WARNINGS
        /W4
        /permissive-
        /w14242  # conversion from 'type1' to 'type2', possible loss of data
        /w14254  # operator: conversion from 'type1' to 'type2', possible loss of data
        /w14263  # member function does not override any base class virtual member function
        /w14265  # class has virtual functions, but destructor is not virtual
        /w14287  # unsigned/negative constant mismatch
        /w14296  # expression is always false
        /w14311  # pointer truncation from 'type' to 'type'
        /w14545  # expression before comma evaluates to a function which is missing an argument list
        /w14546  # function call before comma missing argument list
        /w14547  # operator before comma has no effect
        /w14549  # operator before comma has no effect
        /w14555  # expression has no effect
        /w14619  # pragma warning: there is no warning number 'number'
        /w14640  # thread un-safe static member initialization
        /w14826  # conversion from 'type1' to 'type2' is sign-extended
        /w14905  # wide string literal cast to 'LPSTR'
        /w14906  # string literal cast to 'LPWSTR'
        /w14928  # illegal copy-initialization
    )

    if(UNCOPENER_WARNINGS_AS_ERRORS)
        list(APPEND CLANG_WARNINGS -Werror)
        list(APPEND GCC_WARNINGS -Werror)
        list(APPEND MSVC_WARNINGS /WX)
    endif()

    if(MSVC)
        set(PROJECT_WARNINGS ${MSVC_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PROJECT_WARNINGS ${GCC_WARNINGS})
    else()
        message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}, using Clang warnings")
        set(PROJECT_WARNINGS ${CLANG_WARNINGS})
    endif()

    target_compile_options(${target_name} PRIVATE ${PROJECT_WARNINGS})
endfunction()
