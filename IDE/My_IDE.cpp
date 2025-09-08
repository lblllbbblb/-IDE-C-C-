#include "My_IDE.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QFileInfo> // 用于获取文件名

My_IDE::My_IDE(QMainWindow *parent)
    : QMainWindow(parent)
{
    // 创建 QTabWidget 作为中心部件
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // 初始化 TabWidgetManager
    m_tabManager = new TabWidgetManager(m_tabWidget, this);

    // 默认创建一个新文件
    m_tabManager->newFile();

    // 连接 TabManager 的信号，用于更新状态栏和动作状态
    connect(m_tabManager, &TabWidgetManager::currentEditorChanged, this, &My_IDE::on_currentEditorChanged);

    // 创建状态栏
    QStatusBar *statusBar = new QStatusBar;
    setStatusBar(statusBar);

    // 连接光标位置变化信号 (连接到 TabManager 发出的当前编辑器的信号)
    // 确保在 on_currentEditorChanged 中连接此信号，因为编辑器会动态变化

    // 初始化菜单系统
    initMenuSystem();

    // 设置初始字体大小
    fontsize = 12; // 默认字体大小
    m_tabManager->setFontSize(fontsize);

    // 初始状态更新，以防第一个Tab没有触发信号
    on_currentEditorChanged(m_tabManager->currentEditor());
}

void My_IDE::initMenuSystem()
{
    // 文件菜单
    file = menuBar()->addMenu("文件");
    file_new = new QAction("新建", this); // 新增新建文件动作
    file_open = new QAction("打开", this);
    file_save = new QAction("保存", this);
    file_othersave = new QAction("另存为", this);
    file_closeTab = new QAction("关闭当前文件", this); // 新增关闭 Tab 动作
    file_exit = new QAction("退出", this);

    // 添加快捷键
    file_new->setShortcut(tr("Ctrl+N"));
    file_open->setShortcut(tr("Ctrl+O"));
    file_save->setShortcut(tr("Ctrl+S"));
    file_othersave->setShortcut(tr("Ctrl+Shift+S"));
    file_closeTab->setShortcut(tr("Ctrl+W")); // 关闭 Tab 快捷键

    // 添加菜单项
    file->addActions({file_new, file_open, file_save, file_othersave});
    file->addSeparator();
    file->addAction(file_closeTab);
    file->addSeparator();
    file->addAction(file_exit);

    // 编辑菜单
    edit = menuBar()->addMenu("编辑");
    edit_copy = new QAction("复制", this);
    edit_paste = new QAction("粘贴", this);
    edit_cut = new QAction("剪切", this);
    edit_selectAll = new QAction("全选", this);
    edit->addActions({edit_selectAll, edit_copy, edit_paste, edit_cut});
    edit_findReplace = new QAction("查找/替换", this);
    edit_findReplace->setShortcut(tr("Ctrl+F"));
    edit->addAction(edit_findReplace);
    edit_undo = new QAction("撤销", this);
    edit_undo->setShortcut(tr("Ctrl+Z"));
    edit_redo = new QAction("恢复", this);
    edit_redo->setShortcut(tr("Ctrl+Y"));
    edit->addAction(edit_undo);
    edit->addAction(edit_redo);
    edit->addSeparator();

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

    // 设置菜单
    settings = this->menuBar()->addMenu("设置");
    settings_fontsize = new QAction("设置字体大小", this);
    settings->addAction(settings_fontsize);

    // 连接所有信号槽
    connectActions();
}

void My_IDE::connectActions()
{
    // 文件操作
    connect(file_new, &QAction::triggered, this, &My_IDE::on_new); // 连接新建文件动作
    connect(file_open, &QAction::triggered, this, &My_IDE::on_open);
    connect(file_save, &QAction::triggered, this, &My_IDE::on_save);
    connect(file_othersave, &QAction::triggered, this, &My_IDE::on_othersave);
    connect(file_closeTab, &QAction::triggered, this, &My_IDE::on_closeTab); // 连接关闭 Tab 动作
    connect(file_exit, &QAction::triggered, this, &My_IDE::on_exit);

    // 编辑操作
    connect(edit_copy, &QAction::triggered, this, &My_IDE::on_copy);
    connect(edit_paste, &QAction::triggered, this, &My_IDE::on_paste);
    connect(edit_cut, &QAction::triggered, this, &My_IDE::on_cut);
    connect(edit_selectAll, &QAction::triggered, this, &My_IDE::on_selectAll);
    connect(edit_findReplace, &QAction::triggered, this, &My_IDE::on_findReplace);

    // 构建操作
    connect(build_compile, &QAction::triggered, this, &My_IDE::on_compile);
    connect(build_run, &QAction::triggered, this, &My_IDE::on_run);
    connect(build_compileAndRun, &QAction::triggered, this, &My_IDE::on_compileAndRun);

    // 帮助
    connect(help_about, &QAction::triggered, this, &My_IDE::on_about);

    // 设置
    connect(settings_fontsize, &QAction::triggered, this, &My_IDE::on_fontsize);

    connect(edit_undo, &QAction::triggered, this, &My_IDE::on_undo);
    connect(edit_redo, &QAction::triggered, this, &My_IDE::on_redo);
}

void My_IDE::on_new()
{
    m_tabManager->newFile();
}

void My_IDE::on_open()
{
    QString selectedFilename = QFileDialog::getOpenFileName(this);
    if (!selectedFilename.isEmpty()) {
        m_tabManager->openFile(selectedFilename);
    }
}

void My_IDE::on_save()
{
    m_tabManager->saveCurrentFile();
}

void My_IDE::on_othersave()
{
    m_tabManager->saveCurrentFileAs();
}

void My_IDE::on_about()
{
    QMessageBox::information(this, "关于", "版权所有");
}

void My_IDE::on_exit()
{
    // TODO: 退出前检查所有文件是否保存
    exit(0);
}

void My_IDE::on_copy()
{
    m_tabManager->copy();
}

void My_IDE::on_paste()
{
    m_tabManager->paste();
}

void My_IDE::on_cut()
{
    m_tabManager->cut();
}

void My_IDE::on_selectAll()
{
    m_tabManager->selectAll();
}

void My_IDE::on_compile()
{
    m_tabManager->compileCurrentFile();
}

void My_IDE::on_run()
{
    m_tabManager->runCurrentFile();
}

void My_IDE::on_compileAndRun()
{
    m_tabManager->compileAndRunCurrentFile();
}

void My_IDE::on_findReplace()
{
    m_tabManager->showFindReplaceDialog();
}

void My_IDE::on_fontsize()
{
    bool ok;
    int newFontSize = QInputDialog::getInt(this, "设置字体大小", "请输入字体大小", fontsize, 6, 72, 2, &ok);
    if (ok) {
        fontsize = newFontSize;
        m_tabManager->setFontSize(fontsize);
        on_currentEditorChanged(m_tabManager->currentEditor());
        QMessageBox::information(this, "设置成功", "设置已生效");
    } else {
        QMessageBox::information(this, "取消", "设置已取消");
    }
}

void My_IDE::on_undo()
{
    m_tabManager->undo();
}

void My_IDE::on_redo()
{
    m_tabManager->redo();
}

void My_IDE::on_closeTab()
{
    m_tabManager->closeCurrentTab();
}

void My_IDE::on_currentEditorChanged(CodeEditor *editor)
{
    // 断开旧的连接，连接新的编辑器光标信号
    static QMetaObject::Connection connection;
    if (connection) {
        disconnect(connection);
    }

    if (editor) {
        connection = connect(editor, &QPlainTextEdit::cursorPositionChanged, [=]() {
            int line = editor->textCursor().blockNumber() + 1;
            int col = editor->textCursor().columnNumber() + 1;
            statusBar()->showMessage(QString("行: %1, 列: %2").arg(line).arg(col));
        });
        // 更新窗口标题
        QString filePath = m_tabManager->getCurrentFilePath();
        if (!filePath.isEmpty()) {
            setWindowTitle("My IDE - " + QFileInfo(filePath).fileName());
        } else {
            setWindowTitle("My IDE - " + m_tabWidget->tabText(m_tabWidget->currentIndex()));
        }
    } else {
        statusBar()->showMessage("没有打开的文件");
        setWindowTitle("My IDE");
    }
}

My_IDE::~My_IDE()
{
    // m_tabWidget 和 m_tabManager 会自动被删除，因为它们有父对象
}
