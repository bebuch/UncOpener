find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(uncopener_tests
    test_config.cpp
    test_main.cpp
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
