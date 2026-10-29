#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"
#include "finddialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QColorDialog>
#include <QFontDialog>
#include <QCloseEvent>
#include <QStyle>
#include <QPushButton>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_editor = ui->editor;   // 提升后的 CodeEditor

    // 工具栏图标：使用 Qt 标准图标，保持工具栏整洁
    ui->actNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    ui->actOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->actUndo->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    ui->actRedo->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    ui->actCut->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    ui->actCopy->setIcon(style()->standardIcon(QStyle::SP_DialogYesButton));

    // 文件
    connect(ui->actNew,     &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actOpen,    &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actSave,    &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actSaveAs,  &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actExit,    &QAction::triggered, this, &QWidget::close);
    // 编辑
    connect(ui->actUndo,    &QAction::triggered, m_editor, &QPlainTextEdit::undo);
    connect(ui->actRedo,    &QAction::triggered, m_editor, &QPlainTextEdit::redo);
    connect(ui->actCut,     &QAction::triggered, m_editor, &QPlainTextEdit::cut);
    connect(ui->actCopy,    &QAction::triggered, m_editor, &QPlainTextEdit::copy);
    connect(ui->actPaste,   &QAction::triggered, m_editor, &QPlainTextEdit::paste);
    connect(ui->actSelectAll, &QAction::triggered, m_editor, &QPlainTextEdit::selectAll);
    connect(ui->actFind,    &QAction::triggered, this, &MainWindow::openFindDialog);
    connect(ui->actReplace, &QAction::triggered, this, &MainWindow::openReplaceDialog);
    // 格式
    connect(ui->actFontInc,  &QAction::triggered, this, &MainWindow::increaseFontSize);
    connect(ui->actFontDec,  &QAction::triggered, this, &MainWindow::decreaseFontSize);
    connect(ui->actFontColor, &QAction::triggered, this, &MainWindow::setFontColor);
    connect(ui->actBgColor,   &QAction::triggered, this, &MainWindow::setBackgroundColor);
    connect(ui->actWordWrap,  &QAction::toggled,   this, &MainWindow::toggleWordWrap);
    // 视图
    connect(ui->actLineNumbers, &QAction::toggled, this, &MainWindow::toggleLineNumbers);
    connect(ui->actToolbar,     &QAction::toggled, this, &MainWindow::toggleToolbar);
    connect(ui->actStatusbar,   &QAction::toggled, this, &MainWindow::toggleStatusbar);
    // 帮助
    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::about);

    // 编辑器内容变化
    connect(m_editor, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(m_editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(m_editor, &QPlainTextEdit::undoAvailable, this, [this](bool ok){ ui->actUndo->setEnabled(ok); });
    connect(m_editor, &QPlainTextEdit::redoAvailable, this, [this](bool ok){ ui->actRedo->setEnabled(ok); });
    connect(m_editor, &QPlainTextEdit::copyAvailable, this, &MainWindow::updateActions);

    // 默认字体
    QFont f = m_editor->font();
    f.setPointSize(12);
    m_editor->setFont(f);

    setCurrentFile(QString());
    updateActions();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------
// 未保存修改时弹出提示；返回 false 表示用户取消当前操作
// ---------------------------------------------------------------
bool MainWindow::maybeSave()
{
    if (!m_modified)
        return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("文本编辑器"),
        tr("文档已被修改，是否保存更改？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;   // 不保存
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// ---------------------------------------------------------------
// 文件操作
// ---------------------------------------------------------------
void MainWindow::newFile()
{
    if (!maybeSave())
        return;
    m_editor->clear();
    setCurrentFile(QString());
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;
    QString path = QFileDialog::getOpenFileName(this, tr("打开文件"), QString(),
        tr("文本文件 (*.txt);;所有文件 (*)"));
    if (path.isEmpty())
        return;
    loadFile(path);
}

void MainWindow::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 打开不存在或异常文件时给出合理提示
        QMessageBox::warning(this, tr("文本编辑器"),
                             tr("无法打开文件：%1\n%2").arg(path, file.errorString()));
        return;
    }
    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif
    m_editor->setPlainText(in.readAll());
    file.close();
    setCurrentFile(path);
}

bool MainWindow::saveFile()
{
    if (m_currentFile.isEmpty())
        return saveFileAs();
    QFile file(m_currentFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("文本编辑器"),
                             tr("无法保存文件：%1\n%2").arg(m_currentFile, file.errorString()));
        return false;
    }
    QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
    out << m_editor->toPlainText();
    file.close();
    setCurrentFile(m_currentFile);   // 清除修改标记并更新标题
    return true;
}

bool MainWindow::saveFileAs()
{
    QString path = QFileDialog::getSaveFileName(this, tr("另存为"), QString(),
        tr("文本文件 (*.txt);;所有文件 (*)"));
    if (path.isEmpty())
        return false;
    m_currentFile = path;   // 临时记录，saveFile 会写入
    return saveFile();
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    m_modified = false;
    m_editor->document()->setModified(false);

    QString title = m_currentFile.isEmpty()
        ? tr("未命名")
        : QFileInfo(m_currentFile).fileName();
    setWindowTitle(tr("%1 - 文本编辑器").arg(title));
}

// ---------------------------------------------------------------
// 编辑
// ---------------------------------------------------------------
void MainWindow::openFindDialog()
{
    if (!m_findDialog) {
        m_findDialog = new FindReplaceDialog(false, this);
        connect(m_findDialog, &FindReplaceDialog::findNext, this,
                [this](const QString &text, bool cs){ doFind(text, cs); });
    }
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::openReplaceDialog()
{
    if (!m_replaceDialog) {
        m_replaceDialog = new FindReplaceDialog(true, this);
        connect(m_replaceDialog, &FindReplaceDialog::findNext, this,
                [this](const QString &text, bool cs){ doFind(text, cs); });
        connect(m_replaceDialog, &FindReplaceDialog::replaceOne, this,
                [this](const QString &f, const QString &r, bool cs){ doReplaceOne(f, r, cs); });
        connect(m_replaceDialog, &FindReplaceDialog::replaceAll, this,
                [this](const QString &f, const QString &r, bool cs){ doReplaceAll(f, r, cs); });
    }
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

bool MainWindow::doFind(const QString &text, bool matchCase)
{
    if (text.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (matchCase)
        flags |= QTextDocument::FindCaseSensitively;

    bool found = m_editor->find(text, flags);
    if (!found) {
        // 未找到：询问是否从头再找
        QMessageBox::StandardButton ret = QMessageBox::information(
            this, tr("查找"), tr("未找到 \"%1\"。是否从头开始查找？").arg(text),
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            m_editor->moveCursor(QTextCursor::Start);
            found = m_editor->find(text, flags);
        }
    }
    if (!found)
        QMessageBox::information(this, tr("查找"), tr("未找到 \"%1\"。").arg(text));
    return found;
}

void MainWindow::doReplaceOne(const QString &find, const QString &repl, bool matchCase)
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == find) {
        cursor.insertText(repl);
    }
    doFind(find, matchCase);
}

int MainWindow::doReplaceAll(const QString &find, const QString &repl, bool matchCase)
{
    if (find.isEmpty())
        return 0;
    int count = 0;
    QTextCursor cursor(m_editor->document());
    cursor.movePosition(QTextCursor::Start);

    QTextDocument::FindFlags flags;
    if (matchCase)
        flags |= QTextDocument::FindCaseSensitively;

    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = m_editor->document()->find(find, cursor, flags);
        if (cursor.isNull())
            break;
        cursor.insertText(repl);
        ++count;
    }
    QMessageBox::information(this, tr("替换"),
                             tr("共替换 %1 处。").arg(count));
    return count;
}

// ---------------------------------------------------------------
// 格式
// ---------------------------------------------------------------
void MainWindow::increaseFontSize()
{
    QFont f = m_editor->font();
    f.setPointSize(f.pointSize() + 1);
    m_editor->setFont(f);
}

void MainWindow::decreaseFontSize()
{
    QFont f = m_editor->font();
    f.setPointSize(qMax(1, f.pointSize() - 1));
    m_editor->setFont(f);
}

void MainWindow::setFontColor()
{
    QColor c = QColorDialog::getColor(
        m_editor->currentCharFormat().foreground().color(), this, tr("字体颜色"));
    if (c.isValid()) {
        // QPlainTextEdit 通过字符格式设置前景色（作用于当前光标或选区）
        QTextCharFormat fmt;
        fmt.setForeground(c);
        m_editor->mergeCurrentCharFormat(fmt);
    }
}

void MainWindow::setBackgroundColor()
{
    QColor c = QColorDialog::getColor(m_editor->palette().base().color(), this, tr("背景颜色"));
    if (c.isValid()) {
        QPalette p = m_editor->palette();
        p.setColor(QPalette::Base, c);
        m_editor->setPalette(p);
    }
}

void MainWindow::toggleWordWrap(bool wrap)
{
    m_editor->setLineWrapMode(wrap ? QPlainTextEdit::WidgetWidth
                                   : QPlainTextEdit::NoWrap);
}

// ---------------------------------------------------------------
// 视图
// ---------------------------------------------------------------
void MainWindow::toggleLineNumbers(bool show)
{
    m_editor->setLineNumbersVisible(show);
}

void MainWindow::toggleToolbar(bool show)
{
    ui->mainToolBar->setVisible(show);
}

void MainWindow::toggleStatusbar(bool show)
{
    ui->statusBar->setVisible(show);
}

// ---------------------------------------------------------------
// 帮助
// ---------------------------------------------------------------
void MainWindow::about()
{
    QMessageBox::about(this, tr("关于文本编辑器"),
        tr("<b>简单文本编辑器</b><br/>Qt 6 课程实验二<br/>"
           "支持文件读写、编辑、查找/替换、字体与背景设置等。"));
}

// ---------------------------------------------------------------
// 状态同步
// ---------------------------------------------------------------
void MainWindow::onTextChanged()
{
    bool modified = m_editor->document()->isModified();
    if (modified != m_modified) {
        m_modified = modified;
        // 根据文件修改状态，自动修改窗口标题（在标题前加 *）
        QString title = m_currentFile.isEmpty()
            ? tr("未命名")
            : QFileInfo(m_currentFile).fileName();
        setWindowTitle(QString("%1%2 - 文本编辑器")
                           .arg(m_modified ? "*" : "", title));
    }
    updateActions();
}

void MainWindow::updateActions()
{
    bool hasSelection = m_editor->textCursor().hasSelection();
    ui->actCut->setEnabled(hasSelection);
    ui->actCopy->setEnabled(hasSelection);
    // 粘贴：剪贴板有文本时可用
    ui->actPaste->setEnabled(m_editor->canPaste());
}

void MainWindow::updateStatusBar()
{
    QTextCursor cursor = m_editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    QString msg = tr("第 %1 行，第 %2 列").arg(line).arg(col);
    if (m_modified)
        msg += tr("    已修改");
    ui->statusBar->showMessage(msg);
}
