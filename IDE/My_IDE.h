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
#include "TabWidgetManager.h"
#include <QWheelEvent>
#include "Formatter.h" // 新增：包含 Formatter 头文件

class My_IDE : public QMainWindow
{
    Q_OBJECT
private:
    QTabWidget *m_tabWidget;
    TabWidgetManager *m_tabManager;
    Formatter *m_formatter; // 新增：Formatter 类的实例

    QMenu *file;
    QMenu *edit;
    QMenu *build;
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

    QAction *help_about;

    QAction *edit_copy;
    QAction *edit_paste;
    QAction *edit_cut;
    QAction *edit_selectAll;
    QAction *edit_findReplace;
    QAction *edit_formatCode; // 新增：格式化代码动作

    QAction *settings_fontsize;

    QAction *edit_undo;
    QAction *edit_redo;
    CodeEditor *editor;

    int fontsize;

    bool m_isDarkMode;
    QAction *settings_toggleColor;

    void initMenuSystem();
    void connectActions();

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
    void on_formatCode(); // 新增：格式化代码槽函数

    void on_currentEditorChanged(CodeEditor *editor);
    int calculateVisualColumn(CodeEditor *editor);

    void on_toggleColorMode();
    void applyColorMode(bool darkMode);
};
#endif // MY_IDE_H
