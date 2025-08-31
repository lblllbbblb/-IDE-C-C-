#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QProcess>
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

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
    void onCompileClicked();
    void onRunClicked();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onSaveClicked();
    void onOpenClicked();

private:
    Ui::MainWindow *ui;
    QTextEdit *codeEditor;       // 代码编辑区
    QTextEdit *outputConsole;    // 输出控制台
    QProcess *compilerProcess;   // 编译进程
    QString currentFilePath;     // 当前文件路径

    void setupUI();              // 设置界面
    void createMenus();          // 创建菜单
};
#endif // MAINWINDOW_H
