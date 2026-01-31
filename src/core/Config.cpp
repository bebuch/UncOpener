#include "Config.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

namespace uncopener
{

namespace
{

const QString KEY_SCHEME_NAME = "schemeName";
const QString KEY_UNC_ALLOW_LIST = "uncAllowList";
const QString KEY_SMB_USERNAME = "smbUsername";
const QString KEY_FILETYPE_MODE = "filetypeMode";
const QString KEY_FILETYPE_WHITELIST = "filetypeWhitelist";
const QString KEY_FILETYPE_BLACKLIST = "filetypeBlacklist";

const QString FILETYPE_MODE_WHITELIST = "whitelist";
const QString FILETYPE_MODE_BLACKLIST = "blacklist";

QStringList jsonArrayToStringList(const QJsonArray& array)
{
    QStringList result;
    for (const auto& value : array)
    {
        if (value.isString())
        {
            result.append(value.toString());
        }
    }
    return result;
}

QJsonArray stringListToJsonArray(const QStringList& list)
{
    QJsonArray array;
    for (const QString& item : list)
    {
        array.append(item);
    }
    return array;
}

} // namespace

void Config::applyTo(SecurityPolicy& policy) const
{
    // Apply UNC allow-list
    policy.uncAllowList().setEntries(m_uncAllowList);

    // Apply filetype policy
    policy.filetypePolicy().setMode(m_filetypeMode);
    policy.filetypePolicy().setWhitelist(m_filetypeWhitelist);
    policy.filetypePolicy().setBlacklist(m_filetypeBlacklist);
}

QJsonObject Config::toJson() const
{
    QJsonObject json;

    json[KEY_SCHEME_NAME] = m_schemeName;
    json[KEY_UNC_ALLOW_LIST] = stringListToJsonArray(m_uncAllowList);
    json[KEY_SMB_USERNAME] = m_smbUsername;
    json[KEY_FILETYPE_MODE] = (m_filetypeMode == FiletypeMode::Blacklist) ? FILETYPE_MODE_BLACKLIST
                                                                          : FILETYPE_MODE_WHITELIST;
    json[KEY_FILETYPE_WHITELIST] = stringListToJsonArray(m_filetypeWhitelist);
    json[KEY_FILETYPE_BLACKLIST] = stringListToJsonArray(m_filetypeBlacklist);

    return json;
}

bool Config::fromJson(const QJsonObject& json)
{
    // Scheme name (required, with default)
    if (json.contains(KEY_SCHEME_NAME) && json[KEY_SCHEME_NAME].isString())
    {
        m_schemeName = json[KEY_SCHEME_NAME].toString();
    }
    else
    {
        m_schemeName = DEFAULT_SCHEME_NAME;
    }

    // UNC allow-list (optional)
    if (json.contains(KEY_UNC_ALLOW_LIST) && json[KEY_UNC_ALLOW_LIST].isArray())
    {
        m_uncAllowList = jsonArrayToStringList(json[KEY_UNC_ALLOW_LIST].toArray());
    }
    else
    {
        m_uncAllowList.clear();
    }

    // SMB username (optional)
    if (json.contains(KEY_SMB_USERNAME) && json[KEY_SMB_USERNAME].isString())
    {
        m_smbUsername = json[KEY_SMB_USERNAME].toString();
    }
    else
    {
        m_smbUsername.clear();
    }

    // Filetype mode (optional, with default)
    if (json.contains(KEY_FILETYPE_MODE) && json[KEY_FILETYPE_MODE].isString())
    {
        QString modeStr = json[KEY_FILETYPE_MODE].toString().toLower();
        m_filetypeMode = (modeStr == FILETYPE_MODE_BLACKLIST) ? FiletypeMode::Blacklist
                                                              : FiletypeMode::Whitelist;
    }
    else
    {
        m_filetypeMode = DEFAULT_FILETYPE_MODE;
    }

    // Filetype whitelist (optional)
    if (json.contains(KEY_FILETYPE_WHITELIST) && json[KEY_FILETYPE_WHITELIST].isArray())
    {
        m_filetypeWhitelist = jsonArrayToStringList(json[KEY_FILETYPE_WHITELIST].toArray());
    }
    else
    {
        m_filetypeWhitelist.clear();
    }

    // Filetype blacklist (optional)
    if (json.contains(KEY_FILETYPE_BLACKLIST) && json[KEY_FILETYPE_BLACKLIST].isArray())
    {
        m_filetypeBlacklist = jsonArrayToStringList(json[KEY_FILETYPE_BLACKLIST].toArray());
    }
    else
    {
        m_filetypeBlacklist.clear();
    }

    return true;
}

void Config::reset()
{
    m_schemeName = DEFAULT_SCHEME_NAME;
    m_uncAllowList.clear();
    m_smbUsername.clear();
    m_filetypeMode = DEFAULT_FILETYPE_MODE;
    m_filetypeWhitelist.clear();
    m_filetypeBlacklist.clear();
}

QString Config::configDirPath()
{
#ifdef Q_OS_WIN
    // Windows: %APPDATA%/UncOpener
    QString appData = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return appData + "/UncOpener";
#else
    // Linux: $XDG_CONFIG_HOME/uncopener or ~/.config/uncopener
    QString configHome = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return configHome + "/uncopener";
#endif
}

QString Config::configFilePath()
{
    return configDirPath() + "/config.json";
}

bool Config::load()
{
    return loadFrom(configFilePath());
}

bool Config::loadFrom(const QString& filePath)
{
    QFile file(filePath);
    if (!file.exists())
    {
        // File doesn't exist - use defaults
        reset();
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Can't open file - use defaults
        reset();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError)
    {
        // Invalid JSON - use defaults
        reset();
        return false;
    }

    if (!doc.isObject())
    {
        // Not a JSON object - use defaults
        reset();
        return false;
    }

    return fromJson(doc.object());
}

bool Config::save() const
{
    return saveTo(configFilePath());
}

bool Config::saveTo(const QString& filePath) const
{
    // Ensure the directory exists
    QDir dir = QFileInfo(filePath).dir();
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            return false;
        }
    }

    // Use QSaveFile for atomic save
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QJsonDocument doc(toJson());
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    if (file.write(data) != data.size())
    {
        file.cancelWriting();
        return false;
    }

    return file.commit();
}

} // namespace uncopener
