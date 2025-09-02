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
#include <QDialog>     // New: For the find dialog
#include <QLineEdit>   // New: For the find input
#include <QLabel>      // New: For labels in the find dialog
#include <QCheckBox>   // New: For case sensitivity option

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
    void onFindClicked();             // New: Slot to show find dialog
    void findNext();                  // New: Slot to find next occurrence
    void findPrevious();              // New: Slot to find previous occurrence

private:
    void setupUI();
    void createMenus();

    Ui::MainWindow *ui;
    QTextEdit *codeEditor;
    QTextEdit *outputConsole; // This will now serve as output and *input*
    QProcess *compilerProcess;
    QString currentFilePath;
    CppHighlighter *cppHighlighter; // 声明高亮器指针

    // New: Find functionality members
    QDialog *findDialog = nullptr;    // 查找对话框
    QLineEdit *findLineEdit = nullptr;  // 查找输入框
    QCheckBox *caseSensitiveCheckBox = nullptr; // 大小写敏感选项

    // To track where the user input starts in the console
    qint64 inputStartPos = 0;
};
#endif // MAINWINDOW_H
