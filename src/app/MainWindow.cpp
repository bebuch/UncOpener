#include "MainWindow.hpp"

#include "SecurityPolicy.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("UncOpener - Configuration");
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
    m_schemeNameEdit->setPlaceholderText("unc");
    schemeLayout->addRow("Scheme name:", m_schemeNameEdit);
    connect(m_schemeNameEdit, &QLineEdit::textChanged, this, &MainWindow::onSchemeNameChanged);
    mainLayout->addWidget(schemeGroup);

    // UNC Allow-list section
    auto* uncGroup = new QGroupBox("UNC Allow-List", centralWidget);
    auto* uncLayout = new QVBoxLayout(uncGroup);

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

    // Filetype policy section
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

    // Whitelist
    auto* whitelistLayout = new QVBoxLayout();
    whitelistLayout->addWidget(new QLabel("Whitelist (allowed extensions):"));
    m_whitelistWidget = new QListWidget(filetypeGroup);
    m_whitelistWidget->setMaximumHeight(100);
    whitelistLayout->addWidget(m_whitelistWidget);

    auto* whitelistEntryLayout = new QHBoxLayout();
    m_whitelistEntryEdit = new QLineEdit(filetypeGroup);
    m_whitelistEntryEdit->setPlaceholderText(".txt, .pdf, .doc");
    whitelistEntryLayout->addWidget(m_whitelistEntryEdit);
    m_addWhitelistButton = new QPushButton("Add", filetypeGroup);
    m_removeWhitelistButton = new QPushButton("Remove", filetypeGroup);
    whitelistEntryLayout->addWidget(m_addWhitelistButton);
    whitelistEntryLayout->addWidget(m_removeWhitelistButton);
    whitelistLayout->addLayout(whitelistEntryLayout);
    filetypeLayout->addLayout(whitelistLayout);

    connect(m_addWhitelistButton, &QPushButton::clicked, this, &MainWindow::onAddWhitelistEntry);
    connect(m_removeWhitelistButton, &QPushButton::clicked, this,
            &MainWindow::onRemoveWhitelistEntry);
    connect(m_whitelistEntryEdit, &QLineEdit::returnPressed, this,
            &MainWindow::onAddWhitelistEntry);

    // Blacklist
    auto* blacklistLayout = new QVBoxLayout();
    blacklistLayout->addWidget(new QLabel("Blacklist (blocked extensions):"));
    m_blacklistWidget = new QListWidget(filetypeGroup);
    m_blacklistWidget->setMaximumHeight(100);
    blacklistLayout->addWidget(m_blacklistWidget);

    auto* blacklistEntryLayout = new QHBoxLayout();
    m_blacklistEntryEdit = new QLineEdit(filetypeGroup);
    m_blacklistEntryEdit->setPlaceholderText(".exe, .bat, .cmd");
    blacklistEntryLayout->addWidget(m_blacklistEntryEdit);
    m_addBlacklistButton = new QPushButton("Add", filetypeGroup);
    m_removeBlacklistButton = new QPushButton("Remove", filetypeGroup);
    blacklistEntryLayout->addWidget(m_addBlacklistButton);
    blacklistEntryLayout->addWidget(m_removeBlacklistButton);
    blacklistLayout->addLayout(blacklistEntryLayout);
    filetypeLayout->addLayout(blacklistLayout);

    connect(m_addBlacklistButton, &QPushButton::clicked, this, &MainWindow::onAddBlacklistEntry);
    connect(m_removeBlacklistButton, &QPushButton::clicked, this,
            &MainWindow::onRemoveBlacklistEntry);
    connect(m_blacklistEntryEdit, &QLineEdit::returnPressed, this,
            &MainWindow::onAddBlacklistEntry);

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
    auto* pathLayout = new QFormLayout(pathGroup);
    m_configPathLabel = new QLabel(uncopener::Config::configFilePath(), pathGroup);
    m_configPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_configPathLabel->setWordWrap(true);
    pathLayout->addRow("Config file:", m_configPathLabel);
    mainLayout->addWidget(pathGroup);

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

    m_whitelistWidget->clear();
    m_whitelistWidget->addItems(m_config.filetypeWhitelist());

    m_blacklistWidget->clear();
    m_blacklistWidget->addItems(m_config.filetypeBlacklist());

#ifndef Q_OS_WIN
    if (m_smbUsernameEdit != nullptr)
    {
        m_smbUsernameEdit->setText(m_config.smbUsername());
    }
#endif

    validateAndUpdateStatus();
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

    QStringList whitelist;
    for (int i = 0; i < m_whitelistWidget->count(); ++i)
    {
        whitelist.append(m_whitelistWidget->item(i)->text());
    }
    m_config.setFiletypeWhitelist(whitelist);

    QStringList blacklist;
    for (int i = 0; i < m_blacklistWidget->count(); ++i)
    {
        blacklist.append(m_blacklistWidget->item(i)->text());
    }
    m_config.setFiletypeBlacklist(blacklist);

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
    else if (m_uncAllowList->count() == 0)
    {
        status = "Warning: UNC allow-list is empty - no paths will be allowed";
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
    setModified(true);
    validateAndUpdateStatus();
}

void MainWindow::onAddWhitelistEntry()
{
    QString entry = m_whitelistEntryEdit->text().trimmed();
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
    m_whitelistWidget->addItem(entry);
    m_whitelistEntryEdit->clear();
    setModified(true);
}

void MainWindow::onRemoveWhitelistEntry()
{
    auto* item = m_whitelistWidget->currentItem();
    if (item != nullptr)
    {
        delete m_whitelistWidget->takeItem(m_whitelistWidget->row(item));
        setModified(true);
    }
}

void MainWindow::onAddBlacklistEntry()
{
    QString entry = m_blacklistEntryEdit->text().trimmed();
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
    m_blacklistWidget->addItem(entry);
    m_blacklistEntryEdit->clear();
    setModified(true);
}

void MainWindow::onRemoveBlacklistEntry()
{
    auto* item = m_blacklistWidget->currentItem();
    if (item != nullptr)
    {
        delete m_blacklistWidget->takeItem(m_blacklistWidget->row(item));
        setModified(true);
    }
}

void MainWindow::onSmbUsernameChanged(const QString& /*text*/)
{
    setModified(true);
}
