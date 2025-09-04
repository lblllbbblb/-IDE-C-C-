#include "findreplacedialog.h"
#include <QMessageBox>
#include <QDebug> // 用于调试输出

FindReplaceDialog::FindReplaceDialog(QTextEdit *textEdit, QWidget *parent)
    : QDialog(parent)
    , m_textEdit(textEdit) // 初始化 QTextEdit 指针
{
    // 设置对话框标题
    setWindowTitle("查找与替换");

    // —— 创建控件 ——
    findLabel = new QLabel("查找内容:", this);
    findLineEdit = new QLineEdit(this);

    replaceLabel = new QLabel("替换为:", this);
    replaceLineEdit = new QLineEdit(this);

    caseSensitiveCheckBox = new QCheckBox("区分大小写", this);
    wholeWordsCheckBox = new QCheckBox("全字匹配", this);

    findNextButton = new QPushButton("查找下一个", this);
    findPreviousButton = new QPushButton("查找上一个", this);
    replaceButton = new QPushButton("替换", this);
    replaceAllButton = new QPushButton("替换所有", this);
    closeButton = new QPushButton("关闭", this);

    // —— 布局管理 ——

    // 查找内容行
    QHBoxLayout *findLayout = new QHBoxLayout;
    findLayout->addWidget(findLabel);
    findLayout->addWidget(findLineEdit);

    // 替换为行
    QHBoxLayout *replaceLayout = new QHBoxLayout;
    replaceLayout->addWidget(replaceLabel);
    replaceLayout->addWidget(replaceLineEdit);

    // 选项复选框行
    QHBoxLayout *optionLayout = new QHBoxLayout;
    optionLayout->addWidget(caseSensitiveCheckBox);
    optionLayout->addWidget(wholeWordsCheckBox);
    optionLayout->addStretch(); // 添加伸缩空间，使复选框靠左

    // 按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(findPreviousButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    buttonLayout->addWidget(closeButton);

    // 主垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(findLayout);
    mainLayout->addLayout(replaceLayout);
    mainLayout->addLayout(optionLayout);
    mainLayout->addLayout(buttonLayout);

    // —— 连接信号与槽 ——
    connect(findNextButton, &QPushButton::clicked, this, &FindReplaceDialog::findNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &FindReplaceDialog::findPrevious);
    connect(replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::replace);
    connect(replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::replaceAll);
    connect(closeButton, &QPushButton::clicked, this, &FindReplaceDialog::close);

    // 当查找输入框内容改变时，清空之前的查找结果（取消选择）
    connect(findLineEdit, &QLineEdit::textChanged, [this]() {
        if (m_textEdit) {
            QTextCursor cursor = m_textEdit->textCursor();
            cursor.clearSelection();
            m_textEdit->setTextCursor(cursor);
        }
    });

    // 默认禁用替换按钮，直到有文本被找到
    replaceButton->setEnabled(false);
    replaceAllButton->setEnabled(false);
}

FindReplaceDialog::~FindReplaceDialog()
{
    // 所有在堆上创建的控件都会被父对象自动删除，
    // 所以这里不需要手动删除各个控件
}

// 辅助函数：执行实际的文本查找
bool FindReplaceDialog::findText(const QString &text, QTextDocument::FindFlags flags)
{
    if (!m_textEdit || text.isEmpty()) {
        return false;
    }

    // 清除之前的选择，从当前光标位置开始查找
    QTextCursor cursor = m_textEdit->textCursor();
    // cursor.clearSelection(); // 查找下一个时不需要清除，否则会从头开始
    // m_textEdit->setTextCursor(cursor); // 不需要设置，find() 会操作当前光标

    // 执行查找操作
    bool found = m_textEdit->find(text, flags);

    // 根据查找结果启用或禁用替换按钮
    replaceButton->setEnabled(found);
    replaceAllButton->setEnabled(!text.isEmpty()); // 只要查找内容不为空，替换所有就可用

    return found;
}


// 查找下一个匹配项
void FindReplaceDialog::findNext()
{
    QString searchString = findLineEdit->text();
    if (searchString.isEmpty()) {
        QMessageBox::information(this, "查找", "请输入要查找的内容。");
        return;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively; // 区分大小写
    }
    if (wholeWordsCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;     // 全字匹配
    }

    if (!findText(searchString, flags)) {
        QMessageBox::information(this, "查找", QString("未找到 '%1'。").arg(searchString));
        // 将光标重置到文档开头，以便下一次查找从头开始
        m_textEdit->moveCursor(QTextCursor::Start);
        if(!findText(searchString, flags)) { // 再次查找，如果还是没找到，就说明真的没有了
            QMessageBox::information(this, "查找", QString("文档中没有找到 '%1'。").arg(searchString));
        }
    }
}

// 查找上一个匹配项
void FindReplaceDialog::findPrevious()
{
    QString searchString = findLineEdit->text();
    if (searchString.isEmpty()) {
        QMessageBox::information(this, "查找", "请输入要查找的内容。");
        return;
    }

    QTextDocument::FindFlags flags = QTextDocument::FindBackward; // 向后查找
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWordsCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    if (!findText(searchString, flags)) {
        QMessageBox::information(this, "查找", QString("未找到 '%1'。").arg(searchString));
        // 将光标重置到文档末尾，以便下一次向后查找从末尾开始
        m_textEdit->moveCursor(QTextCursor::End);
        if(!findText(searchString, flags)) {
            QMessageBox::information(this, "查找", QString("文档中没有找到 '%1'。").arg(searchString));
        }
    }
}


// 替换当前匹配项
void FindReplaceDialog::replace()
{
    if (!m_textEdit) return;

    QString replaceString = replaceLineEdit->text();
    QTextCursor cursor = m_textEdit->textCursor();

    if (cursor.hasSelection()) { // 如果当前有选中的文本
        cursor.insertText(replaceString); // 替换选中的文本
        // 替换后，尝试查找下一个，以便用户继续替换
        findNext();
    } else {
        // 如果没有选中任何文本，则可能是用户直接点击替换而未先查找
        // 此时应先尝试查找下一个，如果找到则替换
        if (findText(findLineEdit->text(), QTextDocument::FindFlags())) {
            QTextCursor newCursor = m_textEdit->textCursor();
            if (newCursor.hasSelection()) {
                newCursor.insertText(replaceString);
                // 替换后，再次查找下一个
                findNext();
            }
        } else {
            QMessageBox::information(this, "替换", "没有找到要替换的内容。");
        }
    }
}

// 替换所有匹配项
void FindReplaceDialog::replaceAll()
{
    if (!m_textEdit) return;

    QString searchString = findLineEdit->text();
    QString replaceString = replaceLineEdit->text();

    if (searchString.isEmpty()) {
        QMessageBox::information(this, "替换所有", "请输入要查找的内容。");
        return;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWordsCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    QTextCursor originalCursor = m_textEdit->textCursor(); // 记录原始光标位置
    m_textEdit->moveCursor(QTextCursor::Start); // 从文档开头开始查找和替换

    int count = 0;
    while (m_textEdit->find(searchString, flags)) {
        QTextCursor cursor = m_textEdit->textCursor();
        cursor.insertText(replaceString);
        count++;
    }

    m_textEdit->setTextCursor(originalCursor); // 恢复原始光标位置
    QMessageBox::information(this, "替换所有", QString("已替换 %1 处 '%2'。").arg(count).arg(searchString));
}
