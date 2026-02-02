find_package(Qt6 REQUIRED COMPONENTS Test Svg Widgets)

add_executable(uncopener_tests
    TestMain.cpp
    ConfigTests.cpp
    PathOpenerTests.cpp
    PlaceholderTests.cpp
    SchemeRegistryTests.cpp
    SecurityPolicyTests.cpp
    UrlContractTests.cpp
    ResourceTests.cpp
    DialogTests.cpp
    MainWindowTests.cpp
    $<TARGET_OBJECTS:uncopener_app_objects>
)

qt_add_resources(uncopener_tests "app_icons"
    BIG_RESOURCES
    PREFIX "/icons"
    BASE ${CMAKE_SOURCE_DIR}/assets
    FILES ${CMAKE_SOURCE_DIR}/assets/icon.svg
)

target_link_libraries(uncopener_tests PRIVATE
    uncopener_app_objects
    Qt6::Test
)

set_project_warnings(uncopener_tests)

# Disable readability-function-cognitive-complexity for tests
# Qt test macros (QVERIFY, QCOMPARE, etc.) expand to deeply nested if statements
# which artificially inflate the cognitive complexity score
set_target_properties(uncopener_tests PROPERTIES
    CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};-checks=-readability-function-cognitive-complexity"
)

add_test(NAME uncopener_tests COMMAND uncopener_tests)
set_tests_properties(uncopener_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
