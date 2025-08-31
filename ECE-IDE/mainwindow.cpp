#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>

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
    outputConsole->setReadOnly(true);
    outputConsole->setFontFamily("Courier New");
    outputConsole->setFontPointSize(10);
    outputConsole->setStyleSheet("background-color: #f0f0f0;");

    // 创建标签页
    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->addTab(codeEditor, "代码编辑");
    tabWidget->addTab(outputConsole, "输出");

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
    mainLayout->addWidget(tabWidget);

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

    // 使用g++编译
    QString compiler = "g++";
    QStringList arguments;
    arguments << currentFilePath << "-o" << currentFilePath + ".out";

    compilerProcess->start(compiler, arguments);
}

void MainWindow::onRunClicked()
{
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先保存文件");
        return;
    }

    QString program = currentFilePath + ".out";
    outputConsole->append("[" + QDateTime::currentDateTime().toString() + "] 开始运行...");

    compilerProcess->start(program);
}

void MainWindow::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        if (exitCode == 0) {
            outputConsole->append("操作成功完成");
            outputConsole->append(compilerProcess->readAllStandardOutput());
        } else {
            outputConsole->append("操作失败，退出代码: " + QString::number(exitCode));
        }
    } else {
        outputConsole->append("进程异常终止");
    }
    outputConsole->append(compilerProcess->readAllStandardError());
}

void MainWindow::onProcessErrorOccurred(QProcess::ProcessError error)
{
    outputConsole->append("错误: " + compilerProcess->errorString());
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
    } else {
        QMessageBox::warning(this, "错误", "无法打开文件: " + file.errorString());
    }
}
