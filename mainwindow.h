#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CodeEditor;
class FindReplaceDialog;

// 简单文本编辑器主窗口
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;  // 关闭前检查未保存修改

private slots:
    // 文件
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    // 编辑
    void openFindDialog();
    void openReplaceDialog();
    // 格式
    void increaseFontSize();
    void decreaseFontSize();
    void setFontColor();
    void setBackgroundColor();
    void toggleWordWrap(bool wrap);
    // 视图
    void toggleLineNumbers(bool show);
    void toggleToolbar(bool show);
    void toggleStatusbar(bool show);
    // 帮助
    void about();
    // 编辑器状态同步
    void onTextChanged();
    void updateActions();
    void updateStatusBar();

private:
    bool maybeSave();                              // 未保存修改时弹出提示
    void loadFile(const QString &path);            // 打开文件
    void setCurrentFile(const QString &path);      // 记录当前文件并更新标题
    bool doFind(const QString &text, bool matchCase);   // 查找下一个，返回是否找到
    void doReplaceOne(const QString &find, const QString &repl, bool matchCase);
    int  doReplaceAll(const QString &find, const QString &repl, bool matchCase);
    void initFindDialog(FindReplaceDialog *dlg, bool replaceMode);

    Ui::MainWindow *ui;
    CodeEditor *m_editor = nullptr;
    QString m_currentFile;          // 当前文件完整路径（空表示未命名）
    bool m_modified = false;        // 是否有未保存修改
    FindReplaceDialog *m_findDialog = nullptr;
    FindReplaceDialog *m_replaceDialog = nullptr;
};

#endif // MAINWINDOW_H
