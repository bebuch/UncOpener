#ifndef UNCOPENER_PATHOPENER_HPP
#define UNCOPENER_PATHOPENER_HPP

#include "Config.hpp"
#include "SecurityPolicy.hpp"
#include "UrlParser.hpp"

#include <QString>

namespace uncopener
{

/// Result of attempting to open a path
struct OpenResult
{
    bool success = false;
    QString errorReason;
    QString errorRemediation;

    [[nodiscard]] static OpenResult ok() { return {true, {}, {}}; }

    [[nodiscard]] static OpenResult error(const QString& reason, const QString& remediation)
    {
        return {false, reason, remediation};
    }

    [[nodiscard]] static OpenResult fromParseError(const ParseError& error)
    {
        return {false, error.reason, error.remediation};
    }

    [[nodiscard]] static OpenResult fromPolicyResult(const PolicyCheckResult& result)
    {
        return {false, result.reason, result.remediation};
    }
};

/// Handles opening UNC paths on different platforms
class PathOpener
{
public:
    explicit PathOpener(const Config& config);

    /// Parse and validate a URL, then open it
    /// Returns the result of the operation
    [[nodiscard]] OpenResult open(const QString& url);

    /// Parse and validate a URL without opening
    /// Returns the result (success if valid and allowed)
    [[nodiscard]] OpenResult validate(const QString& url) const;

    /// Get the target URL/path that would be opened for a given input URL
    /// Returns empty string if validation fails
    [[nodiscard]] QString getTargetPath(const QString& url) const;

    /// Get the last parsed UNC path (for display purposes)
    /// Only valid after a successful validate() or open() call
    [[nodiscard]] const UncPath& lastParsedPath() const { return m_lastPath; }

private:
    /// Build the platform-specific target URL/path from a UncPath
    [[nodiscard]] QString buildTargetUrl(const UncPath& path) const;

    /// Actually open the target URL using the system
    [[nodiscard]] static bool openUrl(const QString& url);

    Config m_config;
    SecurityPolicy m_policy;
    UrlParser m_parser;
    mutable UncPath m_lastPath;
};

} // namespace uncopener

#endif // UNCOPENER_PATHOPENER_HPP
