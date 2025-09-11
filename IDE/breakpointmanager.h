#ifndef BREAKPOINTMANAGER_H
#define BREAKPOINTMANAGER_H

#include <QString>
#include <QMap>
#include <QList>

class BreakpointManager
{
public:
    BreakpointManager();
    ~BreakpointManager();

    // 添加断点
    void addBreakpoint(const QString &filePath, int lineNumber);
    // 删除断点
    void removeBreakpoint(const QString &filePath, int lineNumber);
    // 切换断点（如果存在则移除，如果不存在则添加）
    void toggleBreakpoint(const QString &filePath, int lineNumber);
    // 获取文件的所有断点
    QList<int> getBreakpoints(const QString &filePath) const;
    // 清除所有断点
    void clearAllBreakpoints();

private:
    QMap<QString, QList<int>> m_breakpoints; // 存储断点：文件路径 -> 行号列表
};

#endif // BREAKPOINTMANAGER_H
