add_library(uncopener_core STATIC
    Config.cpp
    Config.hpp
    PathOpener.cpp
    PathOpener.hpp
    SecurityPolicy.cpp
    SecurityPolicy.hpp
    UrlParser.cpp
    UrlParser.hpp
)

target_include_directories(uncopener_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(uncopener_core PUBLIC
    Qt6::Core
    Qt6::Gui
)

set_project_warnings(uncopener_core)
