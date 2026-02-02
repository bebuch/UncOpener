#include <QApplication>
#include <QTest>

// Unified test main - uses QApplication for GUI test support

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    int status = 0;

    // Core tests
    {
        extern int runPlaceholderTests(int argc, char* argv[]);
        status |= runPlaceholderTests(argc, argv);
    }

    {
        extern int runUrlContractTests(int argc, char* argv[]);
        status |= runUrlContractTests(argc, argv);
    }

    {
        extern int runSecurityPolicyTests(int argc, char* argv[]);
        status |= runSecurityPolicyTests(argc, argv);
    }

    {
        extern int runConfigTests(int argc, char* argv[]);
        status |= runConfigTests(argc, argv);
    }

    {
        extern int runPathOpenerTests(int argc, char* argv[]);
        status |= runPathOpenerTests(argc, argv);
    }

    {
        extern int runSchemeRegistryTests(int argc, char* argv[]);
        status |= runSchemeRegistryTests(argc, argv);
    }

    // Resource tests
    {
        extern int runResourceTests(int argc, char* argv[]);
        status |= runResourceTests(argc, argv);
    }

    // GUI tests
    {
        extern int runDialogsTests(int argc, char* argv[]);
        status |= runDialogsTests(argc, argv);
    }

    {
        extern int runMainWindowTests(int argc, char* argv[]);
        status |= runMainWindowTests(argc, argv);
    }

    return status;
}
