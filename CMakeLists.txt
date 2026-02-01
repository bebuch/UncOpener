cmake_minimum_required(VERSION 3.20)
project(UncOpener VERSION 1.0 LANGUAGES CXX)

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
