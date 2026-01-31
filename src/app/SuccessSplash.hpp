#ifndef UNCOPENER_SUCCESSSPLASH_HPP
#define UNCOPENER_SUCCESSSPLASH_HPP

#include <QWidget>

class QLabel;
class QTimer;

/// Brief splash window showing success with app icon on green background
class SuccessSplash : public QWidget
{
    Q_OBJECT

public:
    explicit SuccessSplash(QWidget* parent = nullptr);

    /// Show the splash and automatically close after the specified duration
    void showFor(int milliseconds);

signals:
    void finished();

private:
    QTimer* m_timer = nullptr;
    QLabel* m_iconLabel = nullptr;
};

#endif // UNCOPENER_SUCCESSSPLASH_HPP
