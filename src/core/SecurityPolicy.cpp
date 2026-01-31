#include "SecurityPolicy.hpp"

namespace uncopener
{

// UncAllowList implementation

bool UncAllowList::isValidEntry(const QString& entry)
{
    if (entry.trimmed().isEmpty())
    {
        return false;
    }
    // Entries must not contain forward slashes
    return !entry.contains('/');
}

QString UncAllowList::normalizeEntry(const QString& entry)
{
    QString normalized = entry.trimmed();
    // Convert any remaining forward slashes to backslashes (for normalization)
    normalized.replace('/', '\\');
    // Ensure it starts with double backslash
    if (!normalized.startsWith(R"(\\)"))
    {
        if (normalized.startsWith('\\'))
        {
            normalized.prepend('\\');
        }
        else
        {
            normalized.prepend(R"(\\)");
        }
    }
    return normalized;
}

bool UncAllowList::addEntry(const QString& entry)
{
    if (!isValidEntry(entry))
    {
        return false;
    }
    QString normalized = normalizeEntry(entry);
    if (!m_entries.contains(normalized, Qt::CaseInsensitive))
    {
        m_entries.append(normalized);
    }
    return true;
}

QStringList UncAllowList::setEntries(const QStringList& entries)
{
    QStringList rejected;
    m_entries.clear();
    for (const QString& entry : entries)
    {
        if (!addEntry(entry))
        {
            rejected.append(entry);
        }
    }
    return rejected;
}

PolicyCheckResult UncAllowList::check(const QString& uncPath) const
{
    if (m_entries.isEmpty())
    {
        return PolicyCheckResult::deny(
            "No allowed UNC paths configured",
            "Add at least one UNC path prefix to the allow-list in settings.");
    }

    // Normalize the path for comparison
    QString normalizedPath = uncPath;
    normalizedPath.replace('/', '\\');

    // Check against each entry (case-insensitive prefix match)
    for (const QString& entry : m_entries)
    {
        if (normalizedPath.startsWith(entry, Qt::CaseInsensitive))
        {
            return PolicyCheckResult::allow();
        }
    }

    return PolicyCheckResult::deny("Path not in allow-list",
                                   "The UNC path does not match any allowed path prefix. "
                                   "Add an appropriate prefix to the allow-list in settings.");
}

// FiletypePolicy implementation

bool FiletypePolicy::isValidExtension(const QString& extension)
{
    if (extension.trimmed().isEmpty())
    {
        return false;
    }
    // Extensions must not contain path separators
    return !extension.contains('/') && !extension.contains('\\');
}

QString FiletypePolicy::normalizeExtension(const QString& extension)
{
    QString normalized = extension.trimmed().toLower();
    // Ensure it starts with a dot
    if (!normalized.startsWith('.'))
    {
        normalized.prepend('.');
    }
    return normalized;
}

bool FiletypePolicy::addWhitelistEntry(const QString& extension)
{
    if (!isValidExtension(extension))
    {
        return false;
    }
    QString normalized = normalizeExtension(extension);
    if (!m_whitelist.contains(normalized, Qt::CaseInsensitive))
    {
        m_whitelist.append(normalized);
    }
    return true;
}

bool FiletypePolicy::addBlacklistEntry(const QString& extension)
{
    if (!isValidExtension(extension))
    {
        return false;
    }
    QString normalized = normalizeExtension(extension);
    if (!m_blacklist.contains(normalized, Qt::CaseInsensitive))
    {
        m_blacklist.append(normalized);
    }
    return true;
}

QStringList FiletypePolicy::setWhitelist(const QStringList& extensions)
{
    QStringList rejected;
    m_whitelist.clear();
    for (const QString& ext : extensions)
    {
        if (!addWhitelistEntry(ext))
        {
            rejected.append(ext);
        }
    }
    return rejected;
}

QStringList FiletypePolicy::setBlacklist(const QStringList& extensions)
{
    QStringList rejected;
    m_blacklist.clear();
    for (const QString& ext : extensions)
    {
        if (!addBlacklistEntry(ext))
        {
            rejected.append(ext);
        }
    }
    return rejected;
}

PolicyCheckResult FiletypePolicy::check(const QString& filename) const
{
    QString lowercaseFilename = filename.toLower();

    if (m_mode == FiletypeMode::Whitelist)
    {
        // In whitelist mode, empty whitelist allows everything (permissive by default)
        if (m_whitelist.isEmpty())
        {
            return PolicyCheckResult::allow();
        }

        // Check if filename ends with any whitelisted extension
        for (const QString& ext : m_whitelist)
        {
            if (lowercaseFilename.endsWith(ext, Qt::CaseInsensitive))
            {
                return PolicyCheckResult::allow();
            }
        }

        return PolicyCheckResult::deny(
            "File type not in whitelist",
            "This file type is not allowed. Only files with whitelisted extensions can be opened.");
    }

    // Blacklist mode
    if (m_blacklist.isEmpty())
    {
        return PolicyCheckResult::allow();
    }

    // Check if filename ends with any blacklisted extension
    for (const QString& ext : m_blacklist)
    {
        if (lowercaseFilename.endsWith(ext, Qt::CaseInsensitive))
        {
            return PolicyCheckResult::deny(
                "File type is blacklisted",
                "This file type has been blocked. Files with this extension cannot be opened.");
        }
    }

    return PolicyCheckResult::allow();
}

// SecurityPolicy implementation

PolicyCheckResult SecurityPolicy::check(const QString& uncPath) const
{
    // First check the UNC allow-list
    PolicyCheckResult uncResult = m_uncAllowList.check(uncPath);
    if (!uncResult.allowed)
    {
        return uncResult;
    }

    // Then check the filetype policy
    // Extract the filename from the UNC path
    QString filename = uncPath;
    qsizetype lastSlash = uncPath.lastIndexOf('\\');
    if (lastSlash >= 0)
    {
        filename = uncPath.mid(lastSlash + 1);
    }

    // If filename is empty (path ends with slash), skip filetype check
    if (filename.isEmpty())
    {
        return PolicyCheckResult::allow();
    }

    return m_filetypePolicy.check(filename);
}

} // namespace uncopener
