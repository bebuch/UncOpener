#include <QTest>
#include <QString>
#include <QVector>

// Test vectors based on docs/url-contract.md
// These tests define the expected behavior for URL parsing and validation.
// The actual implementation will be added in Step 4.

struct ValidUrlTestCase
{
    QString input;
    QString expectedUncPath;
    QString description;
};

struct InvalidUrlTestCase
{
    QString input;
    QString expectedReason;
};

class UrlContractTest : public QObject
{
    Q_OBJECT

private:
    // Valid URL test cases
    static QVector<ValidUrlTestCase> validUrls()
    {
        return {
            {"unc://server/share", R"(\\server\share)", "Minimal valid path"},
            {"unc://server/share/", R"(\\server\share\)", "Trailing slash preserved"},
            {"unc://server/share/path", R"(\\server\share\path)", "Path component"},
            {"unc://server/share/path/file.txt", R"(\\server\share\path\file.txt)", "Full path"},
            {"unc://server/share/path%20name", R"(\\server\share\path name)",
             "Percent-encoded space"},
            {"unc://server/share/path name", R"(\\server\share\path name)", "Literal space"},
            {"unc://server/share/file%23name", R"(\\server\share\file#name)",
             "Percent-encoded hash"},
            {"unc://server/share/./file", R"(\\server\share\file)", "Dot segment removed"},
            {"unc://server/share//path", R"(\\server\share\path)", "Double slash collapsed"},
            {"unc://SERVER/SHARE/path", R"(\\SERVER\SHARE\path)", "Case preserved"},
            {"unc://server/share/path?query=value", R"(\\server\share\path)", "Query ignored"},
            {"unc://server/share/path#fragment", R"(\\server\share\path)", "Fragment ignored"},
        };
    }

    // Invalid URL test cases
    static QVector<InvalidUrlTestCase> invalidUrls()
    {
        return {
            {"//server/share", "Missing scheme"},
            {"unc:/server/share", "Single slash (invalid format)"},
            {"unc:///share", "Missing authority"},
            {"unc://server", "Missing share"},
            {"unc://server/share/../other", "Directory traversal"},
            {"unc://server/share/path/../../other", "Directory traversal (multiple)"},
            {"http://server/share", "Wrong scheme"},
            {"", "Empty input"},
            {"unc://", "Missing authority and path"},
            {"unc:// /share", "Whitespace authority"},
        };
    }

private slots:
    void testValidUrlCount()
    {
        // Ensure we have test vectors defined
        QVERIFY(validUrls().size() >= 10);
    }

    void testInvalidUrlCount()
    {
        // Ensure we have test vectors defined
        QVERIFY(invalidUrls().size() >= 8);
    }

    void testValidUrlVectorsDocumented()
    {
        // This test verifies that all valid test vectors have descriptions
        for (const auto& testCase : validUrls())
        {
            QVERIFY2(!testCase.description.isEmpty(),
                     qPrintable(QString("Test case '%1' has no description").arg(testCase.input)));
        }
    }

    void testInvalidUrlVectorsDocumented()
    {
        // This test verifies that all invalid test vectors have reasons
        for (const auto& testCase : invalidUrls())
        {
            QVERIFY2(!testCase.expectedReason.isEmpty(),
                     qPrintable(QString("Test case '%1' has no reason").arg(testCase.input)));
        }
    }

    // The following tests will be implemented in Step 4 when the parsing logic is added:
    //
    // void testValidUrlParsing_data();
    // void testValidUrlParsing();
    // void testInvalidUrlRejection_data();
    // void testInvalidUrlRejection();
    // void testPercentDecoding_data();
    // void testPercentDecoding();
    // void testDotSegmentRemoval_data();
    // void testDotSegmentRemoval();
    // void testSlashCollapsing_data();
    // void testSlashCollapsing();
    // void testTrailingSlashPreservation_data();
    // void testTrailingSlashPreservation();
};

int runUrlContractTests(int argc, char* argv[])
{
    UrlContractTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_url_contract.moc"
