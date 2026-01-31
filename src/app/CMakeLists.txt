add_executable(uncopener WIN32
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
)

target_link_libraries(uncopener PRIVATE
    uncopener_core
    Qt6::Core
    Qt6::Widgets
)

set_project_warnings(uncopener)
