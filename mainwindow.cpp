#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 使用样式表(QSS)美化界面：区分数字键与运算符键、显示区深色底白字
    setStyleSheet(R"(
        QMainWindow { background-color: #2b2b2b; }
        QLineEdit#display {
            background-color: #1e1e1e;
            color: #ffffff;
            border: 2px solid #3c3c3c;
            border-radius: 8px;
            padding: 0 12px;
        }
        QPushButton {
            font-size: 20px;
            border-radius: 10px;
            border: 1px solid #555555;
            color: #ffffff;
            background-color: #3a3a3a;
        }
        QPushButton:hover { background-color: #4a4a4a; }
        QPushButton:pressed { background-color: #2a2a2a; }
        QPushButton#btnEquals { background-color: #1a73e8; }
        QPushButton#btnEquals:hover { background-color: #2a82f0; }
        QPushButton#btnClear, QPushButton#btnBackspace { background-color: #d9534f; }
    )");

    // 让所有按键不抢占焦点，使键盘事件统一交给主窗口处理
    const QList<QPushButton*> digits = {
        ui->btn0, ui->btn1, ui->btn2, ui->btn3, ui->btn4,
        ui->btn5, ui->btn6, ui->btn7, ui->btn8, ui->btn9
    };
    const QList<QPushButton*> allButtons = digits + QList<QPushButton*>{
        ui->btnDot, ui->btnAdd, ui->btnSub, ui->btnMul, ui->btnDiv,
        ui->btnEquals, ui->btnClear, ui->btnBackspace
    };
    for (QPushButton *b : allButtons)
        b->setFocusPolicy(Qt::NoFocus);
    ui->display->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::StrongFocus);

    // 数字键：鼠标点击与键盘输入复用同一个 digitPressed()
    for (int i = 0; i < digits.size(); ++i) {
        connect(digits.at(i), &QPushButton::clicked, this, [this, i] {
            digitPressed(i);
        });
    }
    connect(ui->btnDot,       &QPushButton::clicked, this, [this] { decimalPressed(); });
    connect(ui->btnAdd,       &QPushButton::clicked, this, [this] { operatorPressed("+"); });
    connect(ui->btnSub,       &QPushButton::clicked, this, [this] { operatorPressed("-"); });
    connect(ui->btnMul,       &QPushButton::clicked, this, [this] { operatorPressed("*"); });
    connect(ui->btnDiv,       &QPushButton::clicked, this, [this] { operatorPressed("/"); });
    connect(ui->btnEquals,    &QPushButton::clicked, this, [this] { equalsPressed(); });
    connect(ui->btnClear,     &QPushButton::clicked, this, [this] { clearPressed(); });
    connect(ui->btnBackspace, &QPushButton::clicked, this, [this] { backspacePressed(); });

    clearPressed(); // 初始显示 0
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------
// 键盘事件：与鼠标按键完全复用同一套处理逻辑
// ---------------------------------------------------------------
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();

    // 数字键（Qt 会把小键盘 0~9 也映射为 Qt::Key_0 ~ Qt::Key_9）
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        digitPressed(key - Qt::Key_0);
        return;
    }

    switch (key) {
    case Qt::Key_Plus:      operatorPressed("+");   break;
    case Qt::Key_Minus:     operatorPressed("-");   break;
    case Qt::Key_Asterisk:  operatorPressed("*");   break;
    case Qt::Key_Slash:     operatorPressed("/");   break;
    case Qt::Key_Period:
    case Qt::Key_Comma:     decimalPressed();       break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Equal:     equalsPressed();        break;
    case Qt::Key_Backspace: backspacePressed();     break;
    case Qt::Key_Escape:
    case Qt::Key_Delete:    clearPressed();         break;
    default:
        QMainWindow::keyPressEvent(event);          // 其余按键交给基类
    }
}

// ---------------------------------------------------------------
// 数字输入
// ---------------------------------------------------------------
void MainWindow::digitPressed(int digit)
{
    if (m_error)              // 出错后必须先清除
        return;

    if (m_waitingForOperand) { // 开始输入新操作数
        m_operand.clear();
        m_waitingForOperand = false;
    }
    if (m_operand == "0")      // 避免出现 "0123" 这类前导 0
        m_operand.clear();
    if (m_operand.length() >= 15) // 限制长度，防止显示溢出
        return;

    m_operand += QString::number(digit);
    updateDisplay();
}

// ---------------------------------------------------------------
// 小数点：保证同一个操作数只出现一次小数点
// ---------------------------------------------------------------
void MainWindow::decimalPressed()
{
    if (m_error)
        return;

    if (m_waitingForOperand) {  // 尚未输入数字时，先补 "0."
        m_operand = "0";
        m_waitingForOperand = false;
    }
    if (!m_operand.contains('.'))   // 关键：已含小数点则忽略，避免重复
        m_operand += '.';
    updateDisplay();
}

// ---------------------------------------------------------------
// 运算符：管理第一个操作数、运算符、第二个操作数
// ---------------------------------------------------------------
void MainWindow::operatorPressed(const QString &op)
{
    if (m_error)
        return;

    if (!m_waitingForOperand) {          // 已经输完一个操作数
        if (m_operator.isEmpty()) {
            m_accumulator = m_operand.toDouble();   // 第一次：记录第一个操作数
        } else {
            double r = 0;
            if (!calculate(m_accumulator, m_operator, m_operand.toDouble(), r)) {
                setError();                         // 连续运算中除零
                return;
            }
            m_accumulator = r;                       // 先算出上一步结果
        }
        m_operator = op;                 // 记下新的运算符
        m_waitingForOperand = true;
        m_operand = formatNumber(m_accumulator);
        updateDisplay();                 // 显示中间结果
    } else {
        // 连续按运算符（如 "3 + * 4"）：只替换运算符
        m_operator = op;
    }
}

// ---------------------------------------------------------------
// 等号
// ---------------------------------------------------------------
void MainWindow::equalsPressed()
{
    if (m_error)
        return;
    if (m_operator.isEmpty() || m_waitingForOperand)
        return;                          // 没有待执行的运算

    double r = 0;
    if (!calculate(m_accumulator, m_operator, m_operand.toDouble(), r)) {
        setError();
        return;
    }
    m_operand = formatNumber(r);
    m_accumulator = r;
    m_operator.clear();
    m_waitingForOperand = true;          // 结果之后可继续输入新数字
    updateDisplay();
}

// ---------------------------------------------------------------
// 清除与退格
// ---------------------------------------------------------------
void MainWindow::clearPressed()
{
    m_accumulator = 0.0;
    m_operator.clear();
    m_operand = "0";
    m_waitingForOperand = true;
    m_error = false;
    updateDisplay();
}

void MainWindow::backspacePressed()
{
    if (m_error || m_waitingForOperand)
        return;
    m_operand.chop(1);
    if (m_operand.isEmpty() || m_operand == "-") {
        m_operand = "0";
        m_waitingForOperand = true;
    }
    updateDisplay();
}

// ---------------------------------------------------------------
// 二元运算：除数为 0 返回 false
// ---------------------------------------------------------------
bool MainWindow::calculate(double a, const QString &op, double b, double &result)
{
    if (op == "+")       result = a + b;
    else if (op == "-")  result = a - b;
    else if (op == "*")  result = a * b;
    else if (op == "/") {
        if (b == 0.0)    return false;    // 除零错误
        result = a / b;
    } else {
        return false;
    }
    return true;
}

QString MainWindow::formatNumber(double v)
{
    // 整数时去掉小数点；否则用 15 位有效数字的科学计数格式输出
    if (std::abs(v) < 1e15 && std::floor(v) == v)
        return QString::number(static_cast<qint64>(v));
    QString s = QString::number(v, 'g', 15);
    return s.isEmpty() ? "0" : s;
}

void MainWindow::updateDisplay()
{
    ui->display->setText(m_operand);
}

void MainWindow::setError()
{
    m_error = true;
    m_operand = "错误";
    updateDisplay();
}
