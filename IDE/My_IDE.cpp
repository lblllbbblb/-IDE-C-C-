#include "My_IDE.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QVBoxLayout>
#include <QStatusBar>

My_IDE::My_IDE(QMainWindow *parent)
    : QMainWindow(parent)
{
    // 创建主编辑器（替换原来的text1）
    text1 = new CodeEditor(this);
    setCentralWidget(text1);  // 设置为中心部件

    // 设置编辑器属性
    text1->setFont(QFont("Consolas", 12));
    text1->setLineWrapMode(QPlainTextEdit::NoWrap);

    // 初始化语法高亮器（关联到编辑器文档）
    cppHighlighter = new CppHighlighter(text1->document());

    // 创建状态栏
    QStatusBar *statusBar = new QStatusBar;
    setStatusBar(statusBar);

    // 连接光标位置变化信号
    connect(text1, &QPlainTextEdit::cursorPositionChanged, [=](){
        int line = text1->textCursor().blockNumber() + 1;
        statusBar->showMessage(QString("当前行: %1").arg(line));
    });
    // 新增：初始化括号匹配器，并将其安装到 text1 上
    m_bracketMatcher = new BracketMatcher(text1, this); // 将 text1 和 this 作为父对象

    m_findReplaceDialog = new FindReplaceDialog(text1, this);

    // 初始化菜单系统
    initMenuSystem();
}

void My_IDE::initMenuSystem()
{
    // 文件菜单
    file = menuBar()->addMenu("文件");
    file_open = new QAction("打开", this);
    file_save = new QAction("保存", this);
    file_othersave = new QAction("另存为", this);
    file_exit = new QAction("退出", this);

    // 添加快捷键
    file_open->setShortcut(tr("Ctrl+O"));
    file_save->setShortcut(tr("Ctrl+S"));
    file_othersave->setShortcut(tr("Ctrl+Shift+S"));

    // 添加菜单项
    file->addActions({file_open, file_save, file_othersave});
    file->addSeparator();
    file->addAction(file_exit);

    // 编辑菜单
    edit = menuBar()->addMenu("编辑");
    edit_copy = new QAction("复制", this);
    edit_paste = new QAction("粘贴", this);
    edit_cut = new QAction("剪切", this);
    edit_selectAll = new QAction("全选", this);
    edit->addActions({edit_selectAll, edit_copy, edit_paste, edit_cut});
    edit_findReplace = new QAction("查找/替换", this); // 新增查找替换动作
    edit_findReplace->setShortcut(tr("Ctrl+F"));     // 设置查找快捷键 Ctrl+F
    edit->addAction(edit_findReplace);
    edit_undo = new QAction("撤销", this);
    edit_undo->setShortcut(tr("Ctrl+Z")); // 撤销快捷键 Ctrl+Z
    edit_redo = new QAction("恢复", this);
    edit_redo->setShortcut(tr("Ctrl+Y")); // 恢复快捷键 Ctrl+Y (或 Ctrl+Shift+Z)
    edit->addAction(edit_undo);
    edit->addAction(edit_redo);
    edit->addSeparator(); // 分隔符

    // 构建菜单
    build = menuBar()->addMenu("构建");
    build_compile = new QAction("编译", this);
    build_run = new QAction("运行", this);
    build_compileAndRun = new QAction("编译并运行", this);
    build_compile->setShortcut(tr("F9"));
    build_run->setShortcut(tr("F10"));
    build_compileAndRun->setShortcut(tr("F11"));
    build->addActions({build_compile, build_run, build_compileAndRun});

    // 帮助菜单
    help = menuBar()->addMenu("帮助");
    help_about = new QAction("关于", this);
    help_about->setShortcut(tr("Ctrl+H"));
    help->addAction(help_about);

    //设置菜单
    settings=this->menuBar()->addMenu("设置");
    settings_fontsize = new QAction("设置字体大小",this);
    settings->addAction(settings_fontsize);

    // 连接所有信号槽
    connectActions();
}

void My_IDE::connectActions()
{
    // 文件操作
    connect(file_open, &QAction::triggered, this, &My_IDE::on_open);
    connect(file_save, &QAction::triggered, this, &My_IDE::on_save);
    connect(file_othersave, &QAction::triggered, this, &My_IDE::on_othersave);
    connect(file_exit, &QAction::triggered, this, &My_IDE::on_exit);

    // 编辑操作
    connect(edit_copy, &QAction::triggered, text1, &QPlainTextEdit::copy);
    connect(edit_paste, &QAction::triggered, text1, &QPlainTextEdit::paste);
    connect(edit_cut, &QAction::triggered, text1, &QPlainTextEdit::cut);
    connect(edit_selectAll, &QAction::triggered, text1, &QPlainTextEdit::selectAll);
    connect(edit_findReplace, &QAction::triggered, this, &My_IDE::on_findReplace);

    // 构建操作
    connect(build_compile, &QAction::triggered, this, &My_IDE::on_compile);
    connect(build_run, &QAction::triggered, this, &My_IDE::on_run);
    connect(build_compileAndRun, &QAction::triggered, this, &My_IDE::on_compileAndRun);
    connect(build_compileAndRun, &QAction::triggered, this, &My_IDE::on_compileAndRun);

    // 帮助
    connect(help_about, &QAction::triggered, this, &My_IDE::on_about);

    //设置
    connect(settings_fontsize, &QAction::triggered, this, &My_IDE::on_fontsize);

    connect(edit_undo, &QAction::triggered, this, &My_IDE::on_undo);
    connect(edit_redo, &QAction::triggered, this, &My_IDE::on_redo);
}


void My_IDE::on_open()
{
    filename = QFileDialog::getOpenFileName();//打开一个标准文件对话框，返回值是用户选定的文件名

    if(filename.isEmpty()){
        return;
    }
    //filename.toStdString().data();//QSrting转化为const char*
    QString content;//Qt定义的字符串
    FILE *p = fopen(filename.toStdString().data(),"r");
    if(p == NULL){
        QMessageBox::information(this,"错误","打开文件失败\n请检查路径是否正确");
    }
    else
    {
        while(!feof(p)){
            char buf[1024] = { 0 };
            fgets(buf,sizeof(buf),p);
            content += buf;//将buf内容追加到content后面
        }
        fclose(p);
        text1 -> setPlainText(content);//将QString字符串放入text1里面
    }
}

void My_IDE::on_save()
{
    // 检查当前文件是否已保存
    if (filename.isEmpty()) {

        on_othersave();
        return;
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

void My_IDE::on_othersave()
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


void My_IDE::on_about()
{
    QMessageBox::information(this,"关于","版权所有");
}

void My_IDE::on_exit()
{
    exit(0);
}

void My_IDE::on_copy()
{
    text1->copy();
}

void My_IDE::on_paste()
{
    text1->paste();
}

void My_IDE::on_cut()
{
    text1->cut();
}

void My_IDE::on_selectAll()
{
    text1->selectAll();
}

void My_IDE::on_compile()
{
    if (filename.isEmpty()) {

        on_othersave();
        return;
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
    }
    //先实现保存


    QString destname = filename;
    destname.replace(".c",".exe");
    QString command = "gcc -o " + destname + " "+ filename;
    system(command.toStdString().data());
}

void My_IDE::on_run()
{
    QString destname = filename;
    destname.replace(".c",".exe");
    system(destname.toStdString().data());
}

void My_IDE::on_compileAndRun(){
    on_compile();
    on_run();
}
// 显示查找替换对话框的槽函数
void My_IDE::on_findReplace()
{
    // 如果对话框是第一次创建，或者被删除了，重新创建
    if (!m_findReplaceDialog) {
        m_findReplaceDialog = new FindReplaceDialog(text1, this);
    }
    m_findReplaceDialog->show(); // 显示非模态对话框
    m_findReplaceDialog->raise(); // 将对话框带到前面
    m_findReplaceDialog->activateWindow(); // 激活对话框窗口
}

void My_IDE::on_fontsize()
{
    bool ok;
    fontsize = QInputDialog::getInt(nullptr,"设置字体大小","请输入字体大小",fontsize,6,72,2,&ok);
    if(ok)
    {
        QMessageBox::information(nullptr,"设置成功","设置已生效");
    }
    else
    {
        QMessageBox::information(nullptr,"取消","设置已取消");
    }
    QFont f;
    f.setPixelSize(fontsize);
    text1->setFont(f);
    this->setCentralWidget(text1);
}

// --- 新增：撤销操作的槽函数 ---
void My_IDE::on_undo()
{
    text1->undo(); // 调用 QTextEdit 的内置 undo 槽
}

// --- 新增：恢复操作的槽函数 ---
void My_IDE::on_redo()
{
    text1->redo(); // 调用 QTextEdit 的内置 redo 槽
}







My_IDE::~My_IDE()
{
    // 自动管理Qt对象，无需手动删除
}
//修改字体大小的槽函数

