#include "Config.hpp"
#include "ErrorDialog.hpp"
#include "MainWindow.hpp"
#include "PathOpener.hpp"

#include <QApplication>
#include <QIcon>
#include <QSystemTrayIcon>

namespace
{

/// Show a desktop notification
void showNotification(const QString& title, const QString& message,
                      QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information)
{
    // QSystemTrayIcon requires an icon to show notifications
    QSystemTrayIcon trayIcon;
    trayIcon.setIcon(QIcon(":/icons/icon.svg"));
    trayIcon.show();
    trayIcon.showMessage(title, message, icon, 3000);
    // Process events to ensure the notification is shown
    QApplication::processEvents();
}

/// Handle URL opening mode (when called with a URL argument)
int runHandlerMode(QApplication& app, const QString& url)
{
    Q_UNUSED(app)

    // Load configuration
    uncopener::Config config;
    config.load();

    // Create path opener and attempt to open
    uncopener::PathOpener opener(config);
    uncopener::OpenResult result = opener.open(url);

    if (!result.success)
    {
        // Show error dialog
        QString displayUrl = url;
        const uncopener::UncPath& parsedPath = opener.lastParsedPath();
        if (!parsedPath.server.isEmpty())
        {
            displayUrl = parsedPath.toUncString();
        }

        ErrorDialog dialog(displayUrl, result.errorReason, result.errorRemediation);
        dialog.exec();
        return 1;
    }

    // Success: show notification and exit
    const uncopener::UncPath& path = opener.lastParsedPath();
    showNotification("UncOpener", "Opening: " + path.toUncString());
    return 0;
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
    app.setStyle("Fusion");
    app.setApplicationName("UncOpener");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("bebuch");

    // Set application-wide icon (applies to all windows and dialogs)
    app.setWindowIcon(QIcon(":/icons/icon.svg"));

    QStringList args = app.arguments();

    // If called with exactly one argument (besides the program name), it's a URL to handle
    if (args.size() == 2)
    {
        return runHandlerMode(app, args.at(1));
    }

    // Otherwise, run the configuration GUI
    return runConfigMode(app);
}
