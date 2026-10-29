#include "finddialog.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

FindReplaceDialog::FindReplaceDialog(bool enableReplace, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(enableReplace ? QStringLiteral("替换") : QStringLiteral("查找"));
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    m_findEdit = new QLineEdit(this);
    m_findEdit->setPlaceholderText(QStringLiteral("输入要查找的内容"));

    m_replaceEdit = new QLineEdit(this);
    m_replaceEdit->setPlaceholderText(QStringLiteral("输入替换后的内容"));

    m_caseBox = new QCheckBox(QStringLiteral("区分大小写"), this);

    QPushButton *findBtn = new QPushButton(QStringLiteral("查找下一个"), this);
    QPushButton *replaceBtn = new QPushButton(QStringLiteral("替换"), this);
    QPushButton *replaceAllBtn = new QPushButton(QStringLiteral("全部替换"), this);
    QPushButton *cancelBtn = new QPushButton(QStringLiteral("关闭"), this);
    findBtn->setDefault(true);

    auto *grid = new QGridLayout;
    grid->addWidget(new QLabel(QStringLiteral("查找内容：")), 0, 0);
    grid->addWidget(m_findEdit, 0, 1);
    if (enableReplace) {
        grid->addWidget(new QLabel(QStringLiteral("替换为：")), 1, 0);
        grid->addWidget(m_replaceEdit, 1, 1);
    }
    grid->addWidget(m_caseBox, 2, 0, 1, 2);

    auto *btnLayout = new QVBoxLayout;
    btnLayout->addWidget(findBtn);
    if (enableReplace) {
        btnLayout->addWidget(replaceBtn);
        btnLayout->addWidget(replaceAllBtn);
    }
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch();

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(grid);
    mainLayout->addLayout(btnLayout);

    // 信号转发
    connect(findBtn, &QPushButton::clicked, this, [this] {
        emit findNext(findText(), matchCase());
    });
    if (enableReplace) {
        connect(replaceBtn, &QPushButton::clicked, this, [this] {
            emit replaceOne(findText(), replaceText(), matchCase());
        });
        connect(replaceAllBtn, &QPushButton::clicked, this, [this] {
            emit replaceAll(findText(), replaceText(), matchCase());
        });
    }
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

QString FindReplaceDialog::findText() const
{
    return m_findEdit->text();
}

QString FindReplaceDialog::replaceText() const
{
    return m_replaceEdit ? m_replaceEdit->text() : QString();
}

bool FindReplaceDialog::matchCase() const
{
    return m_caseBox->isChecked();
}
