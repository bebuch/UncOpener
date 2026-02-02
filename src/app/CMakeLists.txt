# Object library for app sources (reused by tests)
add_library(uncopener_app_objects OBJECT
    MainWindow.cpp
    MainWindow.hpp
    ErrorDialog.cpp
    ErrorDialog.hpp
    AppIcon.cpp
    AppIcon.hpp
)

target_include_directories(uncopener_app_objects PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(uncopener_app_objects PUBLIC
    uncopener_core
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
)

set_project_warnings(uncopener_app_objects)

add_executable(uncopener
    main.cpp
    $<TARGET_OBJECTS:uncopener_app_objects>
)

qt_add_resources(uncopener "app_icons"
    BIG_RESOURCES
    PREFIX "/icons"
    BASE ${CMAKE_SOURCE_DIR}/assets
    FILES ${CMAKE_SOURCE_DIR}/assets/icon.svg
)

# Set Windows GUI subsystem
if(WIN32)
    set_target_properties(uncopener PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    # Note: lld-link will warn "found both wWinMain and WinMain" when linking
    # Qt6::EntryPoint - this is harmless as Qt provides both for compatibility
endif()

target_link_libraries(uncopener PRIVATE
    uncopener_app_objects
)

set_project_warnings(uncopener)

# Installation rules for Linux (Wayland icons require .desktop file and installed icon)
if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)

    install(TARGETS uncopener
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(FILES ${CMAKE_SOURCE_DIR}/data/org.uncopener.UncOpener.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

    # Install SVG icon for scalable icons
    install(FILES ${CMAKE_SOURCE_DIR}/assets/icon.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME uncopener.svg
    )
endif()
