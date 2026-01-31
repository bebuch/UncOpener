#include <QFile>
#include <QIcon>
#include <QSvgRenderer>
#include <QTest>

class ResourcesTest : public QObject
{
    Q_OBJECT

private slots:
    void testIconResourceExists()
    {
        // Verify the icon resource file exists
        QFile iconFile(":/icons/icon.svg");
        QVERIFY2(iconFile.exists(), "Icon resource :/icons/icon.svg does not exist");
    }

    void testIconResourceReadable()
    {
        // Verify the icon resource can be opened and read
        QFile iconFile(":/icons/icon.svg");
        QVERIFY(iconFile.open(QIODevice::ReadOnly));
        QByteArray content = iconFile.readAll();
        QVERIFY(!content.isEmpty());
        // SVG files should start with XML declaration or svg tag
        QVERIFY(content.contains("<svg") || content.contains("<?xml"));
    }

    void testSvgRendererLoadsIcon()
    {
        // Verify QSvgRenderer can load the icon
        QSvgRenderer renderer(QString(":/icons/icon.svg"));
        QVERIFY2(renderer.isValid(), "QSvgRenderer failed to load icon.svg");
        QVERIFY(renderer.defaultSize().width() > 0);
        QVERIFY(renderer.defaultSize().height() > 0);
    }

    void testQIconLoadsFromResource()
    {
        // Verify QIcon can be created from the resource
        QIcon icon(":/icons/icon.svg");
        QVERIFY2(!icon.isNull(), "QIcon failed to load from :/icons/icon.svg");

        // Verify we can get a pixmap from the icon
        QPixmap pixmap = icon.pixmap(64, 64);
        QVERIFY(!pixmap.isNull());
        QCOMPARE(pixmap.width(), 64);
        QCOMPARE(pixmap.height(), 64);
    }
};

QTEST_MAIN(ResourcesTest)
#include "test_resources.moc"
