#include "UrlParser.hpp"

#include <QByteArray>
#include <QStringList>
#include <QUrl>

#include <utility>

namespace uncopener
{

QString UncPath::toUncString() const
{
    QString result = R"(\\)" + server;
    if (!path.isEmpty())
    {
        result += R"(\)" + path;
    }
    if (hasTrailingSlash && !result.endsWith('\\'))
    {
        result += '\\';
    }
    return result;
}

QString UncPath::toSmbUrl(const QString& username) const
{
    QString result = "smb://";
    if (!username.isEmpty())
    {
        // Percent-encode the username (especially for DOMAIN\user format)
        QString encodedUsername = QString::fromUtf8(QUrl::toPercentEncoding(username));
        result += encodedUsername + "@";
    }
    result += server;
    if (!path.isEmpty())
    {
        // Convert backslashes to forward slashes for SMB URLs
        QString smbPath = path;
        smbPath.replace('\\', '/');
        result += "/" + smbPath;
    }
    if (hasTrailingSlash && !result.endsWith('/'))
    {
        result += '/';
    }
    return result;
}

ParseError ParseError::create(Code code, const QString& input, const QString& expectedScheme,
                              const QString& foundScheme)
{
    ParseError error;
    error.code = code;
    error.input = input;

    QString scheme = expectedScheme.isEmpty() ? "uncopener" : expectedScheme;

    switch (code)
    {
    case Code::EmptyInput:
        error.reason = "Empty input provided";
        error.remediation = "Please provide a valid URL starting with the configured scheme.";
        break;
    case Code::MissingScheme:
        error.reason = "No URL scheme found in input";
        error.remediation = QString("The URL must start with a scheme followed by '://' (e.g., "
                                    "'%1://').")
                                .arg(scheme);
        break;
    case Code::WrongScheme:
        if (!foundScheme.isEmpty())
        {
            error.reason = QString("URL has incorrect scheme \"%1\"").arg(foundScheme);
        }
        else
        {
            error.reason = "URL has incorrect scheme";
        }

        if (!expectedScheme.isEmpty())
        {
            error.remediation =
                QString("The URL must use the configured scheme \"%1\".").arg(expectedScheme);
        }
        else
        {
            error.remediation = "The URL must use the configured scheme.";
        }
        break;
    case Code::InvalidSchemeFormat:
        error.reason = "Invalid URL format - expected '://' after scheme";
        error.remediation =
            QString("Use double slash after the scheme (e.g., '%1://server/path').").arg(scheme);
        break;
    case Code::MissingAuthority:
        error.reason = "No server name found in URL";
        error.remediation =
            QString("The URL must include a server name (e.g., '%1://server/path').").arg(scheme);
        break;
    case Code::WhitespaceAuthority:
        error.reason = "Server name contains only whitespace";
        error.remediation = "Provide a valid server name without leading/trailing spaces.";
        break;
    case Code::DirectoryTraversal:
        error.reason = "Directory traversal detected (..)";
        error.remediation =
            "For security, paths containing '..' are not allowed. Use absolute paths instead.";
        break;
    case Code::InvalidCharacter:
        error.reason = "Invalid character in URL";
        error.remediation = "The URL contains characters that are not allowed.";
        break;
    }

    return error;
}

UrlParser::UrlParser(QString schemeName) : m_schemeName(std::move(schemeName)) {}

QString UrlParser::percentDecode(const QString& input)
{
    return QUrl::fromPercentEncoding(input.toUtf8());
}

std::optional<QString> UrlParser::normalizePath(const QString& path, bool& hasTrailingSlash)
{
    if (path.isEmpty())
    {
        // Don't modify hasTrailingSlash - caller has already determined it
        return QString();
    }

    // Check for trailing slash before normalization
    hasTrailingSlash = path.endsWith('/') || path.endsWith('\\');

    // Split the path into segments
    // Replace backslashes with forward slashes first for uniform processing
    QString normalizedPath = path;
    normalizedPath.replace('\\', '/');

    QStringList segments = normalizedPath.split('/', Qt::SkipEmptyParts);
    QStringList resultSegments;

    for (const QString& segment : segments)
    {
        if (segment == "..")
        {
            // Directory traversal detected - reject
            return std::nullopt;
        }
        if (segment == ".")
        {
            // Skip single-dot segments
            continue;
        }
        resultSegments.append(segment);
    }

    // Convert back to backslash-separated path for UNC
    return resultSegments.join('\\');
}

std::optional<ParseError> UrlParser::checkScheme(const QString& input) const
{
    QString schemePrefix = m_schemeName + "://";
    QString singleSlashPrefix = m_schemeName + ":/";

    // Check for single slash format (invalid)
    if (input.startsWith(singleSlashPrefix) && !input.startsWith(schemePrefix))
    {
        return ParseError::create(ParseError::Code::InvalidSchemeFormat, input, m_schemeName);
    }

    // Check scheme
    if (!input.startsWith(schemePrefix))
    {
        // Check if there's a different scheme
        qsizetype colonPos = input.indexOf(':');
        if (colonPos > 0)
        {
            // Ensure colon is before any path separator causing it to look like a scheme
            qsizetype slashPos = input.indexOf('/');
            if (slashPos < 0 || colonPos < slashPos)
            {
                QString foundScheme = input.left(colonPos);
                return ParseError::create(ParseError::Code::WrongScheme, input, m_schemeName,
                                          foundScheme);
            }
        }
        return ParseError::create(ParseError::Code::MissingScheme, input, m_schemeName);
    }

    return std::nullopt;
}

QString UrlParser::stripQueryAndFragment(const QString& input)
{
    qsizetype queryPos = input.indexOf('?');
    qsizetype fragmentPos = input.indexOf('#');

    qsizetype cutPos = -1;
    if (queryPos >= 0 && fragmentPos >= 0)
    {
        cutPos = qMin(queryPos, fragmentPos);
    }
    else if (queryPos >= 0)
    {
        cutPos = queryPos;
    }
    else if (fragmentPos >= 0)
    {
        cutPos = fragmentPos;
    }

    if (cutPos >= 0)
    {
        return input.left(cutPos);
    }
    return input;
}

ParseResult UrlParser::parse(const QString& input) const
{
    if (input.isEmpty())
    {
        return ParseError::create(ParseError::Code::EmptyInput, input);
    }

    if (auto error = checkScheme(input))
    {
        return *error;
    }

    // Expected format: scheme://server/share/path
    QString schemePrefix = m_schemeName + "://";

    // Extract the part after scheme://
    QString remainder = input.mid(schemePrefix.length());

    // Remove query string and fragment (they are ignored per the contract)
    remainder = stripQueryAndFragment(remainder);

    // Split by forward slash to get authority and path
    qsizetype firstSlash = remainder.indexOf('/');

    QString authority;
    QString pathPart;

    if (firstSlash < 0)
    {
        // No path at all, just authority (and possibly no share)
        authority = remainder;
    }
    else
    {
        authority = remainder.left(firstSlash);
        pathPart = remainder.mid(firstSlash + 1);
    }

    // Check for empty or whitespace-only authority
    if (authority.isEmpty())
    {
        return ParseError::create(ParseError::Code::MissingAuthority, input, m_schemeName);
    }
    if (authority.trimmed().isEmpty())
    {
        return ParseError::create(ParseError::Code::WhitespaceAuthority, input, m_schemeName);
    }

    // Percent-decode the authority (server name)
    QString server = percentDecode(authority);

    // Everything else is the path
    QString rawPath = pathPart;
    bool hasTrailingSlash = false;

    // Check if there's a trailing slash
    // Special case: if remainder ends with '/' but pathPart is empty, we still have a trailing
    // slash (e.g., "server/" means the URL was "scheme://server/")
    if (!rawPath.isEmpty())
    {
        hasTrailingSlash = rawPath.endsWith('/') || rawPath.endsWith('\\');
    }
    else if (firstSlash >= 0 && remainder.endsWith('/'))
    {
        // URL like "scheme://server/" - path is empty but there was a trailing slash
        hasTrailingSlash = true;
    }

    // Normalize the path
    auto normalizedPath = normalizePath(rawPath, hasTrailingSlash);
    if (!normalizedPath.has_value())
    {
        return ParseError::create(ParseError::Code::DirectoryTraversal, input);
    }

    // Build the result
    UncPath result;
    result.server = server;
    // Decode logical path segments (normalizePath returns backslash-separated components)
    // We want the path part of the UNC string to be properly decoded.
    result.path = percentDecode(normalizedPath.value());
    result.hasTrailingSlash = hasTrailingSlash;

    return result;
}

} // namespace uncopener
