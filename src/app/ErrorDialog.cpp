#include "ErrorDialog.hpp"

#include "AppIcon.hpp"

#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QStyle>
#include <QVBoxLayout>

ErrorDialog::ErrorDialog(const QString& inputUrl, const QString& reason, const QString& remediation,
                         QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("UncOpener - Error");
    setWindowIcon(loadAppIcon());
    setMinimumWidth(450);
    setModal(true);

    QFont boldFont = font();
    boldFont.setBold(true);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(15);

    // Icon and title
    auto* headerLayout = new QHBoxLayout();
    auto* iconLabel = new QLabel(this);
    iconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MessageBoxCritical));
    headerLayout->addWidget(iconLabel);
    auto* titleLabel = new QLabel("Failed to open URL", this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    layout->addLayout(headerLayout);

    // Input URL
    auto* inputGroup = new QVBoxLayout();
    auto* inputHeader = new QLabel("Input:", this);
    inputHeader->setObjectName("inputHeader");
    inputHeader->setFont(boldFont);
    inputGroup->addWidget(inputHeader);
    auto* inputValue = new QLineEdit(inputUrl, this);
    inputValue->setObjectName("inputValue");
    inputValue->setReadOnly(true);
    // Move cursor to start so long URLs show the beginning
    inputValue->setCursorPosition(0);
    inputGroup->addWidget(inputValue);
    layout->addLayout(inputGroup);

    // Reason
    auto* reasonGroup = new QVBoxLayout();
    auto* reasonHeader = new QLabel("Reason:", this);
    reasonHeader->setObjectName("reasonHeader");
    reasonHeader->setFont(boldFont);
    reasonGroup->addWidget(reasonHeader);
    auto* reasonValue = new QLabel(reason, this);
    reasonValue->setObjectName("reasonValue");
    reasonValue->setWordWrap(true);
    reasonGroup->addWidget(reasonValue);
    layout->addLayout(reasonGroup);

    // Remediation
    auto* remediationGroup = new QVBoxLayout();
    auto* remediationHeader = new QLabel("What to do:", this);
    remediationHeader->setObjectName("remediationHeader");
    remediationHeader->setFont(boldFont);
    remediationGroup->addWidget(remediationHeader);
    auto* remediationValue = new QLabel(remediation, this);
    remediationValue->setObjectName("remediationValue");
    remediationValue->setWordWrap(true);
    remediationGroup->addWidget(remediationValue);
    layout->addLayout(remediationGroup);

    layout->addStretch();

    // OK button
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttonBox);
}
