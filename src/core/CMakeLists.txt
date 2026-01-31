add_library(uncopener_core STATIC
    UrlParser.cpp
    UrlParser.hpp
)

target_include_directories(uncopener_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(uncopener_core PUBLIC
    Qt6::Core
)

set_project_warnings(uncopener_core)
