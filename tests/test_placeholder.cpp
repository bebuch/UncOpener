#include <QTest>

class PlaceholderTest : public QObject
{
    Q_OBJECT

private slots:
    void testTrue()
    {
        QVERIFY(true);
    }

    void testEquality()
    {
        QCOMPARE(1 + 1, 2);
    }
};

int runPlaceholderTests(int argc, char* argv[])
{
    PlaceholderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_placeholder.moc"
