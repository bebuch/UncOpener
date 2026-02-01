#include <QtGlobal>
#ifdef Q_OS_WIN

#include "SchemeRegistry.hpp"

#include <QCoreApplication>
#include <QSettings>

namespace uncopener
{

namespace
{

/// Registry path for HKCU scheme registration
QString registryPath(const QString& schemeName)
{
    return R"(HKEY_CURRENT_USER\Software\Classes\)" + schemeName;
}

} // namespace

/// Windows implementation using HKCU registry
class WindowsSchemeRegistry : public SchemeRegistry
{
public:
    [[nodiscard]] RegistrationStatus checkRegistration(const QString& schemeName) const override
    {
        QString registeredPath = getRegisteredBinaryPath(schemeName);

        if (registeredPath.isEmpty())
        {
            return RegistrationStatus::NotRegistered;
        }

        // Normalize paths for comparison (case-insensitive on Windows)
        QString currentPath = currentBinaryPath();
        if (registeredPath.compare(currentPath, Qt::CaseInsensitive) == 0)
        {
            return RegistrationStatus::RegisteredToThisBinary;
        }

        return RegistrationStatus::RegisteredToOtherBinary;
    }

    [[nodiscard]] QString getRegisteredBinaryPath(const QString& schemeName) const override
    {
        QSettings registry(registryPath(schemeName) + R"(\shell\open\command)",
                           QSettings::NativeFormat);

        QString command = registry.value("Default").toString();
        if (command.isEmpty())
        {
            return {};
        }

        // Extract binary path from command (format: "path\to\binary.exe" "%1")
        // Handle quoted path
        if (command.startsWith('"'))
        {
            qsizetype endQuote = command.indexOf('"', 1);
            if (endQuote > 0)
            {
                return command.mid(1, endQuote - 1);
            }
        }

        // Handle unquoted path
        qsizetype spacePos = command.indexOf(' ');
        if (spacePos > 0)
        {
            return command.left(spacePos);
        }

        return command;
    }

    [[nodiscard]] RegistrationResult registerScheme(const QString& schemeName) override
    {
        QString basePath = registryPath(schemeName);
        QString binaryPath = currentBinaryPath();

        // Create the base key with URL Protocol marker
        QSettings baseKey(basePath, QSettings::NativeFormat);
        baseKey.setValue("Default", "URL:" + schemeName + " Protocol");
        baseKey.setValue("URL Protocol", "");

        // Create the command key
        QSettings commandKey(basePath + R"(\shell\open\command)", QSettings::NativeFormat);
        QString command = "\"" + binaryPath + R"(" "%1")";
        commandKey.setValue("Default", command);

        // Create the icon key
        QSettings iconKey(basePath + "\\DefaultIcon", QSettings::NativeFormat);
        iconKey.setValue("Default", binaryPath + ",0");

        // Verify registration succeeded
        if (checkRegistration(schemeName) != RegistrationStatus::RegisteredToThisBinary)
        {
            return RegistrationResult::error(
                "Failed to write to registry. Try running as administrator or check permissions.");
        }

        return RegistrationResult::ok();
    }

    [[nodiscard]] RegistrationResult unregisterScheme(const QString& schemeName) override
    {
        QString basePath = registryPath(schemeName);

        // Delete the entire key tree
        QSettings registry("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
        registry.remove(schemeName);

        // Verify unregistration succeeded
        if (checkRegistration(schemeName) != RegistrationStatus::NotRegistered)
        {
            return RegistrationResult::error("Failed to remove registry keys. Try running as "
                                             "administrator or check permissions.");
        }

        return RegistrationResult::ok();
    }
};

std::unique_ptr<SchemeRegistry> SchemeRegistry::create()
{
    return std::make_unique<WindowsSchemeRegistry>();
}

QString SchemeRegistry::currentBinaryPath()
{
    return QCoreApplication::applicationFilePath();
}

} // namespace uncopener

#endif // Q_OS_WIN
