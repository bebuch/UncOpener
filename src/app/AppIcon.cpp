#include "AppIcon.hpp"

#include <QPainter>
#include <QSvgRenderer>

QIcon loadAppIcon()
{
    QIcon icon;
    QSvgRenderer renderer(QString(":/icons/icon.svg"));
    if (renderer.isValid())
    {
        // Render at multiple sizes for better display at different DPIs
        for (int size : {16, 32, 48, 64, 128})
        {
            QPixmap pixmap(size, size);
            pixmap.fill(Qt::transparent);
            QPainter painter(&pixmap);
            renderer.render(&painter);
            icon.addPixmap(pixmap);
        }
    }
    return icon;
}
