// MY_IDE_H
#ifndef MY_IDE_H
#define MY_IDE_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QWidget>
#include <QDir>
#include <QInputDialog>
#include <QTabWidget>
#include <QTextEdit>
#include "TabWidgetManager.h"
#include "DebuggerManager.h"
#include <QWheelEvent>
#include "Formatter.h"
#include "DeepSeekChat.h"
#include "CodeEditor.h"
#include "ColorRefresher.h"

class My_IDE : public QMainWindow
{
    Q_OBJECT
private:
    QTabWidget *m_tabWidget; // 标签页组件
    TabWidgetManager *m_tabManager; // 标签页管理器
    DebuggerManager *m_debuggerManager; // 调试器管理器
    Formatter *m_formatter; // 代码格式化器

    QMenu *file; // 文件菜单
    QMenu *edit; // 编辑菜单
    QMenu *build; // 构建菜单
    QMenu *debug; // 调试菜单
    QMenu *help; // 帮助菜单
    QMenu *settings; // 设置菜单

    QMenu *aiAssistant; // AI 助手菜单
    QAction *aiAssistant_deepSeekChat; // DeepSeek Chat 动作

    QAction *file_new; // 新建文件动作
    QAction *file_open; // 打开文件动作
    QAction *file_save; // 保存文件动作
    QAction *file_othersave; // 另存为动作
    QAction *file_exit; // 退出动作
    QAction *file_closeTab; // 关闭标签页动作

    QAction *build_compile; // 编译动作
    QAction *build_run; // 运行动作
    QAction *build_compileAndRun; // 编译并运行动作

    QAction *debug_start; // 开始调试动作
    QAction *debug_stop; // 停止调试动作
    QAction *debug_continue; // 继续调试动作
    QAction *debug_stepInto; // 步入动作
    QAction *debug_stepOver; // 步过动作
    QAction *debug_stepOut; // 步出动作
    QAction *debug_toggleBreakpoint; // 切换断点动作

    QAction *help_about; // 关于动作

    QAction *edit_copy; // 复制动作
    QAction *edit_paste; // 粘贴动作
    QAction *edit_cut; // 剪切动作
    QAction *edit_selectAll; // 全选动作
    QAction *edit_findReplace; // 查找替换动作
    QAction *edit_formatCode; // 格式化代码动作

    QAction *settings_fontsize; // 字体大小设置动作

    QAction *edit_undo; // 撤销动作
    QAction *edit_redo; // 重做动作
    CodeEditor *editor; // 当前活跃的代码编辑器
    // 为了避免混淆，我们使用 m_tabManager->currentEditor() 来获取当前编辑器

    int fontsize; // 字体大小

    bool m_isDarkMode; // 是否为深色模式
    QAction *settings_toggleColor; // 切换颜色模式动作

    QTextEdit *m_debugOutputWidget; // 调试输出窗口

    void initMenuSystem(); // 初始化菜单系统
    void connectActions(); // 连接动作
    void setupDebuggerUI(); // 设置调试器界面

protected:
    void wheelEvent(QWheelEvent *event) override; // 滚轮事件

public:
    My_IDE(QMainWindow *parent = nullptr); // 构造函数
    ~My_IDE(); // 析构函数

private slots:
    void on_new(); // 新建文件槽
    void on_open(); // 打开文件槽
    void on_save(); // 保存文件槽
    void on_othersave(); // 另存为槽
    void on_about(); // 关于槽
    void on_exit(); // 退出槽
    void on_copy(); // 复制槽
    void on_paste(); // 粘贴槽
    void on_cut(); // 剪切槽
    void on_selectAll(); // 全选槽
    void on_compile(); // 编译槽
    void on_run(); // 运行槽
    void on_compileAndRun(); // 编译并运行槽
    void on_findReplace(); // 查找替换槽
    void on_fontsize(); // 字体大小槽
    void on_undo(); // 撤销槽
    void on_redo(); // 重做槽
    void on_closeTab(); // 关闭标签页槽
    void on_formatCode(); // 格式化代码槽

    void on_currentEditorChanged(CodeEditor *editor); // 当前编辑器改变槽
    int calculateVisualColumn(CodeEditor *editor); // 计算可视列

    void on_toggleColorMode(); // 切换颜色模式槽
    void applyColorMode(bool darkMode); // 应用颜色模式

    // 调试槽
    void on_debugStart(); // 开始调试槽
    void on_debugStop(); // 停止调试槽
    void on_debugContinue(); // 继续调试槽
    void on_debugStepInto(); // 步入槽
    void on_debugStepOver(); // 步过槽
    void on_debugStepOut(); // 步出槽
    void on_debugToggleBreakpoint(); // 切换断点槽

    // DebuggerManager 信号处理槽
    void on_debuggerStarted(); // 调试器启动槽
    void on_debuggerStopped(); // 调试器停止槽
    void on_hitBreakpoint(const QString &filePath, int lineNumber); // 命中断点槽
    void on_currentDebugLineChanged(const QString &filePath, int lineNumber); // 当前调试行改变槽

    // 调试行高亮
    void highlightDebugLine(const QString &filePath, int lineNumber); // 高亮调试行
    void clearDebugHighlights(); // 清除调试高亮

    // DeepSeekChat 槽
    void on_deepSeekChat();

    // 用于接收 DeepSeekChatDialog 传回的代码并插入到当前编辑器
    void insertCodeToCurrentEditor(const QString &code, int startLine, int endLine); // 插入代码到当前编辑器槽
};
#endif // MY_IDE_H
