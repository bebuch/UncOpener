#include "MainWindow.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("UncOpener");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("bebuch");

    MainWindow window;
    window.show();

    return app.exec();
}
