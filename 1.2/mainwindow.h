#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QSettings>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>

#include "cpphighlighter.h" // 包含头文件

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
    void onSaveClicked();
    void onOpenClicked();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);
    void onReadyReadStandardOutput(); // New slot to read stdout
    void onReadyReadStandardError();  // New slot to read stderr
    void onInputReady();              // New slot to handle user input

private:
    void setupUI();
    void createMenus();

    Ui::MainWindow *ui;
    QTextEdit *codeEditor;
    QTextEdit *outputConsole; // This will now serve as output and *input*
    QProcess *compilerProcess;
    QString currentFilePath;
    CppHighlighter *cppHighlighter; // 声明高亮器指针

    // To track where the user input starts in the console
    qint64 inputStartPos = 0;
};
#endif // MAINWINDOW_H
