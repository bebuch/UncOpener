#ifndef UNCOPENER_SECURITYPOLICY_HPP
#define UNCOPENER_SECURITYPOLICY_HPP

#include <QString>
#include <QStringList>

#include <optional>

namespace uncopener
{

/// Result of a security policy check
struct PolicyCheckResult
{
    bool allowed = false;
    QString reason;      // Why the check failed (empty if allowed)
    QString remediation; // How to fix the issue (empty if allowed)

    static PolicyCheckResult allow() { return {true, {}, {}}; }

    static PolicyCheckResult deny(const QString& reason, const QString& remediation)
    {
        return {false, reason, remediation};
    }
};

/// UNC allow-list policy
/// Checks if a UNC path starts with any entry in the allow-list
class UncAllowList
{
public:
    /// Add an entry to the allow-list
    /// The entry should be in UNC format (e.g., "\\server\share")
    /// Returns false if the entry is invalid (contains forward slashes)
    bool addEntry(const QString& entry);

    /// Set all entries at once (replaces existing entries)
    /// Returns list of invalid entries that were rejected
    QStringList setEntries(const QStringList& entries);

    /// Get all current entries
    QStringList entries() const { return m_entries; }

    /// Clear all entries
    void clear() { m_entries.clear(); }

    /// Check if a UNC path is allowed
    /// The path should be in UNC format (e.g., "\\server\share\path")
    PolicyCheckResult check(const QString& uncPath) const;

    /// Check if an entry is valid (no forward slashes, not empty)
    static bool isValidEntry(const QString& entry);

    /// Normalize an entry (convert forward slashes to backslashes, trim)
    static QString normalizeEntry(const QString& entry);

private:
    QStringList m_entries;
};

/// Filetype policy mode
enum class FiletypeMode
{
    Whitelist, // Only allow listed extensions
    Blacklist  // Deny listed extensions
};

/// Filetype allow/deny policy
class FiletypePolicy
{
public:
    /// Set the policy mode
    void setMode(FiletypeMode mode) { m_mode = mode; }

    /// Get the current mode
    FiletypeMode mode() const { return m_mode; }

    /// Add an extension to the whitelist
    /// Returns false if the extension is invalid (contains path separators)
    bool addWhitelistEntry(const QString& extension);

    /// Add an extension to the blacklist
    /// Returns false if the extension is invalid (contains path separators)
    bool addBlacklistEntry(const QString& extension);

    /// Set all whitelist entries (replaces existing)
    /// Returns list of invalid entries that were rejected
    QStringList setWhitelist(const QStringList& extensions);

    /// Set all blacklist entries (replaces existing)
    /// Returns list of invalid entries that were rejected
    QStringList setBlacklist(const QStringList& extensions);

    /// Get whitelist entries
    QStringList whitelist() const { return m_whitelist; }

    /// Get blacklist entries
    QStringList blacklist() const { return m_blacklist; }

    /// Clear the whitelist
    void clearWhitelist() { m_whitelist.clear(); }

    /// Clear the blacklist
    void clearBlacklist() { m_blacklist.clear(); }

    /// Check if a filename is allowed based on its extension
    /// Uses case-insensitive ends-with comparison
    PolicyCheckResult check(const QString& filename) const;

    /// Check if an extension entry is valid (no path separators)
    static bool isValidExtension(const QString& extension);

    /// Normalize an extension (lowercase, add leading dot if missing)
    static QString normalizeExtension(const QString& extension);

private:
    FiletypeMode m_mode = FiletypeMode::Whitelist;
    QStringList m_whitelist;
    QStringList m_blacklist;
};

/// Combined security policy validator
class SecurityPolicy
{
public:
    /// Access the UNC allow-list
    UncAllowList& uncAllowList() { return m_uncAllowList; }
    const UncAllowList& uncAllowList() const { return m_uncAllowList; }

    /// Access the filetype policy
    FiletypePolicy& filetypePolicy() { return m_filetypePolicy; }
    const FiletypePolicy& filetypePolicy() const { return m_filetypePolicy; }

    /// Run all security checks on a UNC path
    /// Returns the first failed check result, or success if all pass
    PolicyCheckResult check(const QString& uncPath) const;

private:
    UncAllowList m_uncAllowList;
    FiletypePolicy m_filetypePolicy;
};

} // namespace uncopener

#endif // UNCOPENER_SECURITYPOLICY_HPP
