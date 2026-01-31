#include "MainWindow.hpp"

#include "AppIcon.hpp"
#include "SecurityPolicy.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_registry(uncopener::SchemeRegistry::create())
{
    setWindowTitle("UncOpener - Configuration");
    setWindowIcon(loadAppIcon());
    setMinimumSize(600, 700);

    setupUi();
    loadConfig();
}

void MainWindow::setupUi()
{
    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    // Scheme name section
    auto* schemeGroup = new QGroupBox("URL Scheme", centralWidget);
    auto* schemeLayout = new QFormLayout(schemeGroup);
    m_schemeNameEdit = new QLineEdit(schemeGroup);
    m_schemeNameEdit->setPlaceholderText("uncopener");
    schemeLayout->addRow("Scheme name:", m_schemeNameEdit);
    connect(m_schemeNameEdit, &QLineEdit::textChanged, this, &MainWindow::onSchemeNameChanged);
    mainLayout->addWidget(schemeGroup);

    // UNC Allow-list section
    auto* uncGroup = new QGroupBox("UNC Allow-List", centralWidget);
    auto* uncLayout = new QVBoxLayout(uncGroup);

    auto* uncHintLabel = new QLabel("If empty, all UNC paths are allowed.", uncGroup);
    uncHintLabel->setStyleSheet("color: gray; font-style: italic;");
    uncLayout->addWidget(uncHintLabel);

    m_uncAllowList = new QListWidget(uncGroup);
    uncLayout->addWidget(m_uncAllowList);

    auto* uncEntryLayout = new QHBoxLayout();
    m_uncEntryEdit = new QLineEdit(uncGroup);
    m_uncEntryEdit->setPlaceholderText(R"(\\server\share)");
    uncEntryLayout->addWidget(m_uncEntryEdit);

    m_addUncButton = new QPushButton("Add", uncGroup);
    m_removeUncButton = new QPushButton("Remove", uncGroup);
    uncEntryLayout->addWidget(m_addUncButton);
    uncEntryLayout->addWidget(m_removeUncButton);
    uncLayout->addLayout(uncEntryLayout);

    connect(m_addUncButton, &QPushButton::clicked, this, &MainWindow::onAddUncEntry);
    connect(m_removeUncButton, &QPushButton::clicked, this, &MainWindow::onRemoveUncEntry);
    connect(m_uncEntryEdit, &QLineEdit::returnPressed, this, &MainWindow::onAddUncEntry);

    mainLayout->addWidget(uncGroup);

    // Filetype policy section - single unified list
    auto* filetypeGroup = new QGroupBox("Filetype Policy", centralWidget);
    auto* filetypeLayout = new QVBoxLayout(filetypeGroup);

    auto* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Mode:", filetypeGroup));
    m_filetypeModeCombo = new QComboBox(filetypeGroup);
    m_filetypeModeCombo->addItem("Whitelist (allow only listed types)");
    m_filetypeModeCombo->addItem("Blacklist (block listed types)");
    modeLayout->addWidget(m_filetypeModeCombo);
    modeLayout->addStretch();
    filetypeLayout->addLayout(modeLayout);
    connect(m_filetypeModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::onFiletypeModeChanged);

    // Single list for either whitelist or blacklist
    m_filetypeListLabel = new QLabel(filetypeGroup);
    filetypeLayout->addWidget(m_filetypeListLabel);

    m_filetypeListWidget = new QListWidget(filetypeGroup);
    m_filetypeListWidget->setMaximumHeight(120);
    filetypeLayout->addWidget(m_filetypeListWidget);

    auto* filetypeEntryLayout = new QHBoxLayout();
    m_filetypeEntryEdit = new QLineEdit(filetypeGroup);
    filetypeEntryLayout->addWidget(m_filetypeEntryEdit);
    m_addFiletypeButton = new QPushButton("Add", filetypeGroup);
    m_removeFiletypeButton = new QPushButton("Remove", filetypeGroup);
    filetypeEntryLayout->addWidget(m_addFiletypeButton);
    filetypeEntryLayout->addWidget(m_removeFiletypeButton);
    filetypeLayout->addLayout(filetypeEntryLayout);

    connect(m_addFiletypeButton, &QPushButton::clicked, this, &MainWindow::onAddFiletypeEntry);
    connect(m_removeFiletypeButton, &QPushButton::clicked, this,
            &MainWindow::onRemoveFiletypeEntry);
    connect(m_filetypeEntryEdit, &QLineEdit::returnPressed, this, &MainWindow::onAddFiletypeEntry);

    mainLayout->addWidget(filetypeGroup);

#ifndef Q_OS_WIN
    // SMB Username section (Linux only)
    auto* smbGroup = new QGroupBox("SMB Settings (Linux)", centralWidget);
    auto* smbLayout = new QFormLayout(smbGroup);
    m_smbUsernameEdit = new QLineEdit(smbGroup);
    m_smbUsernameEdit->setPlaceholderText(R"(Optional: DOMAIN\username or username)");
    smbLayout->addRow("SMB Username:", m_smbUsernameEdit);
    connect(m_smbUsernameEdit, &QLineEdit::textChanged, this, &MainWindow::onSmbUsernameChanged);
    mainLayout->addWidget(smbGroup);
#endif

    // Config path display
    auto* pathGroup = new QGroupBox("Configuration", centralWidget);
    auto* pathLayout = new QVBoxLayout(pathGroup);
    auto* pathFormLayout = new QFormLayout();
    m_configPathLabel = new QLabel(uncopener::Config::configFilePath(), pathGroup);
    m_configPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_configPathLabel->setWordWrap(true);
    m_configPathLabel->setMinimumWidth(400);
    pathFormLayout->addRow("Config file:", m_configPathLabel);
    pathLayout->addLayout(pathFormLayout);
    mainLayout->addWidget(pathGroup);

    // Scheme registration section
    auto* registrationGroup = new QGroupBox("Scheme Registration", centralWidget);
    auto* registrationLayout = new QVBoxLayout(registrationGroup);

    m_registrationStatusLabel = new QLabel(registrationGroup);
    m_registrationStatusLabel->setWordWrap(true);
    registrationLayout->addWidget(m_registrationStatusLabel);

    auto* registrationButtonLayout = new QHBoxLayout();
    m_registerButton = new QPushButton("Register Scheme", registrationGroup);
    m_unregisterButton = new QPushButton("Unregister Scheme", registrationGroup);
    registrationButtonLayout->addWidget(m_registerButton);
    registrationButtonLayout->addWidget(m_unregisterButton);
    registrationButtonLayout->addStretch();
    registrationLayout->addLayout(registrationButtonLayout);

    connect(m_registerButton, &QPushButton::clicked, this, &MainWindow::onRegisterClicked);
    connect(m_unregisterButton, &QPushButton::clicked, this, &MainWindow::onUnregisterClicked);

    mainLayout->addWidget(registrationGroup);

    // Status and save
    auto* bottomLayout = new QHBoxLayout();
    m_statusLabel = new QLabel(centralWidget);
    bottomLayout->addWidget(m_statusLabel);
    bottomLayout->addStretch();
    m_saveButton = new QPushButton("Save", centralWidget);
    m_saveButton->setEnabled(false);
    connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    bottomLayout->addWidget(m_saveButton);
    mainLayout->addLayout(bottomLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::loadConfig()
{
    m_config.load();
    updateUiFromConfig();
    setModified(false);
}

void MainWindow::updateUiFromConfig()
{
    m_schemeNameEdit->setText(m_config.schemeName());

    m_uncAllowList->clear();
    m_uncAllowList->addItems(m_config.uncAllowList());

    m_filetypeModeCombo->setCurrentIndex(
        m_config.filetypeMode() == uncopener::FiletypeMode::Whitelist ? 0 : 1);

    updateFiletypeListFromMode();

#ifndef Q_OS_WIN
    if (m_smbUsernameEdit != nullptr)
    {
        m_smbUsernameEdit->setText(m_config.smbUsername());
    }
#endif

    validateAndUpdateStatus();
    updateRegistrationStatus();
}

void MainWindow::updateFiletypeListFromMode()
{
    m_filetypeListWidget->clear();

    bool isWhitelist = m_filetypeModeCombo->currentIndex() == 0;
    if (isWhitelist)
    {
        m_filetypeListLabel->setText("Allowed extensions (if empty, all types are allowed):");
        m_filetypeEntryEdit->setPlaceholderText(".txt, .pdf, .doc");
        m_filetypeListWidget->addItems(m_config.filetypeWhitelist());
    }
    else
    {
        m_filetypeListLabel->setText("Blocked extensions:");
        m_filetypeEntryEdit->setPlaceholderText(".exe, .bat, .cmd");
        m_filetypeListWidget->addItems(m_config.filetypeBlacklist());
    }

    // Disable mode switching if the current list has entries
    bool hasEntries = m_filetypeListWidget->count() > 0;
    m_filetypeModeCombo->setEnabled(!hasEntries);
}

void MainWindow::updateConfigFromUi()
{
    m_config.setSchemeName(m_schemeNameEdit->text().trimmed());

    QStringList uncList;
    for (int i = 0; i < m_uncAllowList->count(); ++i)
    {
        uncList.append(m_uncAllowList->item(i)->text());
    }
    m_config.setUncAllowList(uncList);

    m_config.setFiletypeMode(m_filetypeModeCombo->currentIndex() == 0
                                 ? uncopener::FiletypeMode::Whitelist
                                 : uncopener::FiletypeMode::Blacklist);

    // Update the appropriate list based on mode
    QStringList filetypeList;
    for (int i = 0; i < m_filetypeListWidget->count(); ++i)
    {
        filetypeList.append(m_filetypeListWidget->item(i)->text());
    }

    if (m_filetypeModeCombo->currentIndex() == 0)
    {
        m_config.setFiletypeWhitelist(filetypeList);
    }
    else
    {
        m_config.setFiletypeBlacklist(filetypeList);
    }

#ifndef Q_OS_WIN
    if (m_smbUsernameEdit != nullptr)
    {
        m_config.setSmbUsername(m_smbUsernameEdit->text().trimmed());
    }
#endif
}

void MainWindow::validateAndUpdateStatus()
{
    QString status;
    bool hasWarnings = false;

    if (m_schemeNameEdit->text().trimmed().isEmpty())
    {
        status = "Warning: Scheme name is empty";
        hasWarnings = true;
    }

    if (hasWarnings)
    {
        m_statusLabel->setStyleSheet("color: orange;");
    }
    else
    {
        status = "Configuration valid";
        m_statusLabel->setStyleSheet("color: green;");
    }

    m_statusLabel->setText(status);
}

void MainWindow::setModified(bool modified)
{
    m_modified = modified;
    m_saveButton->setEnabled(modified);

    QString title = "UncOpener - Configuration";
    if (modified)
    {
        title += " *";
    }
    setWindowTitle(title);
}

void MainWindow::onSaveClicked()
{
    updateConfigFromUi();

    if (m_config.save())
    {
        setModified(false);
        m_statusLabel->setText("Configuration saved");
        m_statusLabel->setStyleSheet("color: green;");
    }
    else
    {
        QMessageBox::warning(this, "Save Failed",
                             "Failed to save configuration to:\n" +
                                 uncopener::Config::configFilePath());
    }
}

void MainWindow::onSchemeNameChanged(const QString& /*text*/)
{
    setModified(true);
    validateAndUpdateStatus();
    updateRegistrationStatus();
}

void MainWindow::onAddUncEntry()
{
    QString entry = m_uncEntryEdit->text().trimmed();
    if (entry.isEmpty())
    {
        return;
    }

    if (!uncopener::UncAllowList::isValidEntry(entry))
    {
        QMessageBox::warning(this, "Invalid Entry",
                             "UNC entries cannot contain forward slashes.\n"
                             R"(Use backslashes instead: \\server\share)");
        return;
    }

    entry = uncopener::UncAllowList::normalizeEntry(entry);
    m_uncAllowList->addItem(entry);
    m_uncEntryEdit->clear();
    setModified(true);
    validateAndUpdateStatus();
}

void MainWindow::onRemoveUncEntry()
{
    auto* item = m_uncAllowList->currentItem();
    if (item != nullptr)
    {
        delete m_uncAllowList->takeItem(m_uncAllowList->row(item));
        setModified(true);
        validateAndUpdateStatus();
    }
}

void MainWindow::onFiletypeModeChanged(int /*index*/)
{
    // Save current list to config before switching
    updateConfigFromUi();
    // Update the list display for the new mode
    updateFiletypeListFromMode();
    setModified(true);
    validateAndUpdateStatus();
}

void MainWindow::onAddFiletypeEntry()
{
    QString entry = m_filetypeEntryEdit->text().trimmed();
    if (entry.isEmpty())
    {
        return;
    }

    if (!uncopener::FiletypePolicy::isValidExtension(entry))
    {
        QMessageBox::warning(this, "Invalid Extension",
                             "Extensions cannot contain path separators (/ or \\).");
        return;
    }

    entry = uncopener::FiletypePolicy::normalizeExtension(entry);
    m_filetypeListWidget->addItem(entry);
    m_filetypeEntryEdit->clear();
    setModified(true);

    // Update config and refresh mode combo state
    updateConfigFromUi();
    m_filetypeModeCombo->setEnabled(m_filetypeListWidget->count() == 0);
}

void MainWindow::onRemoveFiletypeEntry()
{
    auto* item = m_filetypeListWidget->currentItem();
    if (item != nullptr)
    {
        delete m_filetypeListWidget->takeItem(m_filetypeListWidget->row(item));
        setModified(true);

        // Update config and refresh mode combo state
        updateConfigFromUi();
        m_filetypeModeCombo->setEnabled(m_filetypeListWidget->count() == 0);
    }
}

void MainWindow::onSmbUsernameChanged(const QString& /*text*/)
{
    setModified(true);
}

void MainWindow::updateRegistrationStatus()
{
    QString schemeName = m_schemeNameEdit->text().trimmed();

    if (schemeName.isEmpty())
    {
        m_registrationStatusLabel->setText("Enter a scheme name to check registration status.");
        m_registrationStatusLabel->setStyleSheet("");
        m_registerButton->setEnabled(false);
        m_unregisterButton->setEnabled(false);
        return;
    }

    uncopener::RegistrationStatus status = m_registry->checkRegistration(schemeName);

    switch (status)
    {
    case uncopener::RegistrationStatus::NotRegistered:
        m_registrationStatusLabel->setText(
            QString("Scheme '%1' is not registered.").arg(schemeName));
        m_registrationStatusLabel->setStyleSheet("color: gray;");
        m_registerButton->setEnabled(true);
        m_unregisterButton->setEnabled(false);
        break;

    case uncopener::RegistrationStatus::RegisteredToThisBinary:
        m_registrationStatusLabel->setText(
            QString("Scheme '%1' is registered to this application.").arg(schemeName));
        m_registrationStatusLabel->setStyleSheet("color: green;");
        m_registerButton->setEnabled(false);
        m_unregisterButton->setEnabled(true);
        break;

    case uncopener::RegistrationStatus::RegisteredToOtherBinary:
    {
        QString otherPath = m_registry->getRegisteredBinaryPath(schemeName);
        m_registrationStatusLabel->setText(
            QString("Scheme '%1' is registered to another application:\n%2")
                .arg(schemeName, otherPath));
        m_registrationStatusLabel->setStyleSheet("color: orange;");
        m_registerButton->setEnabled(true);
        m_unregisterButton->setEnabled(true);
        break;
    }
    }
}

void MainWindow::onRegisterClicked()
{
    QString schemeName = m_schemeNameEdit->text().trimmed();
    if (schemeName.isEmpty())
    {
        return;
    }

    // Check if registered to another binary
    uncopener::RegistrationStatus status = m_registry->checkRegistration(schemeName);
    if (status == uncopener::RegistrationStatus::RegisteredToOtherBinary)
    {
        QString otherPath = m_registry->getRegisteredBinaryPath(schemeName);
        QMessageBox::StandardButton reply =
            QMessageBox::question(this, "Overwrite Registration?",
                                  QString("Scheme '%1' is currently registered to:\n%2\n\n"
                                          "Do you want to overwrite this registration?")
                                      .arg(schemeName, otherPath),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (reply != QMessageBox::Yes)
        {
            return;
        }
    }

    uncopener::RegistrationResult result = m_registry->registerScheme(schemeName);
    if (result.success)
    {
        QMessageBox::information(this, "Registration Successful",
                                 QString("Scheme '%1' has been registered.").arg(schemeName));
    }
    else
    {
        QMessageBox::warning(
            this, "Registration Failed",
            QString("Failed to register scheme '%1':\n%2").arg(schemeName, result.errorMessage));
    }

    updateRegistrationStatus();
}

void MainWindow::onUnregisterClicked()
{
    QString schemeName = m_schemeNameEdit->text().trimmed();
    if (schemeName.isEmpty())
    {
        return;
    }

    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "Unregister Scheme?",
                              QString("Are you sure you want to unregister scheme '%1'?\n\n"
                                      "Links using this scheme will no longer open automatically.")
                                  .arg(schemeName),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    uncopener::RegistrationResult result = m_registry->unregisterScheme(schemeName);
    if (result.success)
    {
        QMessageBox::information(this, "Unregistration Successful",
                                 QString("Scheme '%1' has been unregistered.").arg(schemeName));
    }
    else
    {
        QMessageBox::warning(
            this, "Unregistration Failed",
            QString("Failed to unregister scheme '%1':\n%2").arg(schemeName, result.errorMessage));
    }

    updateRegistrationStatus();
}
