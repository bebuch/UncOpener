find_package(Qt6 REQUIRED COMPONENTS Test Svg)

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
