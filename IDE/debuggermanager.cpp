#include "DebuggerManager.h"
#include <QProcess>
#include <QMessageBox>
#include <QTextCursor>
#include <QRegularExpression>
#include <QDebug>

// 构造函数
DebuggerManager::DebuggerManager(QObject *parent)
    : QObject(parent), m_gdbProcess(nullptr), m_outputWidget(nullptr)
{
    // 初始化断点数据结构
    m_breakpoints.clear();
    m_gdbBreakpointIds.clear();
}

// 析构函数
DebuggerManager::~DebuggerManager()
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        m_gdbProcess->kill(); // 确保 GDB 进程被终止
        m_gdbProcess->waitForFinished(1000); // 等待一小段时间
    }
    delete m_gdbProcess;
}

// 获取 GDB 路径
QString DebuggerManager::getGdbPath() const
{
#ifdef Q_OS_WIN
    return "gdb.exe"; // 假设 GDB 在 Windows 上路径为 gdb.exe
#else
    return "gdb"; // Linux/macOS 上通常直接是 gdb
#endif
}

// 开始调试
void DebuggerManager::startDebugging(const QString &executablePath)
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        appendToOutput("调试器已经在运行中。请先停止当前的调试会话。\n", Qt::red);
        return;
    }

    if (executablePath.isEmpty()) {
        appendToOutput("没有可执行文件可供调试。请先编译。\n", Qt::red);
        return;
    }

    m_gdbProcess = new QProcess(this);
    connect(m_gdbProcess, &QProcess::readyReadStandardOutput, this, &DebuggerManager::onGdbReadyReadStandardOutput);
    connect(m_gdbProcess, &QProcess::readyReadStandardError, this, &DebuggerManager::onGdbReadyReadStandardError);
    connect(m_gdbProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DebuggerManager::onGdbFinished);

    QString gdbPath = getGdbPath();
    QStringList args;
    args << "--interpreter=mi2"; // 启用 Machine Interface (MI) 模式

    m_gdbProcess->start(gdbPath, args);

    if (!m_gdbProcess->waitForStarted()) {
        appendToOutput("无法启动 GDB 进程。请检查 GDB 是否安装并配置正确。\n", Qt::red);
        delete m_gdbProcess;
        m_gdbProcess = nullptr;
        return;
    }

    // 加载可执行文件
    sendCommandToGdb(QString("-file-exec-and-symbols \"%1\"").arg(executablePath));
    sendCommandToGdb("-enable-pretty-printing"); // 启用美化打印，改善变量显示

    // 设置所有之前记录的断点
    for (const QString &filePath : m_breakpoints.keys()) {
        for (int lineNumber : m_breakpoints.value(filePath)) {
            sendCommandToGdb(QString("-break-insert \"%1:%2\"").arg(filePath).arg(lineNumber));
        }
    }

    // 启动程序
    sendCommandToGdb("-exec-run");
    appendToOutput("调试已启动...\n", Qt::green);
}

// 停止调试
void DebuggerManager::stopDebugging()
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb("-gdb-exit");
        m_gdbProcess->waitForFinished(2000);
        if (m_gdbProcess->state() != QProcess::NotRunning) {
            m_gdbProcess->kill();
        }
        delete m_gdbProcess;
        m_gdbProcess = nullptr;
        appendToOutput("调试已停止。\n", Qt::green);
    }
}

// 设置断点
void DebuggerManager::setBreakpoint(const QString &filePath, int lineNumber)
{
    // 添加断点到断点管理器
    m_breakpoints[filePath].append(lineNumber);

    // 如果 GDB 进程已启动，则添加到 GDB
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb(QString("-break-insert \"%1:%2\"").arg(filePath).arg(lineNumber));
    }

    // 假设 GDB 返回的断点 ID 为整数，存储在 m_gdbBreakpointIds
    int breakpointId = m_gdbBreakpointIds[filePath].size() + 1;
    m_gdbBreakpointIds[filePath].append(breakpointId);

    appendToOutput(QString("断点已设置：%1:%2\n").arg(filePath).arg(lineNumber), Qt::blue);
}

// 清除断点
void DebuggerManager::clearBreakpoint(const QString &filePath, int lineNumber)
{
    if (m_breakpoints.contains(filePath)) {
        m_breakpoints[filePath].removeAll(lineNumber);
    }

    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb(QString("-break-delete \"%1:%2\"").arg(filePath).arg(lineNumber));
    }

    appendToOutput(QString("断点已清除：%1:%2\n").arg(filePath).arg(lineNumber), Qt::red);
}

// 切换断点（设置或清除）
void DebuggerManager::toggleBreakpoint(const QString &filePath, int lineNumber)
{
    if (m_breakpoints.contains(filePath) && m_breakpoints[filePath].contains(lineNumber)) {
        clearBreakpoint(filePath, lineNumber);
    } else {
        setBreakpoint(filePath, lineNumber);
    }
}

// 继续执行
void DebuggerManager::continueExecution()
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb("-exec-continue");
    }
}

// 步入
void DebuggerManager::stepInto()
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb("-exec-step");
    }
}

// 步过
void DebuggerManager::stepOver()
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb("-exec-next");
    }
}

// 步出
void DebuggerManager::stepOut()
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb("-exec-finish");
    }
}

// 检查变量
void DebuggerManager::inspectVariable(const QString &variableName)
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        sendCommandToGdb(QString("-data-evaluate-expression \"%1\"").arg(variableName));
    }
}

// 设置输出窗口
void DebuggerManager::setOutputWidget(QTextEdit *output)
{
    m_outputWidget = output;
}

// 向 GDB 发送命令
void DebuggerManager::sendCommandToGdb(const QString &command)
{
    if (m_gdbProcess && m_gdbProcess->state() != QProcess::NotRunning) {
        QString fullCommand = command + "\n";
        m_gdbProcess->write(fullCommand.toUtf8());
        qDebug() << "Sent to GDB:" << fullCommand.trimmed();
    }
}

// 处理 GDB 标准输出
void DebuggerManager::onGdbReadyReadStandardOutput()
{
    QByteArray data = m_gdbProcess->readAllStandardOutput();
    QString output = QString::fromUtf8(data);
    qDebug() << "GDB STDOUT:" << output;
    appendToOutput(output, QColor("#a9b7c6"));

    // 解析 GDB MI 输出并检测断点命中
    QRegularExpression stopRegex("^\\*stopped,reason=\"(.*?)\",.*?,file=\"(.*?)\",line=\"(\\d+)\"");
    QRegularExpressionMatch match = stopRegex.match(output);

    if (match.hasMatch()) {
        QString reason = match.captured(1);
        QString filePath = match.captured(2);
        int lineNumber = match.captured(3).toInt();

        if (reason == "breakpoint-hit") {
            emit hitBreakpoint(filePath, lineNumber);
            appendToOutput(QString("命中断点：%1:%2\n").arg(filePath).arg(lineNumber), Qt::yellow);
        }
    }
}

// 处理 GDB 标准错误输出
void DebuggerManager::onGdbReadyReadStandardError()
{
    QByteArray data = m_gdbProcess->readAllStandardError();
    QString error = QString::fromUtf8(data);
    qDebug() << "GDB STDERR:" << error;
    appendToOutput(error, Qt::red);
}

// 处理 GDB 进程结束
void DebuggerManager::onGdbFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    qDebug() << "GDB 进程结束。";
    stopDebugging(); // 确保清理
}

// 追加文本到输出窗口
void DebuggerManager::appendToOutput(const QString &text, const QColor &color)
{
    if (m_outputWidget) {
        QTextCharFormat format;
        format.setForeground(color);
        QTextCursor cursor = m_outputWidget->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(text, format);
        m_outputWidget->setTextCursor(cursor);
        m_outputWidget->ensureCursorVisible(); // 确保输出区域始终可见
    }
}
