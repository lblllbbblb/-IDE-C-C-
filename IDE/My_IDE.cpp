#include "My_IDE.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QFileInfo>
#include <QInputDialog>

My_IDE::My_IDE(QMainWindow *parent)
    : QMainWindow(parent), m_isDarkMode(false) // 默认浅色模式
{
    // 创建 QTabWidget 作为中心部件
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // 初始化 TabWidgetManager
    m_tabManager = new TabWidgetManager(m_tabWidget, this);

    // 初始化 Formatter
    m_formatter = new Formatter(); // 新增：初始化 Formatter

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
    fontsize = 20; // 默认字体大小
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
    edit_formatCode = new QAction("格式化代码", this); // 新增：格式化代码动作
    edit_formatCode->setShortcut(tr("Ctrl+Shift+F")); // 快捷键
    edit->addAction(edit_formatCode); // 添加到编辑菜单

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

    // 在设置菜单中添加颜色模式切换
    settings_toggleColor = new QAction("切换深色/浅色模式", this);
    settings->addAction(settings_toggleColor);

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
    connect(edit_formatCode, &QAction::triggered, this, &My_IDE::on_formatCode); // 新增：连接格式化代码动作

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

    // 连接颜色模式切换动作
    connect(settings_toggleColor, &QAction::triggered, this, &My_IDE::on_toggleColorMode);
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

// 新增：格式化代码槽函数
void My_IDE::on_formatCode()
{
    CodeEditor *currentEditor = m_tabManager->currentEditor();
    if (currentEditor) {
        QString originalCode = currentEditor->toPlainText();
        QString formattedCode = m_formatter->formatCppCode(originalCode); // 调用 Formatter 进行格式化
        currentEditor->setPlainText(formattedCode); // 将格式化后的代码设置回编辑器
        QMessageBox::information(this, "格式化", "代码已格式化。");
    } else {
        QMessageBox::warning(this, "格式化", "没有打开的文件可供格式化。");
    }
}

void My_IDE::on_currentEditorChanged(CodeEditor *editor)
{
    // 断开旧的连接，连接新的编辑器光标信号
    static QMetaObject::Connection connection;
    if (connection) {
        disconnect(connection);
    }

    if (editor) {
        // 创建更新状态栏的lambda函数
        auto updateStatusBar = [=]() {
            int line = editor->textCursor().blockNumber() + 1;
            int col = calculateVisualColumn(editor); // 使用新的视觉列计算方法
            statusBar()->showMessage(QString("行: %1, 列: %2").arg(line).arg(col));
        };

        // 连接光标位置变化信号
        connection = connect(editor, &QPlainTextEdit::cursorPositionChanged, updateStatusBar);

        // 立即更新一次状态栏
        updateStatusBar();

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

int My_IDE::calculateVisualColumn(CodeEditor *editor)
{
    QTextCursor cursor = editor->textCursor();
    QTextBlock block = cursor.block();
    int positionInBlock = cursor.position() - block.position();
    QString text = block.text().left(positionInBlock);

    int visualColumn = 0;

    // 获取编辑器的 Tab 停止距离（以像素为单位）
    qreal tabStopDistance = editor->tabStopDistance();

    // 获取字体度量
    QFontMetrics fm(editor->font());

    // 计算一个空格的宽度
    int spaceWidth = fm.horizontalAdvance(' ');

    // 计算 Tab 大小（以空格数为单位）
    int tabSize = (spaceWidth > 0) ? qRound(tabStopDistance / spaceWidth) : 4;

    for (int i = 0; i < text.length(); i++) {
        if (text[i] == '\t') {
            // Tab 字符：前进到下一个 Tab 停止位置
            visualColumn = (visualColumn / tabSize + 1) * tabSize;
        } else {
            // 普通字符：前进一个位置
            visualColumn++;
        }
    }

    return visualColumn + 1; // 列号从 1 开始
}

// 重写滚轮事件处理函数
void My_IDE::wheelEvent(QWheelEvent *event)
{
    // 检查是否按下了 Ctrl 键
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y(); // 获取滚轮的滚动量

        if (delta > 0) { // 滚轮向上滚动
            fontsize = qMin(72, fontsize + 2); // 字体大小最大72
        } else { // 滚轮向下滚动
            fontsize = qMax(6, fontsize - 2); // 字体大小最小6
        }
        m_tabManager->setFontSize(fontsize); // 更新所有编辑器的字体大小
        // 重新触发当前编辑器改变的信号，以更新状态栏和窗口标题等
        on_currentEditorChanged(m_tabManager->currentEditor());
        event->accept(); // 接受事件，表示已处理
    } else {
        // 如果没有按下 Ctrl 键，则调用基类的处理函数
        QMainWindow::wheelEvent(event);
    }
}

void My_IDE::on_toggleColorMode()
{
    // 切换模式状态
    m_isDarkMode = !m_isDarkMode;

    // 应用颜色设置
    applyColorMode(m_isDarkMode);

    // 显示提示
    QString mode = m_isDarkMode ? "深色" : "浅色";
    QMessageBox::information(this, "模式切换", QString("已切换到%1模式").arg(mode));
}

void My_IDE::applyColorMode(bool darkMode)
{
    // 定义黑白两种模式的颜色
    QString bgColor, textColor, menuColor, statusColor;

    if (darkMode) {
        // 深色模式：黑底白字
        bgColor = "black";
        textColor = "white";
        menuColor = "#333333"; // 深灰色菜单
        statusColor = "#444444"; // 深灰色状态栏
    } else {
        // 浅色模式：白底黑字
        bgColor = "white";
        textColor = "black";
        menuColor = "#f0f0f0"; // 浅灰色菜单
        statusColor = "#e0e0e0"; // 浅灰色状态栏
    }

    // 应用到所有编辑器
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        // 假设每个标签页的部件是QPlainTextEdit或其派生类
        QPlainTextEdit *editor = qobject_cast<QPlainTextEdit*>(m_tabWidget->widget(i));
        if (editor) {
            editor->setStyleSheet(QString("background-color: %1; color: %2;")
                                      .arg(bgColor).arg(textColor));
        }
    }

    // 应用到菜单栏
    menuBar()->setStyleSheet(QString("QMenuBar { background-color: %1; color: %2; }"
                                     "QMenu { background-color: %1; color: %2; }"
                                     "QMenu::item:selected { background-color: #666666; }")
                                 .arg(menuColor).arg(textColor));

    // 应用到状态栏
    statusBar()->setStyleSheet(QString("QStatusBar { background-color: %1; color: %2; }")
                                   .arg(statusColor).arg(textColor));
}

My_IDE::~My_IDE()
{
    // m_formatter 会自动被删除，因为它是 My_IDE 的成员变量
    // m_tabWidget 和 m_tabManager 会自动被删除，因为它们有父对象
}
