#include "ErrorDialog.hpp"

#include <QDialogButtonBox>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

ErrorDialog::ErrorDialog(const QString& inputUrl, const QString& reason, const QString& remediation,
                         QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("UncOpener - Error");
    setMinimumWidth(450);
    setModal(true);

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
    inputHeader->setStyleSheet("font-weight: bold;");
    inputGroup->addWidget(inputHeader);
    auto* inputValue = new QLabel(inputUrl, this);
    inputValue->setWordWrap(true);
    inputValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    inputValue->setStyleSheet("background-color: #f0f0f0; padding: 8px; border-radius: 4px;");
    inputGroup->addWidget(inputValue);
    layout->addLayout(inputGroup);

    // Reason
    auto* reasonGroup = new QVBoxLayout();
    auto* reasonHeader = new QLabel("Reason:", this);
    reasonHeader->setStyleSheet("font-weight: bold;");
    reasonGroup->addWidget(reasonHeader);
    auto* reasonValue = new QLabel(reason, this);
    reasonValue->setWordWrap(true);
    reasonGroup->addWidget(reasonValue);
    layout->addLayout(reasonGroup);

    // Remediation
    auto* remediationGroup = new QVBoxLayout();
    auto* remediationHeader = new QLabel("What to do:", this);
    remediationHeader->setStyleSheet("font-weight: bold;");
    remediationGroup->addWidget(remediationHeader);
    auto* remediationValue = new QLabel(remediation, this);
    remediationValue->setWordWrap(true);
    remediationGroup->addWidget(remediationValue);
    layout->addLayout(remediationGroup);

    layout->addStretch();

    // OK button
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttonBox);
}
