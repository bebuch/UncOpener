#include "SchemeRegistry.hpp"

#include <QCoreApplication>
#include <QTest>

using namespace uncopener;

class SchemeRegistryTest : public QObject
{
    Q_OBJECT

private slots:
    void testCreateReturnsValidInstance()
    {
        auto registry = SchemeRegistry::create();
        QVERIFY(registry != nullptr);
    }

    void testCurrentBinaryPathNotEmpty()
    {
        // Note: QCoreApplication must exist for applicationFilePath() to work
        // This is tested via the test runner which provides QCoreApplication
        if (QCoreApplication::instance() == nullptr)
        {
            QSKIP("QCoreApplication not available", SkipSingle);
        }
        QString path = SchemeRegistry::currentBinaryPath();
        QVERIFY(!path.isEmpty());
    }

    void testCurrentBinaryPathIsAbsolute()
    {
        if (QCoreApplication::instance() == nullptr)
        {
            QSKIP("QCoreApplication not available", SkipSingle);
        }
        QString path = SchemeRegistry::currentBinaryPath();
        // On Windows, absolute paths start with drive letter (e.g., C:)
        // On Linux, they start with /
#ifdef Q_OS_WIN
        QVERIFY(path.length() >= 2 && path.at(1) == ':');
#else
        QVERIFY(path.startsWith('/'));
#endif
    }

    void testCheckRegistrationUnknownScheme()
    {
        auto registry = SchemeRegistry::create();
        // Use a very unlikely scheme name that should not be registered
        auto status = registry->checkRegistration("uncopener_test_nonexistent_xyz123");
        QCOMPARE(status, RegistrationStatus::NotRegistered);
    }

    void testGetRegisteredBinaryPathUnknownScheme()
    {
        auto registry = SchemeRegistry::create();
        // Use a very unlikely scheme name that should not be registered
        QString path = registry->getRegisteredBinaryPath("uncopener_test_nonexistent_xyz123");
        QVERIFY(path.isEmpty());
    }

    void testRegistrationResultOk()
    {
        auto result = RegistrationResult::ok();
        QVERIFY(result.success);
        QVERIFY(result.errorMessage.isEmpty());
    }

    void testRegistrationResultError()
    {
        auto result = RegistrationResult::error("Test error message");
        QVERIFY(!result.success);
        QCOMPARE(result.errorMessage, "Test error message");
    }

    void testRegistrationStatusEnumValues()
    {
        // Verify enum values exist and are distinct
        QVERIFY(RegistrationStatus::NotRegistered != RegistrationStatus::RegisteredToThisBinary);
        QVERIFY(RegistrationStatus::NotRegistered != RegistrationStatus::RegisteredToOtherBinary);
        QVERIFY(RegistrationStatus::RegisteredToThisBinary !=
                RegistrationStatus::RegisteredToOtherBinary);
    }

    // Integration test: register and unregister a test scheme
    // This test actually modifies system state (registry on Windows, .desktop files on Linux)
    // so we use a unique test scheme name
    void testRegisterAndUnregisterScheme()
    {
        auto registry = SchemeRegistry::create();
        const QString testScheme = "uncopener_integration_test_scheme";

        // Ensure clean state - unregister if somehow left over from previous test
        (void)registry->unregisterScheme(testScheme);

        // Should start as not registered
        QCOMPARE(registry->checkRegistration(testScheme), RegistrationStatus::NotRegistered);

        // Register the scheme
        auto registerResult = registry->registerScheme(testScheme);
        if (!registerResult.success)
        {
            // On some systems (CI, sandboxed), registration may fail due to permissions
            QSKIP("Cannot test registration - insufficient permissions", SkipSingle);
        }

        // Should now be registered to this binary
        QCOMPARE(registry->checkRegistration(testScheme),
                 RegistrationStatus::RegisteredToThisBinary);

        // Registered binary path should match current binary
        QString registeredPath = registry->getRegisteredBinaryPath(testScheme);
        QCOMPARE(registeredPath.toLower(), SchemeRegistry::currentBinaryPath().toLower());

        // Unregister the scheme
        auto unregisterResult = registry->unregisterScheme(testScheme);
        QVERIFY(unregisterResult.success);

        // Should be back to not registered
        QCOMPARE(registry->checkRegistration(testScheme), RegistrationStatus::NotRegistered);
    }
};

int runSchemeRegistryTests(int argc, char* argv[])
{
    SchemeRegistryTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_scheme_registry.moc"
