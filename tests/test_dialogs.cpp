#include "ErrorDialog.hpp"
#include "MainWindow.hpp"

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

        QPixmap pixmap = windowIcon.pixmap(32, 32);
        QVERIFY2(!pixmap.isNull(), "ErrorDialog window icon pixmap should not be null");
    }

    void testErrorDialogIconHasMultipleSizes()
    {
        ErrorDialog dialog("test://url", "Test reason", "Test remediation");

        QIcon windowIcon = dialog.windowIcon();
        QVERIFY(!windowIcon.isNull());

        QList<QSize> sizes = windowIcon.availableSizes();
        QVERIFY2(sizes.size() >= 3, "ErrorDialog icon should have multiple sizes");
    }

    void testErrorDialogIconMatchesResource()
    {
        ErrorDialog dialog("test://url", "Test reason", "Test remediation");

        QIcon windowIcon = dialog.windowIcon();
        QIcon expectedIcon(":/icons/icon.svg");

        QVERIFY(!expectedIcon.isNull());
        QVERIFY(!windowIcon.isNull());

        QPixmap windowPixmap = windowIcon.pixmap(64, 64);
        QPixmap expectedPixmap = expectedIcon.pixmap(64, 64);

        QCOMPARE(windowPixmap.toImage(), expectedPixmap.toImage());
    }

    void testErrorDialogHasCorrectTitle()
    {
        ErrorDialog dialog("test://url", "Test reason", "Test remediation");
        QCOMPARE(dialog.windowTitle(), QString("UncOpener - Error"));
    }

    void testErrorDialogIsModal()
    {
        ErrorDialog dialog("test://url", "Test reason", "Test remediation");
        QVERIFY(dialog.isModal());
    }

    void testMainWindowHasWindowIcon()
    {
        MainWindow window;

        QIcon windowIcon = window.windowIcon();
        QVERIFY2(!windowIcon.isNull(), "MainWindow window icon should not be null");

        QPixmap pixmap = windowIcon.pixmap(32, 32);
        QVERIFY2(!pixmap.isNull(), "MainWindow window icon pixmap should not be null");
    }

    void testMainWindowIconMatchesResource()
    {
        MainWindow window;

        QIcon windowIcon = window.windowIcon();
        QIcon expectedIcon(":/icons/icon.svg");

        QVERIFY(!expectedIcon.isNull());
        QVERIFY(!windowIcon.isNull());

        QPixmap windowPixmap = windowIcon.pixmap(64, 64);
        QPixmap expectedPixmap = expectedIcon.pixmap(64, 64);

        QCOMPARE(windowPixmap.toImage(), expectedPixmap.toImage());
    }

    void testMainWindowHasCorrectTitle()
    {
        MainWindow window;
        QCOMPARE(window.windowTitle(), QString("UncOpener - Configuration"));
    }

    void testMainWindowHasMinimumSize()
    {
        MainWindow window;
        QVERIFY(window.minimumWidth() >= 600);
        QVERIFY(window.minimumHeight() >= 700);
    }
};

QTEST_MAIN(DialogsTest)
#include "test_dialogs.moc"
