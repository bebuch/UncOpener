add_executable(uncopener
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    ErrorDialog.cpp
    ErrorDialog.hpp
    AppIcon.cpp
    AppIcon.hpp
    resources.qrc
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
    uncopener_core
    Qt6::Core
    Qt6::Widgets
    Qt6::Svg
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
