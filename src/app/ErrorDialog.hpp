#ifndef UNCOPENER_ERRORDIALOG_HPP
#define UNCOPENER_ERRORDIALOG_HPP

#include <QDialog>
#include <QString>

class QLabel;

/// Modal dialog for displaying errors in handler mode
class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(const QString& inputUrl, const QString& reason, const QString& remediation,
                         QWidget* parent = nullptr);
};

#endif // UNCOPENER_ERRORDIALOG_HPP
