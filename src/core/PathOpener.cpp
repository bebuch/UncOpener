#include "PathOpener.hpp"

#include <QDesktopServices>
#include <QUrl>

namespace uncopener
{

PathOpener::PathOpener(const Config& config) : m_config(config), m_parser(config.schemeName())
{
    config.applyTo(m_policy);
}

QString PathOpener::buildTargetUrl(const UncPath& path) const
{
#ifdef Q_OS_WIN
    // Windows: use UNC path directly
    return path.toUncString();
#else
    // Linux: build SMB URL
    return path.toSmbUrl(m_config.smbUsername());
#endif
}

bool PathOpener::openUrl(const QString& url)
{
#ifdef Q_OS_WIN
    // On Windows, convert UNC path to file:// URL for QDesktopServices
    // QDesktopServices::openUrl works better with file:// URLs
    QString fileUrl = "file:" + url;
    return QDesktopServices::openUrl(QUrl(fileUrl, QUrl::TolerantMode));
#else
    // On Linux, use the SMB URL directly
    return QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
#endif
}

OpenResult PathOpener::validate(const QString& url) const
{
    // Parse the URL
    ParseResult parseResult = m_parser.parse(url);
    if (isError(parseResult))
    {
        return OpenResult::fromParseError(getError(parseResult));
    }

    m_lastPath = getPath(parseResult);

    // Check against UNC allow-list (always check against UNC form)
    QString uncPath = m_lastPath.toUncString();
    PolicyCheckResult uncResult = m_policy.uncAllowList().check(uncPath);
    if (!uncResult.allowed)
    {
        return OpenResult::fromPolicyResult(uncResult);
    }

    // Check filetype policy
    PolicyCheckResult filetypeResult = m_policy.check(uncPath);
    if (!filetypeResult.allowed)
    {
        return OpenResult::fromPolicyResult(filetypeResult);
    }

    return OpenResult::ok();
}

QString PathOpener::getTargetPath(const QString& url) const
{
    OpenResult result = validate(url);
    if (!result.success)
    {
        return {};
    }
    return buildTargetUrl(m_lastPath);
}

OpenResult PathOpener::open(const QString& url)
{
    // First validate
    OpenResult validationResult = validate(url);
    if (!validationResult.success)
    {
        return validationResult;
    }

    // Build the target URL for this platform
    QString targetUrl = buildTargetUrl(m_lastPath);

    // Attempt to open
    if (!openUrl(targetUrl))
    {
        return OpenResult::error(
            "Failed to open path",
            "The system could not open the path. Make sure you have access to the network "
            "location and a suitable application is configured to handle it.");
    }

    return OpenResult::ok();
}

} // namespace uncopener
