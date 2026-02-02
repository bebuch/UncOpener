#include <QApplication>
#include <QTest>

// GUI test main - requires QApplication for widget tests

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    int status = 0;

    // Run dialog tests
    {
        extern int runDialogsTests(int argc, char* argv[]);
        status |= runDialogsTests(argc, argv);
    }

    // Run MainWindow workflow tests
    {
        extern int runMainWindowTests(int argc, char* argv[]);
        status |= runMainWindowTests(argc, argv);
    }

    return status;
}
