#ifndef TABWIDGETMANAGER_H
#define TABWIDGETMANAGER_H

#include <QTabWidget>
#include <QMap>
#include "CodeEditor.h"
#include "cpphighlighter.h"
#include "bracketmatcher.h"
#include "findreplacedialog.h" // 可能需要查找替换对话框的引用

class TabWidgetManager : public QObject
{
    Q_OBJECT

public:
    explicit TabWidgetManager(QTabWidget *tabWidget, QObject *parent = nullptr);
    ~TabWidgetManager();

    CodeEditor* newFile(); // 创建一个新的 CodeEditor 并添加到 Tab
    CodeEditor* openFile(const QString &filePath); // 打开一个文件并添加到 Tab
    CodeEditor* currentEditor() const; // 获取当前活动的 CodeEditor

    QString getCurrentFilePath() const; // 获取当前文件的路径
    void setCurrentFilePath(const QString &filePath); // 设置当前文件的路径

    void saveCurrentFile(); // 保存当前文件
    void saveCurrentFileAs(); // 另存为当前文件
    void closeCurrentTab(); // 关闭当前 Tab

    // 编译运行相关，需要传递给当前的 CodeEditor
    void compileCurrentFile();
    void runCurrentFile();
    void compileAndRunCurrentFile();

    // 编辑操作
    void copy();
    void paste();
    void cut();
    void selectAll();
    void undo();
    void redo();
    void showFindReplaceDialog();
    void setFontSize(int size);

signals:
    void currentEditorChanged(CodeEditor *editor); // 当当前编辑器改变时发出信号

private slots:
    void onTabCloseRequested(int index); // 处理 Tab 关闭请求

private:
    QTabWidget *m_tabWidget;
    QMap<CodeEditor*, QString> m_editorFilePaths; // 存储 CodeEditor 及其对应的文件路径

    // 辅助函数，用于初始化新的 CodeEditor
    void initializeEditor(CodeEditor *editor);
};

#endif // TABWIDGETMANAGER_H
