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
#include <QInputDialog> // 确保包含 QInputDialog 头文件

#include "cpphighlighter.h"
#include "findreplacedialog.h"
#include "bracketmatcher.h"

#include <cstdio> // 引入 C 标准库，用于文件操作

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

    void on_findReplace();
    void on_fontsize();

    // --- 新增：撤销和恢复的槽函数 ---
    void on_undo();
    void on_redo();

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

    QAction *edit_copy;
    QAction *edit_paste;
    QAction *edit_cut;
    QAction *edit_selectAll;
    QAction *edit_findReplace;
    // --- 新增：撤销和恢复动作 ---
    QAction *edit_undo;
    QAction *edit_redo;

    QAction *build_compile;
    QAction *build_run;
    QAction *build_compileAndRun;

    QAction *settings_fontsize;

    QAction *help_about;
    int fontsize;

    CppHighlighter *cppHighlighter;
    FindReplaceDialog *m_findReplaceDialog;
    BracketMatcher *m_bracketMatcher;
};

#endif // MAINWINDOW_H
