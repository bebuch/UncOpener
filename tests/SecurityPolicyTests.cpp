#include "SecurityPolicy.hpp"

#include <QString>
#include <QTest>

using namespace uncopener;

class SecurityPolicyTest : public QObject
{
    Q_OBJECT

private slots:
    // UNC Allow-List Tests

    void testUncAllowListEmpty()
    {
        // Empty allow-list allows all UNC paths
        UncAllowList list;
        auto result = list.check(R"(\\server\share\path)");
        QVERIFY(result.allowed);
    }

    void testUncAllowListBasicMatch()
    {
        UncAllowList list;
        list.addEntry(R"(\\server\share)");

        auto result = list.check(R"(\\server\share\path\file.txt)");
        QVERIFY(result.allowed);
    }

    void testUncAllowListExactMatch()
    {
        UncAllowList list;
        list.addEntry(R"(\\server\share)");

        auto result = list.check(R"(\\server\share)");
        QVERIFY(result.allowed);
    }

    void testUncAllowListCaseInsensitive()
    {
        UncAllowList list;
        list.addEntry(R"(\\Server\Share)");

        auto result = list.check(R"(\\SERVER\SHARE\path)");
        QVERIFY(result.allowed);
    }

    void testUncAllowListNoMatch()
    {
        UncAllowList list;
        list.addEntry(R"(\\server\share1)");

        auto result = list.check(R"(\\server\share2\path)");
        QVERIFY(!result.allowed);
    }

    void testUncAllowListPartialMatch()
    {
        UncAllowList list;
        list.addEntry(R"(\\server\share\allowed)");

        // Should not match - the entry is more specific than the path
        auto result = list.check(R"(\\server\share\other)");
        QVERIFY(!result.allowed);
    }

    void testUncAllowListMultipleEntries()
    {
        UncAllowList list;
        list.addEntry(R"(\\server1\share)");
        list.addEntry(R"(\\server2\share)");

        QVERIFY(list.check(R"(\\server1\share\path)").allowed);
        QVERIFY(list.check(R"(\\server2\share\path)").allowed);
        QVERIFY(!list.check(R"(\\server3\share\path)").allowed);
    }

    void testUncAllowListInvalidEntry()
    {
        UncAllowList list;

        // Forward slashes should be rejected
        QVERIFY(!list.addEntry("//server/share"));

        // Empty entries should be rejected
        QVERIFY(!list.addEntry(""));
        QVERIFY(!list.addEntry("   "));
    }

    void testUncAllowListNormalization()
    {
        // Test that entries are normalized with double backslash
        QCOMPARE(UncAllowList::normalizeEntry("server\\share"), R"(\\server\share)");
        QCOMPARE(UncAllowList::normalizeEntry(R"(\server\share)"), R"(\\server\share)");
        QCOMPARE(UncAllowList::normalizeEntry(R"(\\server\share)"), R"(\\server\share)");
    }

    // Filetype Policy Tests

    void testFiletypePolicyWhitelistEmpty()
    {
        FiletypePolicy policy;
        policy.setMode(FiletypeMode::Whitelist);

        // Empty whitelist allows everything
        auto result = policy.check("file.exe");
        QVERIFY(result.allowed);
    }

    void testFiletypePolicyWhitelistMatch()
    {
        FiletypePolicy policy;
        policy.setMode(FiletypeMode::Whitelist);
        policy.addWhitelistEntry(".txt");
        policy.addWhitelistEntry(".pdf");

        QVERIFY(policy.check("document.txt").allowed);
        QVERIFY(policy.check("document.pdf").allowed);
        QVERIFY(!policy.check("document.exe").allowed);
    }

    void testFiletypePolicyWhitelistCaseInsensitive()
    {
        FiletypePolicy policy;
        policy.setMode(FiletypeMode::Whitelist);
        policy.addWhitelistEntry(".TXT");

        QVERIFY(policy.check("file.txt").allowed);
        QVERIFY(policy.check("file.TXT").allowed);
        QVERIFY(policy.check("file.Txt").allowed);
    }

    void testFiletypePolicyBlacklistEmpty()
    {
        FiletypePolicy policy;
        policy.setMode(FiletypeMode::Blacklist);

        // Empty blacklist allows everything
        auto result = policy.check("file.exe");
        QVERIFY(result.allowed);
    }

    void testFiletypePolicyBlacklistMatch()
    {
        FiletypePolicy policy;
        policy.setMode(FiletypeMode::Blacklist);
        policy.addBlacklistEntry(".exe");
        policy.addBlacklistEntry(".bat");

        QVERIFY(!policy.check("file.exe").allowed);
        QVERIFY(!policy.check("script.bat").allowed);
        QVERIFY(policy.check("document.txt").allowed);
    }

    void testFiletypePolicyBlacklistCaseInsensitive()
    {
        FiletypePolicy policy;
        policy.setMode(FiletypeMode::Blacklist);
        policy.addBlacklistEntry(".exe");

        QVERIFY(!policy.check("file.EXE").allowed);
        QVERIFY(!policy.check("file.Exe").allowed);
    }

    void testFiletypePolicyInvalidExtension()
    {
        FiletypePolicy policy;

        // Path separators should be rejected
        QVERIFY(!policy.addWhitelistEntry(".exe/txt"));
        QVERIFY(!policy.addWhitelistEntry(R"(.exe\txt)"));
        QVERIFY(!policy.addBlacklistEntry("path/file.exe"));

        // Empty extensions should be rejected
        QVERIFY(!policy.addWhitelistEntry(""));
        QVERIFY(!policy.addBlacklistEntry("   "));
    }

    void testFiletypePolicyNormalization()
    {
        // Test extension normalization
        QCOMPARE(FiletypePolicy::normalizeExtension("txt"), ".txt");
        QCOMPARE(FiletypePolicy::normalizeExtension(".txt"), ".txt");
        QCOMPARE(FiletypePolicy::normalizeExtension(".TXT"), ".txt");
        QCOMPARE(FiletypePolicy::normalizeExtension("  .txt  "), ".txt");
    }

    void testFiletypePolicyEndsWithMatch()
    {
        FiletypePolicy policy;
        policy.setMode(FiletypeMode::Whitelist);
        policy.addWhitelistEntry(".txt");

        // Should match files that end with the extension
        QVERIFY(policy.check("file.txt").allowed);
        QVERIFY(policy.check("path/to/file.txt").allowed);
        QVERIFY(policy.check("file.backup.txt").allowed);

        // Should not match partial extensions
        QVERIFY(!policy.check("file.txt.exe").allowed);
    }

    // Combined Security Policy Tests

    void testSecurityPolicyBothChecks()
    {
        SecurityPolicy policy;
        policy.uncAllowList().addEntry(R"(\\server\share)");
        policy.filetypePolicy().setMode(FiletypeMode::Whitelist);
        policy.filetypePolicy().addWhitelistEntry(".txt");

        // Path allowed, extension allowed
        QVERIFY(policy.check(R"(\\server\share\file.txt)").allowed);

        // Path allowed, extension not allowed
        QVERIFY(!policy.check(R"(\\server\share\file.exe)").allowed);

        // Path not allowed
        QVERIFY(!policy.check(R"(\\other\share\file.txt)").allowed);
    }

    void testSecurityPolicyDirectoryPath()
    {
        SecurityPolicy policy;
        policy.uncAllowList().addEntry(R"(\\server\share)");
        policy.filetypePolicy().setMode(FiletypeMode::Whitelist);
        policy.filetypePolicy().addWhitelistEntry(".txt");

        // Directory paths (ending with \) should skip filetype check
        QVERIFY(policy.check(R"(\\server\share\folder\)").allowed);
    }
};

int runSecurityPolicyTests(int argc, char* argv[])
{
    SecurityPolicyTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "SecurityPolicyTests.moc"
