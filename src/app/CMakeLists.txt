add_executable(uncopener WIN32
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    ErrorDialog.cpp
    ErrorDialog.hpp
    resources.qrc
)

target_link_libraries(uncopener PRIVATE
    uncopener_core
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
)

set_project_warnings(uncopener)
