#ifndef UNCOPENER_CONFIG_HPP
#define UNCOPENER_CONFIG_HPP

#include "SecurityPolicy.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace uncopener
{

/// Configuration schema for UncOpener
/// Stores all user settings and handles persistence
class Config
{
public:
    /// Default scheme name
    static constexpr const char* DEFAULT_SCHEME_NAME = "uncopener";

    /// Default filetype mode
    static constexpr FiletypeMode DEFAULT_FILETYPE_MODE = FiletypeMode::Whitelist;

    Config() = default;

    /// Get/set the custom URL scheme name
    [[nodiscard]] QString schemeName() const { return m_schemeName; }
    void setSchemeName(const QString& name) { m_schemeName = name; }

    /// Get/set the UNC allow-list
    [[nodiscard]] QStringList uncAllowList() const { return m_uncAllowList; }
    void setUncAllowList(const QStringList& list) { m_uncAllowList = list; }

    /// Get/set the SMB username (Linux only)
    [[nodiscard]] QString smbUsername() const { return m_smbUsername; }
    void setSmbUsername(const QString& username) { m_smbUsername = username; }

    /// Get/set the filetype policy mode
    [[nodiscard]] FiletypeMode filetypeMode() const { return m_filetypeMode; }
    void setFiletypeMode(FiletypeMode mode) { m_filetypeMode = mode; }

    /// Get/set the filetype whitelist
    [[nodiscard]] QStringList filetypeWhitelist() const { return m_filetypeWhitelist; }
    void setFiletypeWhitelist(const QStringList& list) { m_filetypeWhitelist = list; }

    /// Get/set the filetype blacklist
    [[nodiscard]] QStringList filetypeBlacklist() const { return m_filetypeBlacklist; }
    void setFiletypeBlacklist(const QStringList& list) { m_filetypeBlacklist = list; }

    /// Apply this config to a SecurityPolicy
    void applyTo(SecurityPolicy& policy) const;

    /// Serialize to JSON
    [[nodiscard]] QJsonObject toJson() const;

    /// Deserialize from JSON
    /// Returns true if successful, false if invalid JSON
    bool fromJson(const QJsonObject& json);

    /// Reset to default values
    void reset();

    /// Get the default configuration directory path
    [[nodiscard]] static QString configDirPath();

    /// Get the default configuration file path
    [[nodiscard]] static QString configFilePath();

    /// Load config from file
    /// Returns true if loaded successfully, false if file doesn't exist or is invalid
    bool load();

    /// Load config from a specific file path
    bool loadFrom(const QString& filePath);

    /// Save config to file
    /// Returns true if saved successfully
    [[nodiscard]] bool save() const;

    /// Save config to a specific file path
    [[nodiscard]] bool saveTo(const QString& filePath) const;

private:
    QString m_schemeName = DEFAULT_SCHEME_NAME;
    QStringList m_uncAllowList;
    QString m_smbUsername;
    FiletypeMode m_filetypeMode = DEFAULT_FILETYPE_MODE;
    QStringList m_filetypeWhitelist;
    QStringList m_filetypeBlacklist;
};

} // namespace uncopener

#endif // UNCOPENER_CONFIG_HPP
