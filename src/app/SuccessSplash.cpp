#include "SuccessSplash.hpp"

#include <QGuiApplication>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#include <QSvgRenderer>
#include <QTimer>
#include <QVBoxLayout>

SuccessSplash::SuccessSplash(QWidget* parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, false);

    // Green background
    setStyleSheet("background-color: #4CAF50;");
    setFixedSize(200, 200);

    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(128, 128);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    // Load the SVG icon
    QSvgRenderer renderer(QString(":/icons/icon.svg"));
    if (renderer.isValid())
    {
        QPixmap pixmap(128, 128);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        m_iconLabel->setPixmap(pixmap);
    }
    else
    {
        // Fallback: show a checkmark
        m_iconLabel->setText("✓");
        QFont font = m_iconLabel->font();
        font.setPointSize(72);
        m_iconLabel->setFont(font);
        m_iconLabel->setStyleSheet("color: white;");
    }

    layout->addWidget(m_iconLabel);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this,
            [this]()
            {
                close();
                emit finished();
            });
}

void SuccessSplash::showFor(int milliseconds)
{
    // Center on screen
    if (auto* screen = QGuiApplication::primaryScreen())
    {
        QRect screenGeometry = screen->availableGeometry();
        int xPos = (screenGeometry.width() - width()) / 2 + screenGeometry.x();
        int yPos = (screenGeometry.height() - height()) / 2 + screenGeometry.y();
        move(xPos, yPos);
    }

    show();
    m_timer->start(milliseconds);
}
