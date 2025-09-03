#include "mainwindow.h"
#include "./ui_mainwindow.h" // 包含由 Qt Designer 生成的 UI 头文件

// MainWindow 类的构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) // 调用 QMainWindow 的构造函数
    , ui(new Ui::MainWindow) // 初始化 UI 对象
{
    ui->setupUi(this); // 设置由 Qt Designer 生成的界面

    // 创建并设置文本编辑组件
    text1 = new QTextEdit;
    QFont f;
    f.setPixelSize(18); // 设置字体大小为18像素
    text1->setFont(f);
    this->setCentralWidget(text1); // 将 QTextEdit 设置为中心部件

    // 在 codeEditor 创建后，初始化语法高亮器
    cppHighlighter = new CppHighlighter(text1->document()); // 将高亮器关联到 codeEditor 的文档

    // —— 创建菜单栏上的主菜单 ——
    file = this->menuBar()->addMenu("文件"); // “文件”菜单
    edit = this->menuBar()->addMenu("编辑"); // “编辑”菜单
    build =this->menuBar()->addMenu("构建"); // “构建”菜单
    help = this->menuBar()->addMenu("帮助"); // “帮助”菜单

    // —— 文件菜单动作 ——
    file_open = new QAction("打开", this);
    file_open->setShortcut(tr("Ctrl+O")); // 设置打开的快捷键 Ctrl+O

    file_save = new QAction("保存", this);
    file_save->setShortcut(tr("Ctrl+S")); // 设置保存的快捷键 Ctrl+S

    file_exit = new QAction("退出", this);

    file->addAction(file_open);  // 添加“打开”动作
    file->addAction(file_save);  // 添加“保存”动作
    file->addSeparator();        // 添加分隔符
    file->addAction(file_exit);  // 添加“退出”动作

    // —— 构建菜单动作 ——
    build_compile = new QAction("编译",this);
    build->addAction(build_compile); // 添加“编译”动作

    build_run = new QAction("运行",this);
    build->addAction(build_run);     // 添加“运行”动作

    // —— 编辑菜单动作 ——
    QAction *copy_act = new QAction("复制", this);
    copy_act->setShortcut(tr("Ctrl+C")); // 设置复制的快捷键 Ctrl+C

    QAction *paste_act = new QAction("粘贴", this);
    paste_act->setShortcut(tr("Ctrl+V")); // 设置粘贴的快捷键 Ctrl+V

    QAction *cut_act = new QAction("剪切", this);
    cut_act->setShortcut(tr("Ctrl+X"));   // 设置剪切的快捷键 Ctrl+X

    QAction *selectAll_act = new QAction("全选", this);
    selectAll_act->setShortcut(tr("Ctrl+A")); // 设置全选的快捷键 Ctrl+A

    edit->addAction(copy_act);       // 添加“复制”动作
    edit->addAction(cut_act);        // 添加“剪切”动作
    edit->addAction(paste_act);      // 添加“粘贴”动作
    edit->addAction(selectAll_act);  // 添加“全选”动作

    // —— 帮助菜单动作 ——
    QAction *help_about = new QAction("关于", this);
    help->addAction(help_about);     // 添加“关于”动作

    // —— 动作与槽函数关联（连接信号与槽） ——
    connect(file_open, &QAction::triggered, this, &MainWindow::on_open);       // 连接“打开”动作到 on_open 槽
    connect(file_save, &QAction::triggered, this, &MainWindow::on_save);       // 连接“保存”动作到 on_save 槽
    connect(file_exit, &QAction::triggered, this, &MainWindow::close);         // 连接“退出”动作到 close 槽 (关闭窗口)

    connect(copy_act, &QAction::triggered, this, &MainWindow::on_copy);        // 连接“复制”动作到 on_copy 槽
    connect(paste_act, &QAction::triggered, this, &MainWindow::on_paste);      // 连接“粘贴”动作到 on_paste 槽
    connect(cut_act, &QAction::triggered, this, &MainWindow::on_cut);          // 连接“剪切”动作到 on_cut 槽
    connect(selectAll_act, &QAction::triggered, this, &MainWindow::on_selectAll); // 连接“全选”动作到 on_selectAll 槽
    connect(help_about, &QAction::triggered, this, &MainWindow::on_about);    // 连接“关于”动作到 on_about 槽

    connect(build_compile, &QAction::triggered, this, &MainWindow::on_compile); // 连接“编译”动作到 on_compile 槽
    connect(build_run, &QAction::triggered, this, &MainWindow::on_run);         // 连接“运行”动作到 on_run 槽
}

// MainWindow 类的析构函数
MainWindow::~MainWindow()
{
    delete text1; // 释放 QTextEdit 对象
    delete ui;    // 释放 UI 对象
}

// —— 槽函数实现 ——

// 打开文件的槽函数
void MainWindow::on_open()
{
    // 弹出文件对话框，让用户选择要打开的文件
    // 参数1: 父窗口
    // 参数2: 对话框标题
    // 参数3: 默认打开路径 (QString() 表示当前目录)
    // 参数4: 文件过滤器 ("Text Files (*.txt);;All Files (*)" 表示可以筛选 .txt 文件和所有文件)
    filename = QFileDialog::getOpenFileName(this, "打开文件", QString(), "C/C++ Files (*.c *.cpp *.h);;Text Files (*.txt);;All Files (*)");
    if (filename.isEmpty()) {
        return; // 如果用户取消选择，则返回
    }

    // 使用 C 标准库的 fopen 打开文件
    // filename.toUtf8().data() 将 QString 转换为 UTF-8 编码的 C 字符串
    FILE *p = fopen(filename.toUtf8().data(), "r");
    if (p == NULL)
    {
        QMessageBox::information(this, "错误", "打开文件失败");
        return;
    }

    text1->clear(); // 清空文本编辑框原有内容
    char buf[1024]; // 定义缓冲区用于读取文件
    // 循环读取文件直到文件结束
    while (!feof(p))
    {
        // 每次读取一行内容到缓冲区
        if (fgets(buf, sizeof(buf), p) != NULL) {
            // 将读取的内容（UTF-8 编码）追加到文本编辑框
            text1->append(QString::fromUtf8(buf));
        }
    }
    fclose(p); // 关闭文件
}

// 保存文件的槽函数
void MainWindow::on_save()
{
    // 弹出文件对话框，让用户选择保存文件的位置和名称
    filename = QFileDialog::getSaveFileName(this, "保存文件", QString(), "C/C++ Files (*.c *.cpp *.h);;Text Files (*.txt);;All Files (*)");
    if(filename.isEmpty()){
        return; // 如果用户取消选择，则返回
    }

    // 使用 C 标准库的 fopen 以写入模式打开文件
    FILE *p = fopen(filename.toUtf8().data(),"w");
    if(p == NULL){
        QMessageBox::information(this,"错误","保存文件失败");
        return; // 如果文件打开失败，则返回
    } else {
        // 获取 QTextEdit 中的纯文本内容，并转换为 UTF-8 编码的 QByteArray
        QByteArray textData = text1->toPlainText().toUtf8();
        // 将文本数据写入文件
        fputs(textData.constData(), p);
        fclose(p); // 关闭文件
        QMessageBox::information(this, "成功", "文件保存成功");
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
    QMessageBox::information(this, "关于", "这是一个简单的C/C++文本编辑器，具有编译和运行功能。");
}

// 编译代码的槽函数
void MainWindow::on_compile()
{
    // 检查当前文件是否已保存
    if (filename.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先保存文件再编译。");
        return;
    }

    // 获取源文件的完整路径
    QString sourceFile = filename;

    // 构建可执行文件的名称和路径
    QFileInfo fileInfo(sourceFile);
    QString baseName = fileInfo.completeBaseName(); // 获取文件名（不带扩展名）
    // 将可执行文件放在与源文件相同的目录下
    QString executableFile = fileInfo.absolutePath() + QDir::separator() + baseName;

    // 根据操作系统添加可执行文件后缀
#ifdef Q_OS_WIN
    executableFile += ".exe"; // Windows 系统
#else
    // Linux/macOS 无需 .exe 后缀，但可以根据需要添加
#endif

    // 构建 g++ 编译命令
    // 例如：g++ -o "path/to/myprogram.exe" "path/to/myprogram.c"
    QString command = "g++ -o \"" + executableFile + "\" \"" + sourceFile + "\"";
    QMessageBox::information(this, "提示", "编译命令: " + command);

    // 执行编译命令
    // system() 函数执行外部命令并等待其完成
    int result = system(command.toUtf8().constData());

    // 根据编译结果显示消息
    if (result == 0) {
        QMessageBox::information(this, "编译", "编译成功！生成的可执行文件为:\n" + executableFile);
    } else {
        QMessageBox::critical(this, "编译", "编译失败。请检查代码和编译器设置。");
    }
}

// 运行程序的槽函数
void MainWindow::on_run()
{
    // 检查当前文件是否已保存
    if (filename.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先编译文件再运行。");
        return;
    }

    // 构建可执行文件的名称和路径 (与编译时逻辑一致)
    QFileInfo fileInfo(filename);
    QString baseName = fileInfo.completeBaseName();
    QString executablePath = fileInfo.absolutePath(); // 可执行文件所在的目录
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
    // 注意：这里已经包含了 chcp 65001 的逻辑
    commandToExecute = "cmd /c \"chcp 65001 > nul && \"" + executableFile + "\" & pause\"";
    result = system(commandToExecute.toUtf8().constData());
#else
    // 对于 Linux/macOS，直接运行可执行文件。
    // 在这些系统上，通常终端不会在程序结束后立即关闭。
    commandToExecute = executableFile;
    result = system(commandToExecute.toUtf8().constData());
#endif

    // 根据运行结果显示消息
    if (result == 0) {
        QMessageBox::information(this, "运行", "程序运行完成。");
    } else {
        QMessageBox::critical(this, "运行", "程序运行失败或异常终止。");
    }
}
