#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "CodeEditor.h" // 需要访问 CodeEditor 来执行查找和替换操作

class FindReplaceDialog : public QDialog
{
    Q_OBJECT // 启用 Qt 的元对象系统

public:
    // 构造函数，需要一个父窗口和一个 CodeEditor 指针，以便操作文本
    explicit FindReplaceDialog(CodeEditor *textEdit, QWidget *parent = nullptr);
    ~FindReplaceDialog(); // 析构函数

private slots:
    void findNext();        // 查找下一个匹配项
    void replace();         // 替换当前匹配项
    void replaceAll();      // 替换所有匹配项
    void findPrevious();    // 查找上一个匹配项（为了更完善的查找功能）

private:
    CodeEditor *m_textEdit;  // 指向主窗口的 CodeEditor 控件

    QLabel *findLabel;      // “查找内容”标签
    QLineEdit *findLineEdit;// “查找内容”输入框
    QLabel *replaceLabel;   // “替换为”标签
    QLineEdit *replaceLineEdit;// “替换为”输入框

    QCheckBox *caseSensitiveCheckBox; // “区分大小写”复选框
    QCheckBox *wholeWordsCheckBox;    // “全字匹配”复选框

    QPushButton *findNextButton;      // “查找下一个”按钮
    QPushButton *findPreviousButton;  // “查找上一个”按钮
    QPushButton *replaceButton;       // “替换”按钮
    QPushButton *replaceAllButton;    // “替换所有”按钮
    QPushButton *closeButton;         // “关闭”按钮

    // 辅助函数，用于执行实际的文本查找
    bool findText(const QString &text, QTextDocument::FindFlags flags);
};

#endif // FINDREPLACEDIALOG_H
