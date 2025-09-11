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
#include <QTextEdit> // 新增：用于调试输出
#include "TabWidgetManager.h"
#include "DebuggerManager.h" // 新增：包含 DebuggerManager 头文件
#include <QWheelEvent>
#include "Formatter.h"

class My_IDE : public QMainWindow
{
    Q_OBJECT
private:
    QTabWidget *m_tabWidget;
    TabWidgetManager *m_tabManager;
    DebuggerManager *m_debuggerManager; // 新增：DebuggerManager 实例
    Formatter *m_formatter;

    QMenu *file;
    QMenu *edit;
    QMenu *build;
    QMenu *debug; // 新增：调试菜单
    QMenu *help;
    QMenu *settings;

    QAction *file_new;
    QAction *file_open;
    QAction *file_save;
    QAction *file_othersave;
    QAction *file_exit;
    QAction *file_closeTab;

    QAction *build_compile;
    QAction *build_run;
    QAction *build_compileAndRun;

    // 新增：调试动作
    QAction *debug_start;
    QAction *debug_stop;
    QAction *debug_continue;
    QAction *debug_stepInto;
    QAction *debug_stepOver;
    QAction *debug_stepOut;
    QAction *debug_toggleBreakpoint; // 切换断点

    QAction *help_about;

    QAction *edit_copy;
    QAction *edit_paste;
    QAction *edit_cut;
    QAction *edit_selectAll;
    QAction *edit_findReplace;
    QAction *edit_formatCode;

    QAction *settings_fontsize;

    QAction *edit_undo;
    QAction *edit_redo;
    CodeEditor *editor; // 这里可能需要移除，因为 currentEditor() 才是当前的

    int fontsize;

    bool m_isDarkMode;
    QAction *settings_toggleColor;

    QTextEdit *m_debugOutputWidget; // 新增：用于显示调试器输出的文本框

    void initMenuSystem();
    void connectActions();
    void setupDebuggerUI(); // 新增：设置调试器UI

protected:
    void wheelEvent(QWheelEvent *event) override;

public:
    My_IDE(QMainWindow *parent = nullptr);
    ~My_IDE();

private slots:
    void on_new();
    void on_open();
    void on_save();
    void on_othersave();
    void on_about();
    void on_exit();
    void on_copy();
    void on_paste();
    void on_cut();
    void on_selectAll();
    void on_compile();
    void on_run();
    void on_compileAndRun();
    void on_findReplace();
    void on_fontsize();
    void on_undo();
    void on_redo();
    void on_closeTab();
    void on_formatCode();

    void on_currentEditorChanged(CodeEditor *editor);
    int calculateVisualColumn(CodeEditor *editor);

    void on_toggleColorMode();
    void applyColorMode(bool darkMode);

    // 新增：调试槽函数
    void on_debugStart();
    void on_debugStop();
    void on_debugContinue();
    void on_debugStepInto();
    void on_debugStepOver();
    void on_debugStepOut();
    void on_debugToggleBreakpoint();

    // 调试器信号处理槽
    void on_debuggerStarted();
    void on_debuggerStopped();
    void on_hitBreakpoint(const QString &filePath, int lineNumber);
    void on_currentDebugLineChanged(const QString &filePath, int lineNumber);

    // 标记当前调试行
    void highlightDebugLine(const QString &filePath, int lineNumber);
    // 清除所有调试标记
    void clearDebugHighlights();
};
#endif // MY_IDE_H
