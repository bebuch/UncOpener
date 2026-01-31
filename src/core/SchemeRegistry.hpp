#ifndef UNCOPENER_SCHEMEREGISTRY_HPP
#define UNCOPENER_SCHEMEREGISTRY_HPP

#include <QString>

#include <memory>

namespace uncopener
{

/// Status of scheme registration
enum class RegistrationStatus
{
    /// Scheme is not registered
    NotRegistered,
    /// Scheme is registered and points to this binary
    RegisteredToThisBinary,
    /// Scheme is registered but points to a different binary
    RegisteredToOtherBinary
};

/// Result of a registration operation
struct RegistrationResult
{
    bool success = false;
    QString errorMessage;

    [[nodiscard]] static RegistrationResult ok() { return {true, {}}; }
    [[nodiscard]] static RegistrationResult error(const QString& message)
    {
        return {false, message};
    }
};

/// Abstract interface for scheme registration
/// Platform-specific implementations handle Windows registry or Linux xdg-mime
class SchemeRegistry
{
public:
    virtual ~SchemeRegistry() = default;

    /// Check if the scheme is registered and to which binary
    [[nodiscard]] virtual RegistrationStatus checkRegistration(const QString& schemeName) const = 0;

    /// Get the path of the currently registered binary (if any)
    [[nodiscard]] virtual QString getRegisteredBinaryPath(const QString& schemeName) const = 0;

    /// Register the scheme to point to the current binary
    [[nodiscard]] virtual RegistrationResult registerScheme(const QString& schemeName) = 0;

    /// Unregister the scheme
    [[nodiscard]] virtual RegistrationResult unregisterScheme(const QString& schemeName) = 0;

    /// Get the path of the current binary
    [[nodiscard]] static QString currentBinaryPath();

    /// Create the platform-appropriate registry implementation
    [[nodiscard]] static std::unique_ptr<SchemeRegistry> create();
};

} // namespace uncopener

#endif // UNCOPENER_SCHEMEREGISTRY_HPP
