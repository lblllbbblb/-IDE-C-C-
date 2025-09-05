#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include<QInputDialog>

#include "cpphighlighter.h"
#include "findreplacedialog.h" // 包含查找替换对话框的头文件
#include "bracketmatcher.h"   // 新增：包含括号匹配器的头文件

// 引入 C 标准库，用于文件操作（fopen, fclose, fputs, fgets, feof）
#include <cstdio>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_open();
    void on_save();
    void on_anothersave();
    void on_copy();
    void on_paste();
    void on_cut();
    void on_selectAll();
    void on_about();

    void on_compile();
    void on_run();
    void on_compileAndRun();

    void on_findReplace(); // 新增的槽函数，用于显示查找替换对话框
    void on_fontsize();//设置字体大小

private:
    Ui::MainWindow *ui;
    QString filename;
    QTextEdit *text1;

    QMenu *file;
    QMenu *edit;
    QMenu *build;
    QMenu *help;
    QMenu *settings;

    QAction *file_open;
    QAction *file_save;
    QAction *file_anothersave;
    QAction *file_exit;

    QAction *edit_copy;     // 复制动作
    QAction *edit_paste;    // 粘贴动作
    QAction *edit_cut;      // 剪切动作
    QAction *edit_selectAll;// 全选动作
    QAction *edit_findReplace; // 新增的查找替换动作

    QAction *build_compile;
    QAction *build_run;
    QAction *build_compileAndRun;

    QAction *settings_fontsize;//设置-设置字体大小

    QAction *help_about;    // 关于动作
    int fontsize ;

    CppHighlighter *cppHighlighter;
    FindReplaceDialog *m_findReplaceDialog; // 声明查找替换对话框指针
    BracketMatcher *m_bracketMatcher;     // 新增：声明括号匹配器指针
};

#endif // MAINWINDOW_H
