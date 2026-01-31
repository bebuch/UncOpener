#ifndef Q_OS_WIN

#include "SchemeRegistry.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

namespace uncopener
{

namespace
{

/// Get the path to the .desktop file for a scheme
QString desktopFilePath(const QString& schemeName)
{
    QString applicationsDir =
        QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    return applicationsDir + "/uncopener-" + schemeName + ".desktop";
}

/// Get the MIME type for a URL scheme
QString mimeTypeForScheme(const QString& schemeName)
{
    return "x-scheme-handler/" + schemeName;
}

/// Generate .desktop file content
QString generateDesktopFileContent(const QString& schemeName, const QString& binaryPath)
{
    QString content;
    QTextStream stream(&content);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=UncOpener (" << schemeName << ")\n";
    stream << "Comment=Handler for " << schemeName << ":// URLs\n";
    stream << "Exec=\"" << binaryPath << "\" %u\n";
    stream << "Terminal=false\n";
    stream << "NoDisplay=true\n";
    stream << "MimeType=" << mimeTypeForScheme(schemeName) << ";\n";
    return content;
}

/// Parse Exec line from .desktop file to extract binary path
QString extractBinaryFromExecLine(const QString& execLine)
{
    // Handle quoted path: Exec="path/to/binary" %u
    static QRegularExpression quotedPattern(R"re(Exec="([^"]+)")re");
    QRegularExpressionMatch quotedMatch = quotedPattern.match(execLine);
    if (quotedMatch.hasMatch())
    {
        return quotedMatch.captured(1);
    }

    // Handle unquoted path: Exec=path/to/binary %u
    QString line = execLine;
    if (line.startsWith("Exec="))
    {
        line = line.mid(5);
    }
    qsizetype spacePos = line.indexOf(' ');
    if (spacePos > 0)
    {
        return line.left(spacePos);
    }
    return line;
}

} // namespace

/// Linux implementation using .desktop files and xdg-mime
class LinuxSchemeRegistry : public SchemeRegistry
{
public:
    RegistrationStatus checkRegistration(const QString& schemeName) const override
    {
        QString registeredPath = getRegisteredBinaryPath(schemeName);

        if (registeredPath.isEmpty())
        {
            return RegistrationStatus::NotRegistered;
        }

        // Normalize paths for comparison
        QString currentPath = currentBinaryPath();
        if (registeredPath == currentPath)
        {
            return RegistrationStatus::RegisteredToThisBinary;
        }

        return RegistrationStatus::RegisteredToOtherBinary;
    }

    QString getRegisteredBinaryPath(const QString& schemeName) const override
    {
        QString desktopPath = desktopFilePath(schemeName);
        QFile file(desktopPath);

        if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return {};
        }

        // Read the file and find the Exec line
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            if (line.startsWith("Exec="))
            {
                return extractBinaryFromExecLine(line);
            }
        }

        return {};
    }

    RegistrationResult registerScheme(const QString& schemeName) override
    {
        QString desktopPath = desktopFilePath(schemeName);
        QString binaryPath = currentBinaryPath();

        // Ensure the applications directory exists
        QDir applicationsDir(
            QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation));
        if (!applicationsDir.exists())
        {
            if (!applicationsDir.mkpath("."))
            {
                return RegistrationResult::error("Failed to create applications directory: " +
                                                 applicationsDir.path());
            }
        }

        // Write the .desktop file
        QFile file(desktopPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            return RegistrationResult::error("Failed to create .desktop file: " + desktopPath);
        }

        QString content = generateDesktopFileContent(schemeName, binaryPath);
        QTextStream stream(&file);
        stream << content;
        file.close();

        // Make the desktop file executable
        file.setPermissions(file.permissions() | QFileDevice::ExeUser);

        // Update the MIME database to register the scheme handler
        QString mimeType = mimeTypeForScheme(schemeName);
        QString desktopFileName = "uncopener-" + schemeName + ".desktop";

        QProcess updateMime;
        updateMime.start("xdg-mime", {"default", desktopFileName, mimeType});
        if (!updateMime.waitForFinished(5000))
        {
            return RegistrationResult::error("xdg-mime command timed out");
        }

        if (updateMime.exitCode() != 0)
        {
            QString errorOutput = QString::fromUtf8(updateMime.readAllStandardError());
            return RegistrationResult::error("xdg-mime failed: " + errorOutput);
        }

        // Update the desktop database
        QProcess updateDb;
        updateDb.start("update-desktop-database",
                       {QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)});
        updateDb.waitForFinished(5000); // Ignore errors, this is optional

        return RegistrationResult::ok();
    }

    RegistrationResult unregisterScheme(const QString& schemeName) override
    {
        QString desktopPath = desktopFilePath(schemeName);

        // Remove the .desktop file
        QFile file(desktopPath);
        if (file.exists())
        {
            if (!file.remove())
            {
                return RegistrationResult::error("Failed to remove .desktop file: " + desktopPath);
            }
        }

        // Update the desktop database
        QProcess updateDb;
        updateDb.start("update-desktop-database",
                       {QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)});
        updateDb.waitForFinished(5000); // Ignore errors

        // Note: We don't need to explicitly remove the xdg-mime association
        // because removing the .desktop file effectively unregisters it

        return RegistrationResult::ok();
    }
};

std::unique_ptr<SchemeRegistry> SchemeRegistry::create()
{
    return std::make_unique<LinuxSchemeRegistry>();
}

QString SchemeRegistry::currentBinaryPath()
{
    return QCoreApplication::applicationFilePath();
}

} // namespace uncopener

#endif // Q_OS_WIN
