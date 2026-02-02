#include "PathOpener.hpp"

#include <QTest>

using namespace uncopener;

class PathOpenerTest : public QObject
{
    Q_OBJECT

private slots:
    void testValidateSuccess()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});

        PathOpener opener(config);
        OpenResult result = opener.validate("unc://server/share/file.txt");

        QVERIFY(result.success);
        QVERIFY(result.errorReason.isEmpty());
    }

    void testValidateParseError()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});

        PathOpener opener(config);
        OpenResult result = opener.validate("invalid-url");

        QVERIFY(!result.success);
        QVERIFY(!result.errorReason.isEmpty());
    }

    void testValidateNotInAllowList()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\allowed\share)"});

        PathOpener opener(config);
        OpenResult result = opener.validate("unc://notallowed/share/file.txt");

        QVERIFY(!result.success);
        QVERIFY(result.errorReason.contains("allow"));
    }

    void testValidateFiletypeBlocked()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});
        config.setFiletypeMode(FiletypeMode::Blacklist);
        config.setFiletypeBlacklist({".exe"});

        PathOpener opener(config);
        OpenResult result = opener.validate("unc://server/share/malware.exe");

        QVERIFY(!result.success);
        QVERIFY(result.errorReason.contains("blacklist"));
    }

    void testValidateFiletypeAllowed()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});
        config.setFiletypeMode(FiletypeMode::Whitelist);
        config.setFiletypeWhitelist({".txt", ".pdf"});

        PathOpener opener(config);

        QVERIFY(opener.validate("unc://server/share/doc.txt").success);
        QVERIFY(opener.validate("unc://server/share/doc.pdf").success);
        QVERIFY(!opener.validate("unc://server/share/doc.exe").success);
    }

    void testGetTargetPathWindows()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});

        PathOpener opener(config);
        QString target = opener.getTargetPath("unc://server/share/path/file.txt");

#ifdef Q_OS_WIN
        QCOMPARE(target, R"(\\server\share\path\file.txt)");
#else
        QCOMPARE(target, "smb://server/share/path/file.txt");
#endif
    }

    void testGetTargetPathWithUsername()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});
        config.setSmbUsername("testuser");

        PathOpener opener(config);
        QString target = opener.getTargetPath("unc://server/share/file.txt");

#ifdef Q_OS_WIN
        // Windows doesn't use username in path
        QCOMPARE(target, R"(\\server\share\file.txt)");
#else
        QVERIFY(target.contains("testuser@"));
#endif
    }

    void testGetTargetPathWithDomainUsername()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});
        config.setSmbUsername(R"(DOMAIN\user)");

        PathOpener opener(config);
        QString target = opener.getTargetPath("unc://server/share/file.txt");

#ifndef Q_OS_WIN
        // Linux should percent-encode the backslash
        QVERIFY(target.contains("DOMAIN%5C") || target.contains("DOMAIN%5c"));
#endif
    }

    void testGetTargetPathTrailingSlash()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});

        PathOpener opener(config);

        QString withSlash = opener.getTargetPath("unc://server/share/folder/");
        QString withoutSlash = opener.getTargetPath("unc://server/share/folder");

#ifdef Q_OS_WIN
        QVERIFY(withSlash.endsWith('\\'));
        QVERIFY(!withoutSlash.endsWith('\\'));
#else
        QVERIFY(withSlash.endsWith('/'));
        QVERIFY(!withoutSlash.endsWith('/'));
#endif
    }

    void testGetTargetPathInvalid()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});

        PathOpener opener(config);
        QString target = opener.getTargetPath("invalid-url");

        QVERIFY(target.isEmpty());
    }

    void testLastParsedPath()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});

        PathOpener opener(config);
        auto result = opener.validate("unc://server/share/path/file.txt");
        QVERIFY(result.success);

        const UncPath& path = opener.lastParsedPath();
        QCOMPARE(path.server, "server");
        QCOMPARE(path.path, R"(share\path\file.txt)");
    }

    void testDirectoryTraversalBlocked()
    {
        Config config;
        config.setSchemeName("unc");
        config.setUncAllowList({R"(\\server\share)"});

        PathOpener opener(config);
        OpenResult result = opener.validate("unc://server/share/../other/file.txt");

        QVERIFY(!result.success);
        QVERIFY(result.errorReason.contains("traversal"));
    }
};

int runPathOpenerTests(int argc, char* argv[])
{
    PathOpenerTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_path_opener.moc"
