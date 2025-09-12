#include "My_IDE.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QFileInfo>
#include <QInputDialog>
#include <QDockWidget> // 新增：用于调试输出窗口
#include <QTextCharFormat> // 用于行高亮
#include <QTextCursor>
#include <QApplication> // 用于 QApplication::quit()

My_IDE::My_IDE(QMainWindow *parent)
    : QMainWindow(parent), m_isDarkMode(false)
{
    // 设置窗口图标
    setWindowIcon(QIcon(":/icons/IDE.png"));

    // 创建 QTabWidget 作为中心部件
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // 初始化 TabWidgetManager
    m_tabManager = new TabWidgetManager(m_tabWidget, this);

    // 初始化 Formatter
    m_formatter = new Formatter();

    // 初始化 DebuggerManager
    m_debuggerManager = new DebuggerManager(this);

    // 默认创建一个新文件
    m_tabManager->newFile();

    // 连接 TabManager 的信号，用于更新状态栏和动作状态
    connect(m_tabManager, &TabWidgetManager::currentEditorChanged, this, &My_IDE::on_currentEditorChanged);

    // 创建状态栏
    QStatusBar *statusBar = new QStatusBar;
    setStatusBar(statusBar);

    // 设置调试器 UI
    setupDebuggerUI();

    // 连接 DebuggerManager 的信号到 My_IDE 的槽
    connect(m_debuggerManager, &DebuggerManager::debuggerStarted, this, &My_IDE::on_debuggerStarted);
    connect(m_debuggerManager, &DebuggerManager::debuggerStopped, this, &My_IDE::on_debuggerStopped);
    connect(m_debuggerManager, &DebuggerManager::hitBreakpoint, this, &My_IDE::on_hitBreakpoint);
    connect(m_debuggerManager, &DebuggerManager::currentLineChanged, this, &My_IDE::on_currentDebugLineChanged);


    // 初始化菜单系统
    initMenuSystem();

    // 设置初始字体大小
    fontsize = 20; // 默认字体大小
    m_tabManager->setFontSize(fontsize);

    // 初始状态更新，以防第一个Tab没有触发信号
    on_currentEditorChanged(m_tabManager->currentEditor());

    // 初始禁用调试控制动作
    on_debuggerStopped();
}

void My_IDE::setupDebuggerUI()
{
    m_debugOutputWidget = new QTextEdit(this);
    m_debugOutputWidget->setReadOnly(true);
    m_debugOutputWidget->setFont(QFont("Consolas", 10));
    // 初始浅色模式：背景白色，文字天蓝色
    m_debugOutputWidget->setStyleSheet("background-color: white; color: #87CEEB;");
    QDockWidget *debugDock = new QDockWidget("调试输出", this);
    debugDock->setWidget(m_debugOutputWidget);
    addDockWidget(Qt::BottomDockWidgetArea, debugDock);
    m_debuggerManager->setOutputWidget(m_debugOutputWidget);
}

void My_IDE::initMenuSystem()
{
    // 文件菜单
    file = menuBar()->addMenu("文件");
    file_new = new QAction("新建", this);
    file_open = new QAction("打开", this);
    file_save = new QAction("保存", this);
    file_othersave = new QAction("另存为", this);
    file_closeTab = new QAction("关闭当前文件", this);
    file_exit = new QAction("退出", this);

    // 添加快捷键
    file_new->setShortcut(tr("Ctrl+N"));
    file_open->setShortcut(tr("Ctrl+O"));
    file_save->setShortcut(tr("Ctrl+S"));
    file_othersave->setShortcut(tr("Ctrl+Shift+S"));
    file_closeTab->setShortcut(tr("Ctrl+W"));

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
    edit_formatCode = new QAction("格式化代码", this);
    edit_formatCode->setShortcut(tr("Ctrl+Shift+F"));
    edit->addAction(edit_formatCode);

    // 构建菜单
    build = menuBar()->addMenu("构建");
    build_compile = new QAction("编译", this);
    build_run = new QAction("运行", this);
    build_compileAndRun = new QAction("编译并运行", this);
    build_compile->setShortcut(tr("F9"));
    build_run->setShortcut(tr("F10"));
    build_compileAndRun->setShortcut(tr("F11"));
    build->addActions({build_compile, build_run, build_compileAndRun});

    // 新增：调试菜单
    debug = menuBar()->addMenu("调试");
    debug_start = new QAction("开始调试", this);
    debug_stop = new QAction("停止调试", this);
    debug_continue = new QAction("继续", this);
    debug_stepInto = new QAction("步进", this);
    debug_stepOver = new QAction("步过", this);
    debug_stepOut = new QAction("步出", this);
    debug_toggleBreakpoint = new QAction("切换断点", this);

    debug_start->setShortcut(tr("F5"));
    debug_stop->setShortcut(tr("Shift+F5"));
    debug_continue->setShortcut(tr("F8"));
    debug_stepInto->setShortcut(tr("F7"));
    debug_stepOver->setShortcut(tr("F6"));
    debug_stepOut->setShortcut(tr("Shift+F7"));
    debug_toggleBreakpoint->setShortcut(tr("F12")); // 示例快捷键

    debug->addActions({debug_start, debug_stop});
    debug->addSeparator();
    debug->addActions({debug_continue, debug_stepInto, debug_stepOver, debug_stepOut});
    debug->addSeparator();
    debug->addAction(debug_toggleBreakpoint);

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
    connect(file_new, &QAction::triggered, this, &My_IDE::on_new);
    connect(file_open, &QAction::triggered, this, &My_IDE::on_open);
    connect(file_save, &QAction::triggered, this, &My_IDE::on_save);
    connect(file_othersave, &QAction::triggered, this, &My_IDE::on_othersave);
    connect(file_closeTab, &QAction::triggered, this, &My_IDE::on_closeTab);
    connect(file_exit, &QAction::triggered, this, &My_IDE::on_exit);

    // 编辑操作
    connect(edit_copy, &QAction::triggered, this, &My_IDE::on_copy);
    connect(edit_paste, &QAction::triggered, this, &My_IDE::on_paste);
    connect(edit_cut, &QAction::triggered, this, &My_IDE::on_cut);
    connect(edit_selectAll, &QAction::triggered, this, &My_IDE::on_selectAll);
    connect(edit_findReplace, &QAction::triggered, this, &My_IDE::on_findReplace);
    connect(edit_formatCode, &QAction::triggered, this, &My_IDE::on_formatCode);

    // 构建操作
    connect(build_compile, &QAction::triggered, this, &My_IDE::on_compile);
    connect(build_run, &QAction::triggered, this, &My_IDE::on_run);
    connect(build_compileAndRun, &QAction::triggered, this, &My_IDE::on_compileAndRun);

    // 新增：调试操作
    connect(debug_start, &QAction::triggered, this, &My_IDE::on_debugStart);
    connect(debug_stop, &QAction::triggered, this, &My_IDE::on_debugStop);
    connect(debug_continue, &QAction::triggered, this, &My_IDE::on_debugContinue);
    connect(debug_stepInto, &QAction::triggered, this, &My_IDE::on_debugStepInto);
    connect(debug_stepOver, &QAction::triggered, this, &My_IDE::on_debugStepOver);
    connect(debug_stepOut, &QAction::triggered, this, &My_IDE::on_debugStepOut);
    connect(debug_toggleBreakpoint, &QAction::triggered, this, &My_IDE::on_debugToggleBreakpoint);

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
    QString selectedFilename = QFileDialog::getOpenFileName(this, "打开文件", QDir::currentPath(), "C/C++ Files (*.c *.cpp *.h);;Text Files (*.txt);;All Files (*)");
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
    QMessageBox::information(this, "关于 My_IDE",
                             "My_IDE 是一个基于 Qt 的简单 C/C++ 集成开发环境。\n"
                             "版本: 1.0\n"
                             "作者: [你的名字]");
}

void My_IDE::on_exit()
{
    // TODO: 退出前检查所有文件是否保存
    // 可以遍历所有 tab，询问是否保存未保存的文件
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "退出", "有未保存的文件，确定要退出吗？",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
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
        on_currentEditorChanged(m_tabManager->currentEditor()); // 刷新当前编辑器的显示
        // QMessageBox::information(this, "设置成功", "字体大小已更新。"); // 可以选择不弹窗
    } else {
        // QMessageBox::information(this, "取消", "字体大小设置已取消。"); // 可以选择不弹窗
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

void My_IDE::on_formatCode()
{
    CodeEditor *currentEditor = m_tabManager->currentEditor();
    if (currentEditor) {
        QString originalCode = currentEditor->toPlainText();
        QString formattedCode = m_formatter->formatCppCode(originalCode);
        currentEditor->setPlainText(formattedCode);
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
            int col = calculateVisualColumn(editor);
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
    clearDebugHighlights(); // 当编辑器切换时，清除旧的调试高亮
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
    int tabSize = (spaceWidth > 0) ? qRound(tabStopDistance / spaceWidth) : 4; // 默认4个空格

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

void My_IDE::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();

        if (delta > 0) {
            fontsize = qMin(72, fontsize + 2);
        } else {
            fontsize = qMax(6, fontsize - 2);
        }
        m_tabManager->setFontSize(fontsize);
        on_currentEditorChanged(m_tabManager->currentEditor());
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
    }
}

void My_IDE::on_toggleColorMode()
{
    m_isDarkMode = !m_isDarkMode;
    applyColorMode(m_isDarkMode);
    QString mode = m_isDarkMode ? "深色" : "浅色";
    QMessageBox::information(this, "模式切换", QString("已切换到%1模式").arg(mode));
}

void My_IDE::applyColorMode(bool darkMode)
{
    // 基础色
    QString bgColor, textColor, menuColor, statusColor;
    // 调试输出专用色
    QString debugOutputBg, debugOutputText;

    if (darkMode) {
        // 深色模式
        bgColor = "#2b2b2b";      // 编辑区背景
        textColor = "#a9b7c6";    // 编辑区前景
        menuColor = "#3c3f41";    // 菜单/菜单栏背景
        statusColor = "#3c3f41";  // 状态栏背景
        debugOutputBg = "#2b2b2b";
        debugOutputText = "#87CEEB"; // 深色模式下调试输出文本颜色
    } else {
        // 浅色模式
        bgColor = "white";
        textColor = "black";
        menuColor = "#f0f0f0";
        statusColor = "#e0e0e0";
        debugOutputBg = "white";
        debugOutputText = "#87CEEB"; // 浅色模式下调试输出文本为天蓝色
    }

    // 更新所有 CodeEditor 的样式
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (auto *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i))) {
            editor->setStyleSheet(QString("background-color: %1; color: %2;")
                                      .arg(bgColor, textColor));
            // 若有语法高亮器并且需要基于模式重绘，可在此调用 rehighlight()
            // if (editor->highlighter()) editor->highlighter()->rehighlight();
        }
    }

    // 更新菜单栏与菜单
    menuBar()->setStyleSheet(
        QString(
            "QMenuBar { background-color: %1; color: %2; }"
            "QMenuBar::item:selected { background-color: %3; }"
            "QMenu { background-color: %1; color: %2; }"
            "QMenu::item:selected { background-color: %3; }"
            ).arg(menuColor,
                 textColor,
                 darkMode ? "#505354" : "#d0d0d0")
        );

    // 更新状态栏
    statusBar()->setStyleSheet(
        QString("QStatusBar { background-color: %1; color: %2; }")
            .arg(statusColor, textColor)
        );

    // 更新调试输出（关键：保证颜色按模式固定为上面的两种）
    if (m_debugOutputWidget) {
        m_debugOutputWidget->setStyleSheet(
            QString("background-color: %1; color: %2;")
                .arg(debugOutputBg, debugOutputText)
            );

        // 如果你的输出是通过 HTML（setHtml/setText）写入，且内部含 <font color="...">，
        // 这些内联颜色会覆盖样式表。为保险可清除字符格式或统一强制颜色：
        // QTextCharFormat fmt; fmt.setForeground(QColor(debugOutputText));
        // QTextCursor c = m_debugOutputWidget->textCursor();
        // c.select(QTextCursor::Document); c.mergeCharFormat(fmt);
        // m_debugOutputWidget->setTextCursor(c);
    }
}

My_IDE::~My_IDE()
{
    // m_formatter, m_tabWidget, m_tabManager, m_debuggerManager, m_debugOutputWidget
    // 都会因为有父对象而自动删除，无需手动 delete
}

// 调试槽函数实现
void My_IDE::on_debugStart()
{
    CodeEditor *currentEditor = m_tabManager->currentEditor();
    if (!currentEditor) {
        QMessageBox::warning(this, "调试器", "没有打开的文件。");
        return;
    }

    QString currentFilePath = m_tabManager->getCurrentFilePath();
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "调试器", "请先保存文件再开始调试。");
        return;
    }

    // 确保文件已编译并生成可执行文件
    m_tabManager->compileCurrentFile(); // 编译可能会更新文件，所以需要先编译

    QString executablePath = currentFilePath;
    executablePath.replace(".c", ".exe").replace(".cpp", ".exe"); // 根据编译规则获取可执行文件路径

    if (!QFileInfo(executablePath).exists()) {
        QMessageBox::warning(this, "调试器", "可执行文件不存在，请先编译。");
        return;
    }

    m_debugOutputWidget->clear(); // 清空之前的调试输出
    m_debuggerManager->startDebugging(executablePath);
}

void My_IDE::on_debugStop()
{
    m_debuggerManager->stopDebugging();
}

void My_IDE::on_debugContinue()
{
    m_debuggerManager->continueExecution();
}

void My_IDE::on_debugStepInto()
{
    m_debuggerManager->stepInto();
}

void My_IDE::on_debugStepOver()
{
    m_debuggerManager->stepOver();
}

void My_IDE::on_debugStepOut()
{
    m_debuggerManager->stepOut();
}

void My_IDE::on_debugToggleBreakpoint()
{
    CodeEditor *currentEditor = m_tabManager->currentEditor();
    if (!currentEditor) return;

    QString filePath = m_tabManager->getCurrentFilePath();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "断点", "请先保存文件才能设置断点。");
        return;
    }

    int lineNumber = currentEditor->textCursor().blockNumber() + 1; // 当前光标所在行

    m_debuggerManager->toggleBreakpoint(filePath, lineNumber);

    // TODO: 在编辑器中可视化断点（例如，行号区域显示红点）
    // 这需要 CodeEditor 内部支持行号区域的绘制和管理
    // QMessageBox::information(this, "断点", QString("在 %1 行切换断点").arg(lineNumber));
    statusBar()->showMessage(QString("已在 %1 行切换断点").arg(lineNumber));
}

// DebuggerManager 信号处理槽
void My_IDE::on_debuggerStarted()
{
    statusBar()->showMessage("调试器已启动...");
    // 启用/禁用相关动作
    debug_start->setEnabled(false);
    debug_stop->setEnabled(true);
    debug_continue->setEnabled(true);
    debug_stepInto->setEnabled(true);
    debug_stepOver->setEnabled(true);
    debug_stepOut->setEnabled(true);
    debug_toggleBreakpoint->setEnabled(true); // 调试中也可以切换断点
}

void My_IDE::on_debuggerStopped()
{
    statusBar()->showMessage("调试器已停止。");
    // 启用/禁用相关动作
    debug_start->setEnabled(true);
    debug_stop->setEnabled(false);
    debug_continue->setEnabled(false);
    debug_stepInto->setEnabled(false);
    debug_stepOver->setEnabled(false);
    debug_stepOut->setEnabled(false);
    debug_toggleBreakpoint->setEnabled(true); // 即使没有调试，也可以设置断点
    clearDebugHighlights(); // 清除所有调试高亮
}

void My_IDE::on_hitBreakpoint(const QString &filePath, int lineNumber)
{
    statusBar()->showMessage(QString("命中断点于 %1:%2").arg(filePath).arg(lineNumber));
    highlightDebugLine(filePath, lineNumber); // 高亮当前调试行
}

void My_IDE::on_currentDebugLineChanged(const QString &filePath, int lineNumber)
{
    statusBar()->showMessage(QString("当前执行行于 %1:%2").arg(filePath).arg(lineNumber));
    highlightDebugLine(filePath, lineNumber); // 高亮当前调试行
}

void My_IDE::highlightDebugLine(const QString &filePath, int lineNumber)
{
    clearDebugHighlights(); // 清除之前的调试高亮

    // 找到对应的 CodeEditor
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        // 注意：这里需要 TabWidgetManager 的 m_editorFilePaths 是可访问的
        // 如果 m_editorFilePaths 是 private，需要通过 TabWidgetManager::getFilePath(CodeEditor*) 访问
        if (editor && m_tabManager->m_editorFilePaths.value(editor) == filePath) {
            m_tabWidget->setCurrentIndex(i); // 切换到包含该文件的 Tab

            QTextCharFormat format;
            // 调试行使用浅蓝色背景高亮，文本颜色不变以保持可读性
            format.setBackground(QColor("#aaddff")); // 浅蓝色
            // format.setForeground(Qt::black); // 保持原始文本颜色，或根据模式调整

            QTextCursor cursor = editor->textCursor();
            // 移动到指定行（行号从1开始，文档块从0开始）
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
            cursor.select(QTextCursor::LineUnderCursor); // 选中整行
            cursor.mergeCharFormat(format); // 应用格式

            editor->setTextCursor(cursor); // 更新光标以显示高亮
            editor->ensureCursorVisible(); // 确保高亮行可见
            break;
        }
    }
}

void My_IDE::clearDebugHighlights()
{
    // 遍历所有编辑器，清除所有高亮
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        if (editor) {
            // 创建一个默认格式，背景透明，前景根据当前模式设置
            QTextCharFormat defaultFormat;
            // 根据当前颜色模式设置默认前景色
            if (m_isDarkMode) {
                defaultFormat.setForeground(QColor("#a9b7c6")); // 深色模式的文本颜色
            } else {
                defaultFormat.setForeground(Qt::black); // 浅色模式的文本颜色
            }
            defaultFormat.setBackground(Qt::transparent); // 背景透明

            QTextCursor cursor(editor->document());
            cursor.select(QTextCursor::Document); // 选中整个文档
            cursor.setCharFormat(defaultFormat); // 应用默认格式，清除所有特殊格式
        }
    }
}
// ... 你的 CppHighlighter.cpp 其他代码保持不变 ...

void CppHighlighter::rehighlight()
{
    QSyntaxHighlighter::rehighlight(); // 调用基类的 rehighlight 方法
}

// ... 你的 CppHighlighter.cpp 其他代码保持不变 ...
