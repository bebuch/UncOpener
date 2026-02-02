#include "Config.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QTest>

using namespace uncopener;

class ConfigTest : public QObject
{
    Q_OBJECT

private slots:
    void testDefaultValues()
    {
        Config config;
        QCOMPARE(config.schemeName(), QString(Config::DEFAULT_SCHEME_NAME));
        QVERIFY(config.uncAllowList().isEmpty());
        QVERIFY(config.smbUsername().isEmpty());
        QCOMPARE(config.filetypeMode(), Config::DEFAULT_FILETYPE_MODE);
        QVERIFY(config.filetypeWhitelist().isEmpty());
        QVERIFY(config.filetypeBlacklist().isEmpty());
    }

    void testSettersAndGetters()
    {
        Config config;

        config.setSchemeName("myscheme");
        QCOMPARE(config.schemeName(), "myscheme");

        config.setUncAllowList({R"(\\server\share)"});
        QCOMPARE(config.uncAllowList(), QStringList{R"(\\server\share)"});

        config.setSmbUsername("user");
        QCOMPARE(config.smbUsername(), "user");

        config.setFiletypeMode(FiletypeMode::Blacklist);
        QCOMPARE(config.filetypeMode(), FiletypeMode::Blacklist);

        config.setFiletypeWhitelist({".txt", ".pdf"});
        QCOMPARE(config.filetypeWhitelist(), QStringList({".txt", ".pdf"}));

        config.setFiletypeBlacklist({".exe"});
        QCOMPARE(config.filetypeBlacklist(), QStringList{".exe"});
    }

    void testReset()
    {
        Config config;
        config.setSchemeName("custom");
        config.setUncAllowList({R"(\\server\share)"});
        config.setFiletypeMode(FiletypeMode::Blacklist);

        config.reset();

        QCOMPARE(config.schemeName(), QString(Config::DEFAULT_SCHEME_NAME));
        QVERIFY(config.uncAllowList().isEmpty());
        QCOMPARE(config.filetypeMode(), Config::DEFAULT_FILETYPE_MODE);
    }

    void testJsonSerialization()
    {
        Config config;
        config.setSchemeName("myscheme");
        config.setUncAllowList({R"(\\server1\share)", R"(\\server2\share)"});
        config.setSmbUsername("domain\\user");
        config.setFiletypeMode(FiletypeMode::Blacklist);
        config.setFiletypeWhitelist({".txt"});
        config.setFiletypeBlacklist({".exe", ".bat"});

        QJsonObject json = config.toJson();

        QCOMPARE(json["schemeName"].toString(), "myscheme");
        QCOMPARE(json["uncAllowList"].toArray().size(), 2);
        QCOMPARE(json["smbUsername"].toString(), "domain\\user");
        QCOMPARE(json["filetypeMode"].toString(), "blacklist");
        QCOMPARE(json["filetypeWhitelist"].toArray().size(), 1);
        QCOMPARE(json["filetypeBlacklist"].toArray().size(), 2);
    }

    void testToJsonBytes()
    {
        Config config;
        config.setSchemeName("bytestest");
        config.setUncAllowList({R"(\\server\share)"});

        QByteArray bytes = config.toJsonBytes();

        // Should be valid JSON
        QVERIFY(!bytes.isEmpty());

        // Should be parseable
        QJsonDocument doc = QJsonDocument::fromJson(bytes);
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());

        QJsonObject json = doc.object();
        QCOMPARE(json["schemeName"].toString(), QString("bytestest"));
    }

    void testToJsonBytesComparison()
    {
        // Two configs with same values should produce same JSON bytes
        Config config1;
        config1.setSchemeName("compare");
        config1.setUncAllowList({R"(\\server\share)"});
        config1.setFiletypeMode(FiletypeMode::Whitelist);
        config1.setFiletypeWhitelist({".txt"});

        Config config2;
        config2.setSchemeName("compare");
        config2.setUncAllowList({R"(\\server\share)"});
        config2.setFiletypeMode(FiletypeMode::Whitelist);
        config2.setFiletypeWhitelist({".txt"});

        QCOMPARE(config1.toJsonBytes(), config2.toJsonBytes());
    }

    void testToJsonBytesDifferentConfigs()
    {
        // Two configs with different values should produce different JSON bytes
        Config config1;
        config1.setSchemeName("first");

        Config config2;
        config2.setSchemeName("second");

        QVERIFY(config1.toJsonBytes() != config2.toJsonBytes());
    }

    void testJsonDeserialization()
    {
        QJsonObject json;
        json["schemeName"] = "custom";
        json["uncAllowList"] = QJsonArray{R"(\\server\share)"};
        json["smbUsername"] = "user";
        json["filetypeMode"] = "blacklist";
        json["filetypeWhitelist"] = QJsonArray{".txt", ".pdf"};
        json["filetypeBlacklist"] = QJsonArray{".exe"};

        Config config;
        QVERIFY(config.fromJson(json));

        QCOMPARE(config.schemeName(), "custom");
        QCOMPARE(config.uncAllowList(), QStringList{R"(\\server\share)"});
        QCOMPARE(config.smbUsername(), "user");
        QCOMPARE(config.filetypeMode(), FiletypeMode::Blacklist);
        QCOMPARE(config.filetypeWhitelist(), QStringList({".txt", ".pdf"}));
        QCOMPARE(config.filetypeBlacklist(), QStringList{".exe"});
    }

    void testJsonDeserializationDefaults()
    {
        // Empty JSON should use defaults
        QJsonObject json;

        Config config;
        config.setSchemeName("custom"); // Set something first
        QVERIFY(config.fromJson(json));

        QCOMPARE(config.schemeName(), QString(Config::DEFAULT_SCHEME_NAME));
        QVERIFY(config.uncAllowList().isEmpty());
        QCOMPARE(config.filetypeMode(), Config::DEFAULT_FILETYPE_MODE);
    }

    void testJsonRoundTrip()
    {
        Config original;
        original.setSchemeName("roundtrip");
        original.setUncAllowList({R"(\\server\share1)", R"(\\server\share2)"});
        original.setSmbUsername("testuser");
        original.setFiletypeMode(FiletypeMode::Blacklist);
        original.setFiletypeWhitelist({".doc", ".docx"});
        original.setFiletypeBlacklist({".exe", ".bat", ".cmd"});

        QJsonObject json = original.toJson();

        Config loaded;
        QVERIFY(loaded.fromJson(json));

        QCOMPARE(loaded.schemeName(), original.schemeName());
        QCOMPARE(loaded.uncAllowList(), original.uncAllowList());
        QCOMPARE(loaded.smbUsername(), original.smbUsername());
        QCOMPARE(loaded.filetypeMode(), original.filetypeMode());
        QCOMPARE(loaded.filetypeWhitelist(), original.filetypeWhitelist());
        QCOMPARE(loaded.filetypeBlacklist(), original.filetypeBlacklist());
    }

    void testFilePersistence()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString filePath = tempDir.path() + "/config.json";

        // Save config
        Config original;
        original.setSchemeName("filetest");
        original.setUncAllowList({R"(\\server\share)"});
        QVERIFY(original.saveTo(filePath));

        // Verify file was created
        QVERIFY(QFile::exists(filePath));

        // Load config
        Config loaded;
        QVERIFY(loaded.loadFrom(filePath));

        QCOMPARE(loaded.schemeName(), "filetest");
        QCOMPARE(loaded.uncAllowList(), QStringList{R"(\\server\share)"});
    }

    void testLoadNonExistentFile()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString filePath = tempDir.path() + "/nonexistent.json";

        Config config;
        config.setSchemeName("willbereset");

        // Loading non-existent file should reset to defaults
        QVERIFY(!config.loadFrom(filePath));
        QCOMPARE(config.schemeName(), QString(Config::DEFAULT_SCHEME_NAME));
    }

    void testLoadInvalidJson()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QString filePath = tempDir.path() + "/invalid.json";

        // Write invalid JSON
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{ invalid json");
        file.close();

        Config config;
        config.setSchemeName("willbereset");

        // Loading invalid JSON should reset to defaults
        QVERIFY(!config.loadFrom(filePath));
        QCOMPARE(config.schemeName(), QString(Config::DEFAULT_SCHEME_NAME));
    }

    void testApplyToSecurityPolicy()
    {
        Config config;
        config.setUncAllowList({R"(\\server\share)"});
        config.setFiletypeMode(FiletypeMode::Blacklist);
        config.setFiletypeBlacklist({".exe"});

        SecurityPolicy policy;
        config.applyTo(policy);

        // Verify allow-list was applied
        QVERIFY(policy.uncAllowList().check(R"(\\server\share\file.txt)").allowed);
        QVERIFY(!policy.uncAllowList().check(R"(\\other\share\file.txt)").allowed);

        // Verify filetype policy was applied
        QCOMPARE(policy.filetypePolicy().mode(), FiletypeMode::Blacklist);
        QVERIFY(!policy.filetypePolicy().check("file.exe").allowed);
        QVERIFY(policy.filetypePolicy().check("file.txt").allowed);
    }

    void testCreateConfigDirectory()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        // Save to a path that doesn't exist yet
        QString filePath = tempDir.path() + "/subdir/config.json";

        Config config;
        config.setSchemeName("dirtest");
        QVERIFY(config.saveTo(filePath));

        // Verify directory and file were created
        QVERIFY(QFile::exists(filePath));
    }
};

int runConfigTests(int argc, char* argv[])
{
    ConfigTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "ConfigTests.moc"
