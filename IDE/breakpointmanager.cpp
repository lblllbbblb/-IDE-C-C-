#include "BreakpointManager.h"

BreakpointManager::BreakpointManager()
{
}

BreakpointManager::~BreakpointManager()
{
}

void BreakpointManager::addBreakpoint(const QString &filePath, int lineNumber)
{
    if (!m_breakpoints.contains(filePath)) {
        m_breakpoints[filePath] = QList<int>();
    }
    if (!m_breakpoints[filePath].contains(lineNumber)) {
        m_breakpoints[filePath].append(lineNumber);
    }
}

void BreakpointManager::removeBreakpoint(const QString &filePath, int lineNumber)
{
    if (m_breakpoints.contains(filePath)) {
        m_breakpoints[filePath].removeOne(lineNumber);
    }
}

void BreakpointManager::toggleBreakpoint(const QString &filePath, int lineNumber)
{
    if (m_breakpoints.contains(filePath) && m_breakpoints[filePath].contains(lineNumber)) {
        removeBreakpoint(filePath, lineNumber);
    } else {
        addBreakpoint(filePath, lineNumber);
    }
}

QList<int> BreakpointManager::getBreakpoints(const QString &filePath) const
{
    return m_breakpoints.value(filePath);
}

void BreakpointManager::clearAllBreakpoints()
{
    m_breakpoints.clear();
}
