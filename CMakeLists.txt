cmake_minimum_required(VERSION 3.20)
project(UncOpener VERSION 1.0 LANGUAGES CXX)

# Enforce Ninja Multi-Config generator (required for this project)
if(NOT CMAKE_GENERATOR STREQUAL "Ninja Multi-Config")
    message(FATAL_ERROR
        "This project requires the 'Ninja Multi-Config' generator.\n"
        "Please configure with: cmake --preset default\n"
        "Current generator: ${CMAKE_GENERATOR}"
    )
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)

option(UNCOPENER_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

include(cmake/CompilerWarnings.cmake)
include(cmake/ClangFormat.cmake)
include(cmake/ClangTidy.cmake)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
