find_package(Qt6 REQUIRED COMPONENTS Test Svg Widgets)

add_executable(uncopener_tests
    test_config.cpp
    test_main.cpp
    test_path_opener.cpp
    test_placeholder.cpp
    test_security_policy.cpp
    test_url_contract.cpp
)

target_link_libraries(uncopener_tests PRIVATE
    uncopener_core
    Qt6::Core
    Qt6::Test
)

set_project_warnings(uncopener_tests)

add_test(NAME uncopener_tests COMMAND uncopener_tests)
set_tests_properties(uncopener_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Separate test for resources (needs Qt6::Svg and the resources.qrc)
add_executable(uncopener_resource_tests
    test_resources.cpp
    ../src/app/resources.qrc
)

target_link_libraries(uncopener_resource_tests PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    Qt6::Test
)

set_project_warnings(uncopener_resource_tests)

add_test(NAME uncopener_resource_tests COMMAND uncopener_resource_tests)
set_tests_properties(uncopener_resource_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# Test for dialog window icons
add_executable(uncopener_dialog_tests
    test_dialogs.cpp
    ../src/app/ErrorDialog.cpp
    ../src/app/ErrorDialog.hpp
    ../src/app/resources.qrc
)

target_include_directories(uncopener_dialog_tests PRIVATE
    ../src/app
)

target_link_libraries(uncopener_dialog_tests PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    Qt6::Widgets
    Qt6::Test
)

set_project_warnings(uncopener_dialog_tests)

add_test(NAME uncopener_dialog_tests COMMAND uncopener_dialog_tests)
set_tests_properties(uncopener_dialog_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
