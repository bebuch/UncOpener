#include "MainWindow.hpp"

#include <QLabel>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("UncOpener");
    setMinimumSize(400, 300);

    auto* label = new QLabel("UncOpener - Configuration coming soon", this);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}
