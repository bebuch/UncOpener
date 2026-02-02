#include <QCoreApplication>
#include <QTest>

// Test classes are registered via QTEST_APPLESS_MAIN in individual test files
// This main simply provides QTest infrastructure

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;

    // Run placeholder tests
    {
        extern int runPlaceholderTests(int argc, char* argv[]);
        status |= runPlaceholderTests(argc, argv);
    }

    // Run URL contract tests
    {
        extern int runUrlContractTests(int argc, char* argv[]);
        status |= runUrlContractTests(argc, argv);
    }

    // Run security policy tests
    {
        extern int runSecurityPolicyTests(int argc, char* argv[]);
        status |= runSecurityPolicyTests(argc, argv);
    }

    // Run config tests
    {
        extern int runConfigTests(int argc, char* argv[]);
        status |= runConfigTests(argc, argv);
    }

    // Run path opener tests
    {
        extern int runPathOpenerTests(int argc, char* argv[]);
        status |= runPathOpenerTests(argc, argv);
    }

    // Run scheme registry tests
    {
        extern int runSchemeRegistryTests(int argc, char* argv[]);
        status |= runSchemeRegistryTests(argc, argv);
    }

    return status;
}
