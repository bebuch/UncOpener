#include "Config.hpp"
#include "ErrorDialog.hpp"
#include "MainWindow.hpp"
#include "PathOpener.hpp"
#include "SuccessSplash.hpp"

#include <QApplication>

namespace
{

/// Handle URL opening mode (when called with a URL argument)
int runHandlerMode(QApplication& app, const QString& url)
{
    // Load configuration
    uncopener::Config config;
    config.load();

    // Create path opener and attempt to open
    uncopener::PathOpener opener(config);
    uncopener::OpenResult result = opener.open(url);

    if (!result.success)
    {
        // Show error dialog
        ErrorDialog dialog(url, result.errorReason, result.errorRemediation);
        dialog.exec();
        return 1;
    }

    // Success: show splash for 1 second
    SuccessSplash splash;
    QObject::connect(&splash, &SuccessSplash::finished, &app, &QApplication::quit);
    splash.showFor(1000);

    return app.exec();
}

/// Run the configuration GUI mode
int runConfigMode(QApplication& app)
{
    MainWindow window;
    window.show();
    return app.exec();
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("UncOpener");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("bebuch");

    QStringList args = app.arguments();

    // If called with exactly one argument (besides the program name), it's a URL to handle
    if (args.size() == 2)
    {
        return runHandlerMode(app, args.at(1));
    }

    // Otherwise, run the configuration GUI
    return runConfigMode(app);
}
