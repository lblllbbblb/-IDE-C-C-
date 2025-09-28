#ifndef DEBUGGERMANAGER_H
#define DEBUGGERMANAGER_H

#include <QObject>
#include <QProcess>
#include <QTextEdit>
#include <QMap>
#include <QList>

class DebuggerManager : public QObject
{
    Q_OBJECT

public:
    explicit DebuggerManager(QObject *parent = nullptr); // 构造函数
    ~DebuggerManager(); // 析构函数

    void startDebugging(const QString &executablePath); // 开始调试
    void stopDebugging(); // 停止调试
    void setBreakpoint(const QString &filePath, int lineNumber); // 设置断点
    void clearBreakpoint(const QString &filePath, int lineNumber); // 清除断点
    void toggleBreakpoint(const QString &filePath, int lineNumber); // 切换断点
    void continueExecution(); // 继续执行
    void stepInto(); // 步入
    void stepOver(); // 步过
    void stepOut(); // 步出
    void inspectVariable(const QString &variableName); // 检查变量
    void setOutputWidget(QTextEdit *output); // 设置输出窗口

    QString getGdbPath() const; // 获取 GDB 路径

    // 断点相关成员
    QMap<QString, QList<int>> m_breakpoints; // 断点列表
    QMap<QString, QList<int>> m_gdbBreakpointIds; // GDB 断点 ID

signals:
    void debuggerStarted(); // 调试器启动信号
    void debuggerStopped(); // 调试器停止信号
    void hitBreakpoint(const QString &filePath, int lineNumber); // 命中断点信号
    void currentLineChanged(const QString &filePath, int lineNumber); // 当前行改变信号
    void outputReceived(const QString &text); // 收到输出信号

private slots:
    void onGdbReadyReadStandardOutput(); // GDB 标准输出就绪槽
    void onGdbReadyReadStandardError(); // GDB 标准错误就绪槽
    void onGdbFinished(int exitCode, QProcess::ExitStatus exitStatus); // GDB 进程结束槽

private:
    void sendCommandToGdb(const QString &command); // 发送命令到 GDB
    void appendToOutput(const QString &text, const QColor &color = QColor("#a9b7c6")); // 追加到输出窗口

    QProcess *m_gdbProcess; // GDB 进程
    QTextEdit *m_outputWidget; // 输出窗口
};

#endif // DEBUGGERMANAGER_H
