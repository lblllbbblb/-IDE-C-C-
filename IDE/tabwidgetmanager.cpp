#include "TabWidgetManager.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

TabWidgetManager::TabWidgetManager(QTabWidget *tabWidget, QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget)
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
}

TabWidgetManager::~TabWidgetManager()
{
    // QTabWidget 会自动删除其子部件，m_editorFilePaths 中的 CodeEditor 无需手动删除
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
    destname.replace(".c", ".exe").replace(".cpp", ".exe"); // 支持 .c 和 .cpp

    QString command = "g++ -o \"" + destname + "\" \"" + currentPath + "\" -finput-charset=UTF-8 -fexec-charset=UTF-8";
    qDebug() << "Compile command:" << command;
    int result = system(command.toStdString().data());

    if (result == 0) {
        QMessageBox::information(m_tabWidget, "编译", "编译成功！");
    } else {
        QMessageBox::warning(m_tabWidget, "编译", "编译失败！请检查代码。");
    }
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
    destname.replace(".c", ".exe").replace(".cpp", ".exe");

    QFileInfo executable(destname);
    if (!executable.exists()) {
        QMessageBox::warning(m_tabWidget, "运行", "可执行文件不存在，请先编译。");
        return;
    }

    QString commandToExecute = "cmd /c \"chcp 65001 > nul && \"" + destname + "\" & pause\"";
    qDebug() << "Run command:" << commandToExecute;
    system(commandToExecute.toStdString().data());
}

void TabWidgetManager::compileAndRunCurrentFile()
{
    compileCurrentFile();
    runCurrentFile();
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
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        CodeEditor *otherEditor = qobject_cast<CodeEditor*>(m_tabWidget->widget(i));
        if (otherEditor && otherEditor != editor) {
            QFont otherFont = otherEditor->font();
            otherFont.setPixelSize(size);
            otherEditor->updateFont(otherFont);
        }
    }
}
