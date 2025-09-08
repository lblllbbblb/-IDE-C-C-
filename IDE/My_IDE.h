#ifndef MY_IDE_H
#define MY_IDE_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QWidget>
#include <QDir>
#include <QInputDialog>
#include <QTabWidget> // 新增：包含 QTabWidget 头文件
// #include "cpphighlighter.h" // 这些现在由 TabWidgetManager 管理
// #include "CodeEditor.h"
// #include "bracketmatcher.h"
// #include "findreplacedialog.h"
#include "TabWidgetManager.h" // 新增：包含 TabWidgetManager 头文件
#include <QWheelEvent> // 新增：包含 QWheelEvent 头文件

class My_IDE : public QMainWindow
{
    Q_OBJECT
private:
    // QString filename; // 不再由 My_IDE 直接管理，而是由 TabWidgetManager 管理
    QTabWidget *m_tabWidget; // 新增：Tab 控件
    TabWidgetManager *m_tabManager; // 新增：Tab 管理器

    QMenu *file;
    QMenu *edit;
    QMenu *build;
    QMenu *help;
    QMenu *settings;

    QAction *file_new; // 新增：新建文件动作
    QAction *file_open;
    QAction *file_save;
    QAction *file_othersave;
    QAction *file_exit;
    QAction *file_closeTab; // 新增：关闭当前 Tab 动作

    QAction *build_compile;
    QAction *build_run;
    QAction *build_compileAndRun;

    QAction *help_about;

    QAction *edit_copy;
    QAction *edit_paste;
    QAction *edit_cut;
    QAction *edit_selectAll;
    QAction *edit_findReplace;

    QAction *settings_fontsize;

    QAction *edit_undo;
    QAction *edit_redo;
    CodeEditor *editor;

    // CppHighlighter *cppHighlighter; // 由 TabWidgetManager 管理
    // CodeEditor* text1; // 不再直接使用 CodeEditor
    // BracketMatcher *m_bracketMatcher; // 由 TabWidgetManager 管理
    // FindReplaceDialog *m_findReplaceDialog; // 由 TabWidgetManager 管理

    int fontsize; // 字体大小仍然在 My_IDE 中维护

    void initMenuSystem();
    void connectActions();

protected:
    void wheelEvent(QWheelEvent *event) override; // 重写滚轮事件

public:
    My_IDE(QMainWindow *parent = nullptr);
    ~My_IDE();

private slots:
    void on_new(); // 新增槽函数
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
    void on_closeTab(); // 新增槽函数

    void on_currentEditorChanged(CodeEditor *editor); // 接收 TabWidgetManager 的信号
    int calculateVisualColumn(CodeEditor *editor);
};
#endif // MY_IDE_H
