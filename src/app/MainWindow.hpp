#ifndef UNCOPENER_MAINWINDOW_HPP
#define UNCOPENER_MAINWINDOW_HPP

#include "Config.hpp"
#include "SchemeRegistry.hpp"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>

#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onSaveClicked();
    void onSchemeNameChanged(const QString& text);
    void onAddUncEntry();
    void onRemoveUncEntry();
    void onFiletypeModeChanged(int index);
    void onAddWhitelistEntry();
    void onRemoveWhitelistEntry();
    void onAddBlacklistEntry();
    void onRemoveBlacklistEntry();
    void onSmbUsernameChanged(const QString& text);
    void onRegisterClicked();
    void onUnregisterClicked();

private:
    void setupUi();
    void loadConfig();
    void updateUiFromConfig();
    void updateConfigFromUi();
    void validateAndUpdateStatus();
    void updateRegistrationStatus();
    void setModified(bool modified);

    uncopener::Config m_config;
    std::unique_ptr<uncopener::SchemeRegistry> m_registry;
    bool m_modified = false;

    // Widgets
    QLineEdit* m_schemeNameEdit = nullptr;
    QListWidget* m_uncAllowList = nullptr;
    QLineEdit* m_uncEntryEdit = nullptr;
    QPushButton* m_addUncButton = nullptr;
    QPushButton* m_removeUncButton = nullptr;

    QComboBox* m_filetypeModeCombo = nullptr;
    QListWidget* m_whitelistWidget = nullptr;
    QLineEdit* m_whitelistEntryEdit = nullptr;
    QPushButton* m_addWhitelistButton = nullptr;
    QPushButton* m_removeWhitelistButton = nullptr;

    QListWidget* m_blacklistWidget = nullptr;
    QLineEdit* m_blacklistEntryEdit = nullptr;
    QPushButton* m_addBlacklistButton = nullptr;
    QPushButton* m_removeBlacklistButton = nullptr;

    QLineEdit* m_smbUsernameEdit = nullptr;
    QLabel* m_configPathLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QPushButton* m_saveButton = nullptr;

    // Registration widgets
    QLabel* m_registrationStatusLabel = nullptr;
    QPushButton* m_registerButton = nullptr;
    QPushButton* m_unregisterButton = nullptr;
};

#endif // UNCOPENER_MAINWINDOW_HPP
