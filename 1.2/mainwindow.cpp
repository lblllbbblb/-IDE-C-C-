#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QTextCursor> // For QTextCursor

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , compilerProcess(new QProcess(this))
    , cppHighlighter(nullptr) // 初始化为 nullptr
{
    ui->setupUi(this);
    setupUI();
    createMenus();

    // 在 codeEditor 创建后，初始化语法高亮器
    cppHighlighter = new CppHighlighter(codeEditor->document()); // 将高亮器关联到 codeEditor 的文档

    // 连接进程信号与槽
    connect(compilerProcess, &QProcess::finished, this, &MainWindow::onProcessFinished);
    connect(compilerProcess, &QProcess::errorOccurred, this, &MainWindow::onProcessErrorOccurred);
    // 新增：连接标准输出和错误输出的信号
    connect(compilerProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onReadyReadStandardOutput);
    connect(compilerProcess, &QProcess::readyReadStandardError, this, &MainWindow::onReadyReadStandardError);

    // 连接 outputConsole 的 textChanged 信号来处理用户输入
    connect(outputConsole, &QTextEdit::textChanged, this, &MainWindow::onInputReady);

    // 初始设置 inputStartPos
    inputStartPos = outputConsole->document()->toPlainText().length();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("C/C++ Compiler IDE");
    setMinimumSize(800, 600);

    // 创建中央部件和布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 代码编辑区
    codeEditor = new QTextEdit();
    codeEditor->setPlaceholderText("请输入C/C++代码...");
    codeEditor->setFontFamily("Courier New");
    codeEditor->setFontPointSize(10);

    // 输出控制台
    outputConsole = new QTextEdit();
    outputConsole->setReadOnly(false); // 允许用户在 outputConsole 中输入
    outputConsole->setFontFamily("Courier New");
    outputConsole->setFontPointSize(10);
    outputConsole->setStyleSheet("background-color: #f0f0f0;");

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *compileBtn = new QPushButton("编译");
    QPushButton *runBtn = new QPushButton("运行");
    QPushButton *saveBtn = new QPushButton("保存");
    QPushButton *openBtn = new QPushButton("打开");

    connect(compileBtn, &QPushButton::clicked, this, &MainWindow::onCompileClicked);
    connect(runBtn, &QPushButton::clicked, this, &MainWindow::onRunClicked);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(openBtn, &QPushButton::clicked, this, &MainWindow::onOpenClicked);

    buttonLayout->addWidget(openBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(compileBtn);
    buttonLayout->addWidget(runBtn);

    // 添加到主布局
    mainLayout->addLayout(buttonLayout);
    // 直接将代码编辑器和输出控制台添加到主布局中
    mainLayout->addWidget(codeEditor);
    mainLayout->addWidget(outputConsole);

    // 可以选择性地设置拉伸因子，让代码编辑器占据更多空间
    mainLayout->setStretchFactor(codeEditor, 3); // 代码编辑器占据3份空间
    mainLayout->setStretchFactor(outputConsole, 1); // 输出控制台占据1份空间


    setCentralWidget(centralWidget);
}

void MainWindow::createMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件");

    QAction *newAction = new QAction("新建", this);
    QAction *openAction = new QAction("打开", this);
    QAction *saveAction = new QAction("保存", this);
    QAction *exitAction = new QAction("退出", this);

    connect(newAction, &QAction::triggered, [this]() {
        codeEditor->clear();
        currentFilePath = "";
    });
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenClicked);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveClicked);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    // 编译菜单
    QMenu *buildMenu = menuBar()->addMenu("编译");

    QAction *compileAction = new QAction("编译", this);
    QAction *runAction = new QAction("运行", this);

    connect(compileAction, &QAction::triggered, this, &MainWindow::onCompileClicked);
    connect(runAction, &QAction::triggered, this, &MainWindow::onRunClicked);

    buildMenu->addAction(compileAction);
    buildMenu->addAction(runAction);
}

void MainWindow::onCompileClicked()
{
    if (currentFilePath.isEmpty()) {
        onSaveClicked(); // 如果未保存，先保存文件
        if (currentFilePath.isEmpty()) return; // 如果取消保存，则返回
    }

    outputConsole->append("[" + QDateTime::currentDateTime().toString() + "] 开始编译...");
    outputConsole->moveCursor(QTextCursor::End); // 确保光标在最后

    // 清理之前的输出（可选，但通常有助于编译日志清晰）
    // outputConsole->clear(); // 如果希望每次编译都清空控制台，可以取消注释

    // 使用g++编译
    QString compiler = "g++";
    QStringList arguments;
    arguments << currentFilePath << "-o" << currentFilePath + ".out";

    // 启动编译进程 (这里不需要设置Input/Output，因为编译是非交互式的)
    compilerProcess->start(compiler, arguments);
}

void MainWindow::onRunClicked()
{
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先保存文件");
        return;
    }

    // 确保可执行文件存在
    QString program = currentFilePath + ".out";
    if (!QFile::exists(program)) {
        QMessageBox::warning(this, "警告", "可执行文件不存在。请先编译。");
        return;
    }

    outputConsole->append("[" + QDateTime::currentDateTime().toString() + "] 开始运行...");
    outputConsole->moveCursor(QTextCursor::End); // 移动光标到末尾

    // 清理之前的进程（如果还在运行）
    if (compilerProcess->state() != QProcess::NotRunning) {
        compilerProcess->kill();
        compilerProcess->waitForFinished();
    }

    compilerProcess->setProgram(program);
    compilerProcess->setArguments(QStringList()); // 暂时没有命令行参数

    // !!! 关键改动：设置进程通道模式为分开，并以读写模式打开 !!!
    compilerProcess->setProcessChannelMode(QProcess::SeparateChannels);
    compilerProcess->open(QIODevice::ReadWrite);

    // 记录当前 outputConsole 文本的长度，作为用户输入开始的标记
    inputStartPos = outputConsole->document()->toPlainText().length();

    // 如果进程未能成功启动
    if (compilerProcess->state() == QProcess::NotRunning) {
        outputConsole->append("错误: 无法启动程序 " + program + "\n" + compilerProcess->errorString());
        inputStartPos = outputConsole->document()->toPlainText().length(); // 重置
    }
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // 如果是编译进程，确保清空输出
    if (compilerProcess->program() == "g++") {
        outputConsole->append(compilerProcess->readAllStandardOutput());
        outputConsole->append(compilerProcess->readAllStandardError());
    }

    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == 0) {
            outputConsole->append("操作成功完成。");
        } else {
            outputConsole->append("操作失败，退出代码: " + QString::number(exitCode));
        }
    } else {
        outputConsole->append("进程异常终止。");
    }
    outputConsole->moveCursor(QTextCursor::End); // 确保光标在最后
    inputStartPos = outputConsole->document()->toPlainText().length(); // 重置输入起始位置
}

void MainWindow::onProcessErrorOccurred(QProcess::ProcessError error)
{
    outputConsole->append("进程错误: " + compilerProcess->errorString());
    outputConsole->moveCursor(QTextCursor::End); // 确保光标在最后
    inputStartPos = outputConsole->document()->toPlainText().length(); // 重置输入起始位置
}

// 新增：处理标准输出
void MainWindow::onReadyReadStandardOutput()
{
    outputConsole->insertPlainText(compilerProcess->readAllStandardOutput());
    outputConsole->moveCursor(QTextCursor::End); // 保持光标在文本末尾
    inputStartPos = outputConsole->document()->toPlainText().length(); // 更新输入起始位置
}

// 新增：处理标准错误
void MainWindow::onReadyReadStandardError()
{
    outputConsole->insertPlainText(compilerProcess->readAllStandardError());
    outputConsole->moveCursor(QTextCursor::End); // 保持光标在文本末尾
    inputStartPos = outputConsole->document()->toPlainText().length(); // 更新输入起始位置
}

// 新增：处理用户在 outputConsole 中的输入
void MainWindow::onInputReady()
{
    // Only process input if the program is running
    if (compilerProcess->state() == QProcess::Running) {
        QString currentText = outputConsole->document()->toPlainText();

        // Check if there's new content compared to the last update
        if (currentText.length() > inputStartPos) {
            QString newText = currentText.mid(inputStartPos);

            // Check for a newline character, indicating the user has finished a line of input
            if (newText.contains('\n')) {
                int newlineIndex = newText.lastIndexOf('\n');
                QString lineToSend = newText.left(newlineIndex + 1); // Include the newline character

                // Write the user input to the child process's standard input
                compilerProcess->write(lineToSend.toUtf8());
                // After successful write, update inputStartPos
                inputStartPos = outputConsole->document()->toPlainText().length();
            }
        }
    } else {
        // If the program is not running, and the user typed in the outputConsole
        // We do nothing special here; the input won't be sent to any process.
    }
}

void MainWindow::onSaveClicked()
{
    if (currentFilePath.isEmpty()) {
        currentFilePath = QFileDialog::getSaveFileName(this, "保存文件", "", "C/C++ Files (*.c *.cpp)");
        if (currentFilePath.isEmpty()) return;
    }

    QFile file(currentFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << codeEditor->toPlainText();
        file.close();
        outputConsole->append("文件已保存: " + currentFilePath);
        outputConsole->moveCursor(QTextCursor::End);
    } else {
        QMessageBox::warning(this, "错误", "无法保存文件: " + file.errorString());
    }
}

void MainWindow::onOpenClicked()
{
    currentFilePath = QFileDialog::getOpenFileName(this, "打开文件", "", "C/C++ Files (*.c *.cpp)");
    if (currentFilePath.isEmpty()) return;

    QFile file(currentFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        codeEditor->setText(in.readAll());
        file.close();
        outputConsole->append("文件已打开: " + currentFilePath);
        outputConsole->moveCursor(QTextCursor::End);
    } else {
        QMessageBox::warning(this, "错误", "无法打开文件: " + file.errorString());
    }
}
