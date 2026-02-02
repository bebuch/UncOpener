#include "UrlParser.hpp"

#include <QString>
#include <QTest>
#include <QVector>

using namespace uncopener;

// Test vectors based on docs/url-contract.md

struct ValidUrlTestCase
{
    QString input;
    QString expectedUncPath;
    QString description;
};

struct InvalidUrlTestCase
{
    QString input;
    ParseError::Code expectedCode;
    QString description;
};

class UrlContractTest : public QObject
{
    Q_OBJECT

private:
    // Valid URL test cases
    static QVector<ValidUrlTestCase> validUrls()
    {
        return {
            {"unc://server", R"(\\server)", "Server only"},
            {"unc://server/", R"(\\server\)", "Server with trailing slash"},
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
            {"//server/share", ParseError::Code::MissingScheme, "Missing scheme"},
            {"unc:/server/share", ParseError::Code::InvalidSchemeFormat,
             "Single slash (invalid format)"},
            {"unc:///share", ParseError::Code::MissingAuthority, "Missing authority"},
            {"unc://server/share/../other", ParseError::Code::DirectoryTraversal,
             "Directory traversal"},
            {"unc://server/share/path/../../other", ParseError::Code::DirectoryTraversal,
             "Directory traversal (multiple)"},
            {"http://server/share", ParseError::Code::WrongScheme, "Wrong scheme"},
            {"", ParseError::Code::EmptyInput, "Empty input"},
            {"unc://", ParseError::Code::MissingAuthority, "Missing authority and path"},
            {"unc:// /share", ParseError::Code::WhitespaceAuthority, "Whitespace authority"},
        };
    }

private slots:
    void testValidUrlParsing_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("expectedUncPath");

        for (const auto& testCase : validUrls())
        {
            QTest::newRow(qPrintable(testCase.description))
                << testCase.input << testCase.expectedUncPath;
        }
    }

    void testValidUrlParsing()
    {
        QFETCH(QString, input);
        QFETCH(QString, expectedUncPath);

        UrlParser parser("unc");
        ParseResult result = parser.parse(input);

        QVERIFY2(isSuccess(result),
                 qPrintable(QString("Expected success for '%1', got error: %2")
                                .arg(input, isError(result) ? getError(result).reason : "")));

        const UncPath& path = getPath(result);
        QCOMPARE(path.toUncString(), expectedUncPath);
    }

    void testInvalidUrlRejection_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("expectedCode");

        for (const auto& testCase : invalidUrls())
        {
            QTest::newRow(qPrintable(testCase.description))
                << testCase.input << static_cast<int>(testCase.expectedCode);
        }
    }

    void testInvalidUrlRejection()
    {
        QFETCH(QString, input);
        QFETCH(int, expectedCode);

        UrlParser parser("unc");
        ParseResult result = parser.parse(input);

        QVERIFY2(isError(result),
                 qPrintable(QString("Expected error for '%1', got success").arg(input)));

        const ParseError& error = getError(result);
        QCOMPARE(static_cast<int>(error.code), expectedCode);
    }

    void testTrailingSlashPreservation()
    {
        UrlParser parser("unc");

        // Without trailing slash
        {
            ParseResult result = parser.parse("unc://server/share/path");
            QVERIFY(isSuccess(result));
            QVERIFY(!getPath(result).hasTrailingSlash);
            QVERIFY(!getPath(result).toUncString().endsWith('\\'));
        }

        // With trailing slash
        {
            ParseResult result = parser.parse("unc://server/share/path/");
            QVERIFY(isSuccess(result));
            QVERIFY(getPath(result).hasTrailingSlash);
            QVERIFY(getPath(result).toUncString().endsWith('\\'));
        }
    }

    void testSmbUrlGeneration()
    {
        UrlParser parser("unc");

        // Basic SMB URL
        {
            ParseResult result = parser.parse("unc://server/share/path");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).toSmbUrl(), "smb://server/share/path");
        }

        // SMB URL with username
        {
            ParseResult result = parser.parse("unc://server/share/path");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).toSmbUrl("myuser"), "smb://myuser@server/share/path");
        }

        // SMB URL with domain username
        {
            ParseResult result = parser.parse("unc://server/share/path");
            QVERIFY(isSuccess(result));
            QString smbUrl = getPath(result).toSmbUrl(R"(DOMAIN\user)");
            QVERIFY(smbUrl.contains("DOMAIN%5Cuser@"));
        }

        // SMB URL with trailing slash
        {
            ParseResult result = parser.parse("unc://server/share/path/");
            QVERIFY(isSuccess(result));
            QVERIFY(getPath(result).toSmbUrl().endsWith('/'));
        }
    }

    void testDifferentScheme()
    {
        // Using a different scheme name
        UrlParser parser("myscheme");

        // Should succeed with matching scheme
        {
            ParseResult result = parser.parse("myscheme://server/share");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).server, "server");
        }

        // Should fail with different scheme
        {
            ParseResult result = parser.parse("unc://server/share");
            QVERIFY(isError(result));
            QCOMPARE(getError(result).code, ParseError::Code::WrongScheme);
        }
    }

    void testPercentDecoding()
    {
        UrlParser parser("unc");

        // Percent-encoded space
        {
            ParseResult result = parser.parse("unc://server/share/my%20file.txt");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).path, "my file.txt");
        }

        // Percent-encoded special characters
        {
            ParseResult result = parser.parse("unc://server/share/file%23name");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).path, "file#name");
        }

        // Mixed percent-encoded and literal
        {
            ParseResult result = parser.parse("unc://server/share/path%20with spaces");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).path, "path with spaces");
        }
    }

    void testSlashCollapsing()
    {
        UrlParser parser("unc");

        // Double slashes in path
        {
            ParseResult result = parser.parse("unc://server/share//path//file");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).toUncString(), R"(\\server\share\path\file)");
        }

        // Multiple consecutive slashes
        {
            ParseResult result = parser.parse("unc://server/share///path");
            QVERIFY(isSuccess(result));
            QCOMPARE(getPath(result).toUncString(), R"(\\server\share\path)");
        }
    }

    void testErrorMessages()
    {
        UrlParser parser("unc");

        // Verify error messages are populated
        ParseResult result = parser.parse("");
        QVERIFY(isError(result));

        const ParseError& error = getError(result);
        QVERIFY(!error.reason.isEmpty());
        QVERIFY(!error.remediation.isEmpty());
        QCOMPARE(error.input, "");
    }

    void testDetailedWrongSchemeError()
    {
        UrlParser parser("unc");

        // Test with a recognizable scheme
        ParseResult result = parser.parse("http://example.com");
        QVERIFY(isError(result));
        const ParseError& error = getError(result);

        QCOMPARE(error.code, ParseError::Code::WrongScheme);
        QVERIFY(error.reason.contains("http"));
        QVERIFY(error.remediation.contains("unc"));
    }
};

int runUrlContractTests(int argc, char* argv[])
{
    UrlContractTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_url_contract.moc"
