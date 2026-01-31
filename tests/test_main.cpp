#include <QTest>

// Test classes are registered via QTEST_APPLESS_MAIN in individual test files
// This main simply provides QTest infrastructure

int main(int argc, char* argv[])
{
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

    return status;
}
