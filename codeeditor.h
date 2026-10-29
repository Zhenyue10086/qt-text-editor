#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

// 从 QPlainTextEdit 派生，在左侧增加行号区域（参考资料3的做法）
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth();

    bool lineNumbersVisible() const;
    void setLineNumbersVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *m_lineNumberArea;
    bool m_lineNumbersVisible = true;
};

#endif // CODEEDITOR_H
