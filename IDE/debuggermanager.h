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
    explicit DebuggerManager(QObject *parent = nullptr);
    ~DebuggerManager();

    void startDebugging(const QString &executablePath);
    void stopDebugging();
    void setBreakpoint(const QString &filePath, int lineNumber);
    void clearBreakpoint(const QString &filePath, int lineNumber);
    void toggleBreakpoint(const QString &filePath, int lineNumber);
    void continueExecution();
    void stepInto();
    void stepOver();
    void stepOut();
    void inspectVariable(const QString &variableName);
    void setOutputWidget(QTextEdit *output);

    QString getGdbPath() const; // ←←← 修正此声明

    // 断点相关成员
    QMap<QString, QList<int>> m_breakpoints;
    QMap<QString, QList<int>> m_gdbBreakpointIds;

signals:
    void debuggerStarted();
    void debuggerStopped();
    void hitBreakpoint(const QString &filePath, int lineNumber);
    void currentLineChanged(const QString &filePath, int lineNumber);
    void outputReceived(const QString &text);

private slots:
    void onGdbReadyReadStandardOutput();
    void onGdbReadyReadStandardError();
    void onGdbFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void sendCommandToGdb(const QString &command);
    void appendToOutput(const QString &text, const QColor &color = QColor("#a9b7c6"));

    QProcess *m_gdbProcess;
    QTextEdit *m_outputWidget;
};

#endif // DEBUGGERMANAGER_H
