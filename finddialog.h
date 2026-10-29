#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;

// 查找 / 替换对话框：enableReplace=true 时显示替换相关控件
class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(bool enableReplace, QWidget *parent = nullptr);

    QString findText() const;
    QString replaceText() const;
    bool    matchCase() const;

signals:
    void findNext(const QString &text, bool matchCase);
    void replaceOne(const QString &find, const QString &repl, bool matchCase);
    void replaceAll(const QString &find, const QString &repl, bool matchCase);

private:
    QLineEdit *m_findEdit = nullptr;
    QLineEdit *m_replaceEdit = nullptr;
    QCheckBox *m_caseBox = nullptr;
};

#endif // FINDDIALOG_H
