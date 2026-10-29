#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 带键盘事件的计算器主窗口
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 键盘事件：把按键映射到与鼠标点击相同的处理函数，复用同一套逻辑
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void digitPressed(int digit);            // 数字 0~9
    void decimalPressed();                   // 小数点
    void operatorPressed(const QString &op); // 运算符 + - * /
    void equalsPressed();                    // 等号
    void clearPressed();                     // 清除 (C)
    void backspacePressed();                 // 退格

private:
    // 执行一次二元运算 a op b，成功返回 true；除数为 0 时返回 false
    bool calculate(double a, const QString &op, double b, double &result);
    // 把计算结果转换成简洁的字符串（去掉多余的 0）
    QString formatNumber(double v);
    void updateDisplay();   // 把当前操作数显示到 QLineEdit
    void setError();        // 进入除零错误状态

    Ui::MainWindow *ui;

    // 计算器状态
    double  m_accumulator = 0.0;    // 已累计的结果 / 第一个操作数
    QString m_operator;             // 待执行的运算符
    QString m_operand;              // 正在输入的操作数（字符串形式）
    bool    m_waitingForOperand = true; // 下一次数字输入是否开启一个新操作数
    bool    m_error = false;        // 是否处于除零错误状态
};

#endif // MAINWINDOW_H
