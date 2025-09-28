#include "BreakpointManager.h"

BreakpointManager::BreakpointManager()
{
}

BreakpointManager::~BreakpointManager()
{
}

// 添加断点
void BreakpointManager::addBreakpoint(const QString &filePath, int lineNumber)
{
    // 如果文件路径不存在，则创建一个新的QList
    if (!m_breakpoints.contains(filePath)) {
        m_breakpoints[filePath] = QList<int>();
    }
    // 如果行号不存在，则添加断点
    if (!m_breakpoints[filePath].contains(lineNumber)) {
        m_breakpoints[filePath].append(lineNumber);
    }
}

// 移除断点
void BreakpointManager::removeBreakpoint(const QString &filePath, int lineNumber)
{
    // 如果文件路径存在，则移除指定行号的断点
    if (m_breakpoints.contains(filePath)) {
        m_breakpoints[filePath].removeOne(lineNumber);
    }
}

// 切换断点（如果存在则移除，否则添加）
void BreakpointManager::toggleBreakpoint(const QString &filePath, int lineNumber)
{
    // 如果文件路径存在且包含该行号的断点，则移除
    if (m_breakpoints.contains(filePath) && m_breakpoints[filePath].contains(lineNumber)) {
        removeBreakpoint(filePath, lineNumber);
    } else { // 否则添加断点
        addBreakpoint(filePath, lineNumber);
    }
}

// 获取指定文件路径的所有断点
QList<int> BreakpointManager::getBreakpoints(const QString &filePath) const
{
    return m_breakpoints.value(filePath);
}

// 清除所有断点
void BreakpointManager::clearAllBreakpoints()
{
    m_breakpoints.clear();
}
