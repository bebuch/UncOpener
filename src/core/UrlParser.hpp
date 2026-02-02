#ifndef UNCOPENER_URLPARSER_HPP
#define UNCOPENER_URLPARSER_HPP

#include <QString>

#include <cstdint>
#include <optional>
#include <variant>

namespace uncopener
{

/// Represents a successfully parsed UNC path
struct UncPath
{
    QString server;
    QString path; // Path after server, may be empty
    bool hasTrailingSlash = false;

    /// Returns the full UNC path string (e.g., "\\server\path")
    [[nodiscard]] QString toUncString() const;

    /// Returns the SMB URL for Linux (e.g., "smb://server/path")
    [[nodiscard]] QString toSmbUrl(const QString& username = {}) const;
};

/// Error information for failed URL parsing
struct ParseError
{
    enum class Code : std::uint8_t
    {
        EmptyInput,
        MissingScheme,
        WrongScheme,
        InvalidSchemeFormat, // Single slash instead of double
        MissingAuthority,
        WhitespaceAuthority,
        DirectoryTraversal,
        InvalidCharacter,
    };

    Code code{};
    QString reason;      // Human-readable error description
    QString remediation; // Suggestion for fixing the error
    QString input;       // The original input that failed

    /// Creates a ParseError with localized messages
    [[nodiscard]] static ParseError create(Code code, const QString& input,
                                           const QString& expectedScheme = {},
                                           const QString& foundScheme = {});
};

/// Result type for URL parsing: either a UncPath or a ParseError
using ParseResult = std::variant<UncPath, ParseError>;

/// URL parser for converting scheme URLs to UNC paths
class UrlParser
{
public:
    explicit UrlParser(QString schemeName);

    /// Parse a URL string and return either a UncPath or ParseError
    [[nodiscard]] ParseResult parse(const QString& input) const;

    /// Get the expected scheme name
    [[nodiscard]] QString schemeName() const { return m_schemeName; }

private:
    QString m_schemeName;

    /// Percent-decode a string
    [[nodiscard]] static QString percentDecode(const QString& input);

    /// Normalize path: collapse slashes, remove dot segments
    /// Returns nullopt if directory traversal (..) is detected
    [[nodiscard]] static std::optional<QString> normalizePath(const QString& path,
                                                              bool& hasTrailingSlash);

    /// Check if the input starts with the correct scheme
    [[nodiscard]] std::optional<ParseError> checkScheme(const QString& input) const;

    /// Remove query and fragment from the input
    [[nodiscard]] static QString stripQueryAndFragment(const QString& input);
};

/// Helper functions for working with ParseResult
[[nodiscard]] inline bool isSuccess(const ParseResult& result)
{
    return std::holds_alternative<UncPath>(result);
}

[[nodiscard]] inline bool isError(const ParseResult& result)
{
    return std::holds_alternative<ParseError>(result);
}

[[nodiscard]] inline const UncPath& getPath(const ParseResult& result)
{
    return std::get<UncPath>(result);
}

[[nodiscard]] inline const ParseError& getError(const ParseResult& result)
{
    return std::get<ParseError>(result);
}

} // namespace uncopener

#endif // UNCOPENER_URLPARSER_HPP
