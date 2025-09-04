#include "mainwindow.h"
#include "./ui_mainwindow.h"

// MainWindow 类的构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    text1 = new QTextEdit;
    QFont f;
    f.setPixelSize(18);
    text1->setFont(f);
    this->setCentralWidget(text1);

    cppHighlighter = new CppHighlighter(text1->document());

    // 初始化查找替换对话框，并传递 text1 指针
    m_findReplaceDialog = new FindReplaceDialog(text1, this);

    // 新增：初始化括号匹配器，并将其安装到 text1 上
    m_bracketMatcher = new BracketMatcher(text1, this); // 将 text1 和 this 作为父对象

    // —— 创建菜单栏上的主菜单 ——
    file = this->menuBar()->addMenu("文件");
    edit = this->menuBar()->addMenu("编辑");
    build =this->menuBar()->addMenu("构建");
    help = this->menuBar()->addMenu("帮助");

    // —— 文件菜单动作 ——
    file_open = new QAction("打开", this);
    file_open->setShortcut(tr("Ctrl+O"));

    file_save = new QAction("保存", this);
    file_save->setShortcut(tr("Ctrl+S"));

    file_anothersave = new QAction("另存为", this); // 更改为“另存为”更符合语义
    file_anothersave->setShortcut(tr("Ctrl+Shift+S")); // 快捷键保持不变

    file_exit = new QAction("退出", this);

    file->addAction(file_open);
    file->addAction(file_save);
    file->addAction(file_anothersave);
    file->addSeparator();
    file->addAction(file_exit);

    // —— 构建菜单动作 ——
    build_compile = new QAction("编译",this);
    build->addAction(build_compile);

    build_run = new QAction("运行",this);
    build->addAction(build_run);

    build_compileAndRun = new QAction("编译并运行", this);
    build->addAction(build_compileAndRun);

    // —— 编辑菜单动作 ——
    edit_copy = new QAction("复制", this);
    edit_copy->setShortcut(tr("Ctrl+C"));

    edit_paste = new QAction("粘贴", this);
    edit_paste->setShortcut(tr("Ctrl+V"));

    edit_cut = new QAction("剪切", this);
    edit_cut->setShortcut(tr("Ctrl+X"));

    edit_selectAll = new QAction("全选", this);
    edit_selectAll->setShortcut(tr("Ctrl+A"));

    edit_findReplace = new QAction("查找/替换", this); // 新增查找替换动作
    edit_findReplace->setShortcut(tr("Ctrl+F"));     // 设置查找快捷键 Ctrl+F

    edit->addAction(edit_copy);
    edit->addAction(edit_cut);
    edit->addAction(edit_paste);
    edit->addSeparator(); // 分隔符
    edit->addAction(edit_selectAll);
    edit->addSeparator(); // 分隔符
    edit->addAction(edit_findReplace); // 添加查找替换动作

    // —— 帮助菜单动作 ——
    help_about = new QAction("关于", this);
    help->addAction(help_about);

    // —— 动作与槽函数关联（连接信号与槽） ——
    connect(file_open, &QAction::triggered, this, &MainWindow::on_open);
    connect(file_save, &QAction::triggered, this, &MainWindow::on_save);
    connect(file_anothersave, &QAction::triggered, this, &MainWindow::on_anothersave);
    connect(file_exit, &QAction::triggered, this, &MainWindow::close);

    connect(edit_copy, &QAction::triggered, this, &MainWindow::on_copy);
    connect(edit_paste, &QAction::triggered, this, &MainWindow::on_paste);
    connect(edit_cut, &QAction::triggered, this, &MainWindow::on_cut);
    connect(edit_selectAll, &QAction::triggered, this, &MainWindow::on_selectAll);
    connect(edit_findReplace, &QAction::triggered, this, &MainWindow::on_findReplace); // 连接查找替换动作

    connect(help_about, &QAction::triggered, this, &MainWindow::on_about);

    connect(build_compile, &QAction::triggered, this, &MainWindow::on_compile);
    connect(build_run, &QAction::triggered, this, &MainWindow::on_run);
    connect(build_compileAndRun, &QAction::triggered, this, &MainWindow::on_compileAndRun);
}

// MainWindow 类的析构函数
MainWindow::~MainWindow()
{
    // text1 和 m_findReplaceDialog 会被父对象(MainWindow)自动删除，因为它们都指定了父对象。
    // 但是 text1 在构造函数中被 this->setCentralWidget(text1); 设为中心部件，
    // Qt 会负责它的生命周期。m_findReplaceDialog 也指定了 this 作为父对象。
    // 理论上不需要手动 delete。
    // 为了安全起见，显式 delete 也是可以的，但要注意避免双重删除。
    // 这里保持 Qt 的父子关系管理原则，不显式删除。
    delete ui; // ui 是一个裸指针，必须手动删除
}

// —— 槽函数实现 ——

// 打开文件的槽函数
void MainWindow::on_open()
{
    filename = QFileDialog::getOpenFileName(this, "打开文件", QString(), "C/C++ Files (*.c *.cpp *.h);;Text Files (*.txt);;All Files (*)");
    if (filename.isEmpty()) {
        return;
    }

    FILE *p = fopen(filename.toUtf8().data(), "r");
    if (p == NULL)
    {
        QMessageBox::information(this, "错误", "打开文件失败");
        return;
    }

    text1->clear();
    char buf[1024];
    while (!feof(p))
    {
        if (fgets(buf, sizeof(buf), p) != NULL) {
            text1->append(QString::fromUtf8(buf));
        }
    }
    fclose(p);
}

void MainWindow::on_save()
{
    if(filename.isEmpty()){
        filename = QFileDialog::getSaveFileName(this, "保存文件", QString(), "C/C++ Files (*.c *.cpp *.h);;Text Files (*.txt);;All Files (*)");
        if(filename.isEmpty()){
            return;
        }

        FILE *p = fopen(filename.toUtf8().data(),"w");
        if(p == NULL){
            QMessageBox::information(this,"错误","保存文件失败");
            return;
        } else {
            QByteArray textData = text1->toPlainText().toUtf8();
            fputs(textData.constData(), p);
            fclose(p);
            QMessageBox::information(this, "成功", "文件保存成功");
        }
        return;
    }

    FILE *p = fopen(filename.toUtf8().data(),"w");
    if(p == NULL){
        QMessageBox::information(this,"错误","保存文件失败");
        return;
    } else {
        QByteArray textData = text1->toPlainText().toUtf8();
        fputs(textData.constData(), p);
        fclose(p);
        QMessageBox::information(this, "成功", "文件保存成功");
    }
}

// 另存为文件的槽函数
void MainWindow::on_anothersave()
{
    QString tempFilename = QFileDialog::getSaveFileName(this, "另存为文件", QString(), "C/C++ Files (*.c *.cpp *.h);;Text Files (*.txt);;All Files (*)");
    if(tempFilename.isEmpty()){
        return;
    }

    FILE *p = fopen(tempFilename.toUtf8().data(),"w");
    if(p == NULL){
        QMessageBox::information(this,"错误","保存文件失败");
        return;
    } else {
        QByteArray textData = text1->toPlainText().toUtf8();
        fputs(textData.constData(), p);
        fclose(p);
        filename = tempFilename; // 更新当前文件名，以便后续保存操作
        QMessageBox::information(this, "成功", "文件已另存成功");
    }
}

// 复制选中文本的槽函数
void MainWindow::on_copy()
{
    text1->copy();
}

// 粘贴剪贴板内容的槽函数
void MainWindow::on_paste()
{
    text1->paste();
}

// 剪切选中文本的槽函数
void MainWindow::on_cut()
{
    text1->cut();
}

// 全选文本编辑框所有内容的槽函数
void MainWindow::on_selectAll()
{
    text1->selectAll();
}

// 显示“关于”信息的槽函数
void MainWindow::on_about()
{
    QMessageBox::information(this, "关于", "这是一个简单的C/C++文本编辑器，具有语法高亮、查找替换、编译和运行功能。");
}

// 显示查找替换对话框的槽函数
void MainWindow::on_findReplace()
{
    // 如果对话框是第一次创建，或者被删除了，重新创建
    if (!m_findReplaceDialog) {
        m_findReplaceDialog = new FindReplaceDialog(text1, this);
    }
    m_findReplaceDialog->show(); // 显示非模态对话框
    m_findReplaceDialog->raise(); // 将对话框带到前面
    m_findReplaceDialog->activateWindow(); // 激活对话框窗口
}

// *** 编译代码的槽函数 ***
void MainWindow::on_compile()
{
    if (filename.isEmpty()) {
        on_anothersave(); // 调用另存为函数
        if (filename.isEmpty()) { // 如果用户在另存为对话框中取消，则再次返回
            return;
        }
    }

    // 先保存当前文本到文件
    FILE *p = fopen(filename.toUtf8().data(),"w");
    if(p == NULL){
        QMessageBox::information(this,"错误","保存文件失败");
        return;
    } else {
        QByteArray textData = text1->toPlainText().toUtf8();
        fputs(textData.constData(), p);
        fclose(p);
    }

    // 构建可执行文件的名称和路径
    QFileInfo fileInfo(filename);
    QString baseName = fileInfo.completeBaseName(); // 获取文件名（不带扩展名）
    QString executableFile = fileInfo.absolutePath() + QDir::separator() + baseName;

#ifdef Q_OS_WIN
    executableFile += ".exe"; // Windows 系统
#endif

    // 使用 g++ 进行编译（因为你的原始代码是 C++ 项目，这里继续使用 g++）
    // 如果你确定只编译 C 文件，可以将 "g++" 改为 "gcc"
    QString command = "g++ -o \"" + executableFile + "\" \"" + filename + "\"";
    QMessageBox::information(this, "提示", "编译命令: " + command);

    int result = system(command.toUtf8().constData());

    if (result == 0) {
        QMessageBox::information(this, "编译", "编译成功！生成的可执行文件为:\n" + executableFile);
    } else {
        QMessageBox::critical(this, "编译", "编译失败。请检查代码和编译器设置。");
    }
}

// *** 运行程序的槽函数 ***
void MainWindow::on_run()
{
    if (filename.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先编译或保存文件再运行。");
        return;
    }

    // 构建可执行文件的名称和路径 (与编译时逻辑一致)
    QFileInfo fileInfo(filename);
    QString baseName = fileInfo.completeBaseName();
    QString executablePath = fileInfo.absolutePath();
    QString executableFile = executablePath + QDir::separator() + baseName;

#ifdef Q_OS_WIN
    executableFile += ".exe"; // Windows 系统
#endif

    // 检查可执行文件是否存在
    if (!QFile::exists(executableFile)) {
        QMessageBox::warning(this, "运行", "可执行文件不存在。请先编译代码。");
        return;
    }

    QMessageBox::information(this, "提示", "运行文件: " + executableFile);

    int result;
    QString commandToExecute;
#ifdef Q_OS_WIN
    // 对于 Windows，我们通过命令行运行程序，并在其后加上 " & pause"
    // 并且在执行程序前设置控制台为 UTF-8 编码
    commandToExecute = "cmd /c \"chcp 65001 > nul && \"" + executableFile + "\" & pause\"";
    result = system(commandToExecute.toUtf8().constData());
#else
    // 对于 Linux/macOS，直接运行可执行文件。
    commandToExecute = executableFile;
    result = system(commandToExecute.toUtf8().constData());
#endif

    if (result == 0) {
        QMessageBox::information(this, "运行", "程序运行完成。");
    } else {
        QMessageBox::critical(this, "运行", "程序运行失败或异常终止。");
    }
}

// *** 编译并运行代码的槽函数 ***
void MainWindow::on_compileAndRun()
{
    on_compile(); // 调用编译函数
    // 只有当编译成功后才尝试运行
    QFileInfo fileInfo(filename);
    QString baseName = fileInfo.completeBaseName();
    QString executableFile = fileInfo.absolutePath() + QDir::separator() + baseName;
#ifdef Q_OS_WIN
    executableFile += ".exe";
#endif

    if (QFile::exists(executableFile)) {
        on_run(); // 调用运行函数
    } else {
        QMessageBox::warning(this, "编译并运行", "编译失败，无法运行。");
    }
}
