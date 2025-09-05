#ifndef MY_IDE_H
#define MY_IDE_H

#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QWidget>
#include <QDir>
#include<QInputDialog>
#include "cpphighlighter.h"
#include "CodeEditor.h"
#include "bracketmatcher.h"   // 新增：包含括号匹配器的头文件
#include "findreplacedialog.h" // 包含查找替换对话框的头文件

class My_IDE : public QMainWindow
{
    Q_OBJECT
private:
    QString filename;//定义文件名

    QMenu *file;
    QMenu *edit;
    QMenu *build;
    QMenu *help;
    QMenu *settings;

    QAction *file_open;//下拉;QAction 是一个核心类，用于封装用户界面的 “动作”（如菜单选项、工具栏按钮、快捷键等)
    QAction *file_save;
    QAction *file_othersave;
    QAction *file_exit;

    QAction *build_compile;
    QAction *build_run;
    QAction *build_compileAndRun;

    QAction *help_about;

    QAction *edit_copy;
    QAction *edit_paste;
    QAction *edit_cut;
    QAction *edit_selectAll;
    QAction *edit_findReplace; // 新增的查找替换动作

    QAction *settings_fontsize;
    //撤销恢复
    QAction *edit_undo;
    QAction *edit_redo;

    CppHighlighter *cppHighlighter; // 声明高亮器指针

    CodeEditor* text1;

    BracketMatcher *m_bracketMatcher;     // 新增：声明括号匹配器指针



    void initMenuSystem();
    void connectActions();

    FindReplaceDialog *m_findReplaceDialog; // 声明查找替换对话框指针

    //字体大小设置
    int fontsize ;


public:
    My_IDE(QMainWindow *parent = nullptr);
    ~My_IDE();

private slots:
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
    void on_fontsize();    // 新增的槽函数，用于显示查找替换对话框
    void on_undo();
    void on_redo();

};
#endif // MY_IDE_H
