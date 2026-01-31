#include "ErrorDialog.hpp"

#include <QApplication>
#include <QIcon>
#include <QTest>

class DialogsTest : public QObject
{
    Q_OBJECT

private slots:
    void testErrorDialogHasWindowIcon()
    {
        ErrorDialog dialog("test://url", "Test reason", "Test remediation");

        QIcon windowIcon = dialog.windowIcon();
        QVERIFY2(!windowIcon.isNull(), "ErrorDialog window icon should not be null");

        // Verify we can get a pixmap from the icon (proves it loaded correctly)
        QPixmap pixmap = windowIcon.pixmap(32, 32);
        QVERIFY2(!pixmap.isNull(), "ErrorDialog window icon pixmap should not be null");
    }

    void testErrorDialogIconMatchesResource()
    {
        ErrorDialog dialog("test://url", "Test reason", "Test remediation");

        QIcon windowIcon = dialog.windowIcon();
        QIcon expectedIcon(":/icons/icon.svg");

        QVERIFY(!expectedIcon.isNull());
        QVERIFY(!windowIcon.isNull());

        // Compare pixmaps at the same size
        QPixmap windowPixmap = windowIcon.pixmap(64, 64);
        QPixmap expectedPixmap = expectedIcon.pixmap(64, 64);

        QCOMPARE(windowPixmap.toImage(), expectedPixmap.toImage());
    }
};

QTEST_MAIN(DialogsTest)
#include "test_dialogs.moc"
