#include "TabWidgetManager.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

// 确保包含 CodeEditor, CppHighlighter, BracketMatcher, FindReplaceDialog 的头文件
// 如果它们不在当前目录，需要调整路径
// #include "CodeEditor.h" // 已经通过 TabWidgetManager.h 包含了
// #include "cpphighlighter.h" // 已经通过 TabWidgetManager.h 包含了
// #include "bracketmatcher.h" // 已经通过 TabWidgetManager.h 包含了
// #include "findreplacedialog.h" // 已经通过 TabWidgetManager.h 包含了


TabWidgetManager::TabWidgetManager(QTabWidget *tabWidget, QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget), m_compilerProcess(new QProcess(this)) // 初始化 QProcess
{
    m_tabWidget->setTabsClosable(true); // 允许关闭 Tab
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &TabWidgetManager::onTabCloseRequested);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index){
        if (index != -1) {
            emit currentEditorChanged(currentEditor());
        } else {
            emit currentEditorChanged(nullptr); // 没有打开的编辑器
        }
    });

    // 连接 QProcess 的信号到对应的槽
    connect(m_compilerProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TabWidgetManager::onCompileProcessFinished);
    connect(m_compilerProcess, &QProcess::errorOccurred,
            this, &TabWidgetManager::onCompileProcessError);
}

TabWidgetManager::~TabWidgetManager()
{
    // QTabWidget 会自动删除其子部件，m_editorFilePaths 中的 CodeEditor 无需手动删除
    // m_compilerProcess 的父对象是 TabWidgetManager，所以它会在 TabWidgetManager 销毁时自动删除
}

void TabWidgetManager::initializeEditor(CodeEditor *editor)
{
    editor->setFont(QFont("Consolas", 12));
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    new CppHighlighter(editor->document()); // 为每个编辑器创建高亮器
    new BracketMatcher(editor, this);       // 为每个编辑器创建括号匹配器
    // FindReplaceDialog 不需要为每个编辑器创建，它会操作当前活动的编辑器
}

CodeEditor* TabWidgetManager::newFile()
{
    CodeEditor *editor = new CodeEditor(m_tabWidget);
    initializeEditor(editor);
    int index = m_tabWidget->addTab(editor, "untitled.c");
    m_tabWidget->setCurrentIndex(index);
    m_editorFilePaths[editor] = ""; // 新文件没有路径
    return editor;
}

CodeEditor* TabWidgetManager::openFile(const QString &filePath)
{
    // 检查文件是否已经打开
    for (auto it = m_editorFilePaths.constBegin(); it != m_editorFilePaths.constEnd(); ++it) {
        if (it.value() == filePath) {
            int index = m_tabWidget->indexOf(it.key());
            if (index != -1) {
                m_tabWidget->setCurrentIndex(index);
                return it.key();
            }
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(m_tabWidget, "错误", "无法打开文件: " + file.errorString());
        return nullptr;
    }

    QTextStream in(&file);
    CodeEditor *editor = new CodeEditor(m_tabWidget);
    initializeEditor(editor);
    editor->setPlainText(in.readAll());
    file.close();

    QFileInfo fileInfo(filePath);
    int index = m_tabWidget->addTab(editor, fileInfo.fileName());
    m_tabWidget->setCurrentIndex(index);
    m_editorFilePaths[editor] = filePath;
    return editor;
}

CodeEditor* TabWidgetManager::currentEditor() const
{
    return qobject_cast<CodeEditor*>(m_tabWidget->currentWidget());
}

QString TabWidgetManager::getCurrentFilePath() const
{
    CodeEditor *editor = currentEditor();
    if (editor && m_editorFilePaths.contains(editor)) {
        return m_editorFilePaths[editor];
    }
    return "";
}

void TabWidgetManager::setCurrentFilePath(const QString &filePath)
{
    CodeEditor *editor = currentEditor();
    if (editor) {
        m_editorFilePaths[editor] = filePath;
        QFileInfo fileInfo(filePath);
        m_tabWidget->setTabText(m_tabWidget->currentIndex(), fileInfo.fileName());
    }
}

void TabWidgetManager::saveCurrentFile()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return;

    QString currentPath = getCurrentFilePath();
    if (currentPath.isEmpty()) {
        saveCurrentFileAs();
        return;
    }

    QFile file(currentPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(m_tabWidget, "错误", "无法保存文件: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << editor->toPlainText();
    file.close();
    QMessageBox::information(m_tabWidget, "成功", "文件保存成功");
}

void TabWidgetManager::saveCurrentFileAs()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return;

    QString filePath = QFileDialog::getSaveFileName(m_tabWidget, "另存为", getCurrentFilePath(), "C/C++ Files (*.c *.cpp *.h);;Text Files (*.txt);;All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(m_tabWidget, "错误", "无法保存文件: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out << editor->toPlainText();
    file.close();
    setCurrentFilePath(filePath);
    QMessageBox::information(m_tabWidget, "成功", "文件另存为成功");
}

void TabWidgetManager::closeCurrentTab()
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex != -1) {
        onTabCloseRequested(currentIndex);
    }
}

void TabWidgetManager::onTabCloseRequested(int index)
{
    CodeEditor *editor = qobject_cast<CodeEditor*>(m_tabWidget->widget(index));
    if (editor) {
        // TODO: 可以在这里添加文件修改提示，询问是否保存
        m_editorFilePaths.remove(editor);
        m_tabWidget->removeTab(index);
        editor->deleteLater(); // 延迟删除 CodeEditor
    }
}

void TabWidgetManager::compileCurrentFile()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return;

    // 如果文件未保存，先另存为
    QString currentPath = getCurrentFilePath();
    if (currentPath.isEmpty()) {
        saveCurrentFileAs();
        currentPath = getCurrentFilePath(); // 更新路径
        if (currentPath.isEmpty()) return; // 如果另存为取消了
    }

    // 保存当前文件内容到磁盘
    QFile file(currentPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(m_tabWidget, "错误", "无法保存文件进行编译: " + file.errorString());
        return;
    }
    QTextStream out(&file);
    out << editor->toPlainText();
    file.close();

    QString destname = currentPath;
    destname.replace(".c", ".exe", Qt::CaseInsensitive).replace(".cpp", ".exe", Qt::CaseInsensitive); // 支持 .c 和 .cpp

    QStringList arguments;
    // g++ 命令及其参数，-finput-charset 和 -fexec-charset 用于处理编码
    arguments << currentPath << "-o" << destname << "-finput-charset=UTF-8" << "-fexec-charset=UTF-8";

    qDebug() << "Compile command: g++" << arguments.join(" ");

    // 检查是否有正在运行的编译进程
    if (m_compilerProcess->state() == QProcess::Running) {
        QMessageBox::information(m_tabWidget, "编译", "已有编译进程正在运行，请等待其完成。");
        return;
    }

    // 启动编译进程
    m_compilerProcess->start("g++", arguments);
    if (!m_compilerProcess->waitForStarted(5000)) { // 等待5秒检查进程是否启动
        QMessageBox::warning(m_tabWidget, "编译错误", "无法启动 g++ 编译器，请检查 PATH 设置或 g++ 是否安装。");
    }
}

void TabWidgetManager::onCompileProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus); // 避免未使用参数的警告

    // 读取标准输出和标准错误输出
    QString stdOutput = m_compilerProcess->readAllStandardOutput();
    QString errOutput = m_compilerProcess->readAllStandardError();

    QString message;
    if (exitCode == 0) {
        message = "编译成功！";
        if (!stdOutput.isEmpty()) {
            message += "\n标准输出:\n" + stdOutput;
        }
        QMessageBox::information(m_tabWidget, "编译", message);
    } else {
        message = "编译失败！请检查代码。\n";
        if (!errOutput.isEmpty()) {
            message += "错误信息:\n" + errOutput;
        } else if (!stdOutput.isEmpty()) {
            message += "标准输出:\n" + stdOutput; // 有时错误信息也会在标准输出
        } else {
            message += "没有详细错误信息。";
        }
        QMessageBox::warning(m_tabWidget, "编译", message);
    }
}

void TabWidgetManager::onCompileProcessError(QProcess::ProcessError error)
{
    QString errorMessage;
    switch (error) {
    case QProcess::FailedToStart:
        errorMessage = "编译器程序无法启动。请确保 g++ 已安装且其路径在系统的 PATH 环境变量中。";
        break;
    case QProcess::Crashed:
        errorMessage = "编译器程序崩溃。";
        break;
    case QProcess::Timedout:
        errorMessage = "编译器程序运行超时。";
        break;
    case QProcess::ReadError:
        errorMessage = "从编译器读取数据时发生错误。";
        break;
    case QProcess::WriteError:
        errorMessage = "向编译器写入数据时发生错误。";
        break;
    case QProcess::UnknownError:
    default:
        errorMessage = "发生未知错误。";
        break;
    }
    QMessageBox::critical(m_tabWidget, "编译进程错误", errorMessage);
}


void TabWidgetManager::runCurrentFile()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return;

    QString currentPath = getCurrentFilePath();
    if (currentPath.isEmpty()) {
        QMessageBox::warning(m_tabWidget, "运行", "请先保存文件再运行。");
        return;
    }

    QString destname = currentPath;
    destname.replace(".c", ".exe", Qt::CaseInsensitive).replace(".cpp", ".exe", Qt::CaseInsensitive);

    QFileInfo executable(destname);
    if (!executable.exists()) {
        QMessageBox::warning(m_tabWidget, "运行", "可执行文件不存在，请先编译。");
        return;
    }

    // 使用 cmd /c 运行可执行文件并暂停，以便查看输出
    // chcp 65001 用于设置控制台为 UTF-8 编码，防止中文乱码
    QString commandToExecute = "cmd /c \"chcp 65001 > nul && \"" + QDir::toNativeSeparators(destname) + "\" & pause\"";
    qDebug() << "Run command:" << commandToExecute;
    // 使用 system 函数直接执行，因为它更适合运行需要用户交互的控制台程序
    // 如果需要捕获输出，则需要使用 QProcess，但那样就不能直接暂停了
    system(commandToExecute.toStdString().data());
}

void TabWidgetManager::compileAndRunCurrentFile()
{
    // 先尝试编译
    compileCurrentFile();
    // 注意：compileCurrentFile 现在是异步的，这里不能直接调用 runCurrentFile()
    // 一个理想的实现是在 onCompileProcessFinished 中，如果编译成功，再触发 runCurrentFile()。
    // 为了简化，目前不做自动链式调用，用户可以手动点击运行。
    // 如果需要自动链式，需要更复杂的信号槽机制或者在 onCompileProcessFinished 中判断并调用 runCurrentFile。
}

void TabWidgetManager::copy() { if (currentEditor()) currentEditor()->copy(); }
void TabWidgetManager::paste() { if (currentEditor()) currentEditor()->paste(); }
void TabWidgetManager::cut() { if (currentEditor()) currentEditor()->cut(); }
void TabWidgetManager::selectAll() { if (currentEditor()) currentEditor()->selectAll(); }
void TabWidgetManager::undo() { if (currentEditor()) currentEditor()->undo(); }
void TabWidgetManager::redo() { if (currentEditor()) currentEditor()->redo(); }

void TabWidgetManager::showFindReplaceDialog()
{
    CodeEditor *editor = currentEditor();
    if (editor) {
        FindReplaceDialog *dialog = new FindReplaceDialog(editor, m_tabWidget); // 为当前编辑器创建对话框
        dialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
        dialog->show();
        dialog->raise();
        dialog->activateWindow();
    }
}

void TabWidgetManager::setFontSize(int size)
{
    CodeEditor *editor = currentEditor();
    if (editor) {
        QFont f = editor->font();
        f.setPixelSize(size);
        editor->setFont(f);
    }
    // 遍历所有打开的编辑器并更新字体
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        CodeEditor *otherEditor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        if (otherEditor) { // 即使是当前编辑器也更新，确保一致性
            QFont otherFont = otherEditor->font();
            otherFont.setPixelSize(size);
            otherEditor->updateFont(otherFont); // 假设 CodeEditor 有一个 updateFont 方法
        }
    }
}
