#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>       // For checking file existence
#include <QFileInfo>   // For getting file information (like base name)
#include <QDir>        // For getting current directory

#include "cpphighlighter.h" // 包含头文件

// 引入 C 标准库，用于文件操作（fopen, fclose, fputs, fgets, feof）
#include <cstdio>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT // 必不可少的宏，用于启用Qt的元对象系统（信号与槽机制）

public:
    // 构造函数，初始化主窗口
    MainWindow(QWidget *parent = nullptr);
    // 析构函数，释放资源
    ~MainWindow();

private slots:
    // 打开文件的槽函数
    void on_open();
    // 保存文件的槽函数
    void on_save();
    // 另存文件的槽函数 (你的新代码中是 on_othersave, 这里保持 on_anothersave)
    void on_anothersave(); // 对应你的 on_othersave
    // 复制文本的槽函数
    void on_copy();
    // 粘贴文本的槽函数
    void on_paste();
    // 剪切文本的槽函数
    void on_cut();
    // 全选文本的槽函数
    void on_selectAll();
    // 显示“关于”信息的槽函数
    void on_about();

    // *** 替换后的编译和运行槽函数 ***
    void on_compile();
    void on_run();
    void on_compileAndRun(); // 新增的槽函数

private:
    Ui::MainWindow *ui;         // Qt设计师生成的界面对象指针
    QString filename;           // 当前打开或保存的文件名
    QTextEdit *text1;           // 文本编辑组件（核心编辑区域）

    QMenu *file;                // “文件”菜单
    QMenu *edit;                // “编辑”菜单
    QMenu *build;               // “构建”菜单
    QMenu *help;                // “帮助”菜单

    QAction *file_open;         // “文件”->“打开”动作
    QAction *file_save;         // “文件”->“保存”动作
    QAction *file_anothersave;         // “文件”->“另存”动作
    QAction *file_exit;         // “文件”->“退出”动作

    QAction *build_compile;     // “构建”->“编译”动作
    QAction *build_run;         // “构建”->“运行”动作
    QAction *build_compileAndRun; // 新增的“构建”->“编译并运行”动作

    CppHighlighter *cppHighlighter; // 声明高亮器指针
};

#endif // MAINWINDOW_H
