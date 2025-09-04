#include "bracketmatcher.h"
#include <QTextCursor>
#include <QDebug> // 用于调试输出

BracketMatcher::BracketMatcher(QTextEdit *textEdit, QObject *parent)
    : QObject(parent),
    m_textEdit(textEdit)
{
    if (m_textEdit) {
        m_textEdit->installEventFilter(this);
        // 确保 QTextEdit 接受键盘事件
        m_textEdit->setFocusPolicy(Qt::StrongFocus);
        qDebug() << "BracketMatcher 已安装事件过滤器到 QTextEdit。";
    } else {
        qDebug() << "警告: BracketMatcher 收到空的 QTextEdit 指针。";
    }
}

BracketMatcher::~BracketMatcher()
{
    if (m_textEdit) {
        m_textEdit->removeEventFilter(this);
        qDebug() << "BracketMatcher 已卸载事件过滤器。";
    }
}

// 辅助函数：判断光标位置是否在字符串或注释内部（优化版本）
// 这个版本尝试只分析当前光标所在块（行）的文本，并结合 QSyntaxHighlighter 的信息（如果可用）
// 然而，直接从 QSyntaxHighlighter 获取精确的当前位置的语义信息比较复杂，
// 简单起见，我们还是在当前行内进行一个轻量级的判断。
bool BracketMatcher::isInStringOrCommentOptimized(const QTextCursor& cursor)
{
    if (!m_textEdit || !m_textEdit->document()) {
        return false;
    }

    QTextBlock currentBlock = cursor.block();
    QString blockText = currentBlock.text();
    int cursorInBlockPos = cursor.positionInBlock();

    // 只检查到光标位置之前的文本
    QString textUntilCursor = blockText.left(cursorInBlockPos);

    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    // 不检查行注释和块注释，因为 QSyntaxHighlighter 已经处理了高亮，
    // 并且在文本输入时实时判断这些状态容易出错且开销大。
    // 如果需要更精确的注释判断，需要与语法高亮器深度集成。
    // 考虑到性能和复杂度，此处仅处理字符串。
    // 自动匹配括号通常不应在字符串内部发生。

    for (int i = 0; i < textUntilCursor.length(); ++i) {
        QChar currentChar = textUntilCursor.at(i);
        if (currentChar == '\\') { // 遇到转义字符，跳过下一个字符
            i++;
            continue;
        }

        if (inSingleQuote) {
            if (currentChar == '\'') {
                inSingleQuote = false;
            }
        } else if (inDoubleQuote) {
            if (currentChar == '"') {
                inDoubleQuote = false;
            }
        } else { // 不在任何引号内部
            if (currentChar == '\'') {
                inSingleQuote = true;
            } else if (currentChar == '"') {
                inDoubleQuote = true;
            }
        }
    }
    return inSingleQuote || inDoubleQuote;
}


// 辅助函数：删除一个字符
void BracketMatcher::deleteCharacter(QTextCursor& cursor, bool forward)
{
    if (forward) {
        // 删除光标后面的字符
        cursor.deleteChar();
    } else {
        // 删除光标前面的字符
        cursor.deletePreviousChar();
    }
}

// 事件过滤器实现
bool BracketMatcher::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_textEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        QTextCursor cursor = m_textEdit->textCursor();

        // 避免在多重选择或非单一光标状态下进行自动匹配
        if (cursor.hasSelection()) {
            qDebug() << "有选中区域，跳过括号匹配。";
            return QObject::eventFilter(obj, event);
        }

        // 获取当前按下的文本，确保它是单个字符
        QString typedText = keyEvent->text();
        if (typedText.isEmpty()) {
            qDebug() << "按键文本为空，跳过括号匹配。";
            return QObject::eventFilter(obj, event);
        }
        QChar typedChar = typedText.at(0);

        // 优化：在字符串内部不进行自动匹配
        if (isInStringOrCommentOptimized(cursor)) {
            qDebug() << "在字符串内部，跳过括号匹配。";
            return QObject::eventFilter(obj, event);
        }

        // 检查是否是开括号
        if (typedChar == '(') {
            cursor.insertText("()");
            cursor.movePosition(QTextCursor::PreviousCharacter); // 将光标移动到括号中间
            m_textEdit->setTextCursor(cursor);
            qDebug() << "插入 ()";
            return true; // 事件已处理
        }
        else if (typedChar == '{') {
            cursor.insertText("{}");
            cursor.movePosition(QTextCursor::PreviousCharacter);
            m_textEdit->setTextCursor(cursor);
            qDebug() << "插入 {}";
            return true;
        }
        else if (typedChar == '[') {
            cursor.insertText("[]");
            cursor.movePosition(QTextCursor::PreviousCharacter);
            m_textEdit->setTextCursor(cursor);
            qDebug() << "插入 []";
            return true;
        }
        // 当输入闭括号时，如果光标后面紧跟着的是匹配的闭括号，则直接跳过
        else if (typedChar == ')' || typedChar == '}' || typedChar == ']') {
            int currentPos = cursor.position();
            QTextDocument *doc = m_textEdit->document();
            if (currentPos < doc->characterCount() - 1) { // 确保不是在文档末尾（QTextDocument::characterCount 包含文档结束符）
                QChar charAfterCursor = doc->characterAt(currentPos);
                if ((typedChar == ')' && charAfterCursor == ')') ||
                    (typedChar == '}' && charAfterCursor == '}') ||
                    (typedChar == ']' && charAfterCursor == ']'))
                {
                    cursor.movePosition(QTextCursor::NextCharacter); // 跳过已有的闭括号
                    m_textEdit->setTextCursor(cursor);
                    qDebug() << "跳过匹配的闭括号。";
                    return true; // 事件已处理
                }
            }
        }
        // 处理 Backspace 删除匹配的括号
        else if (keyEvent->key() == Qt::Key_Backspace) {
            int currentPos = cursor.position();
            if (currentPos > 0 && currentPos < m_textEdit->document()->characterCount() - 1) {
                QChar charBefore = m_textEdit->document()->characterAt(currentPos - 1);
                QChar charAfter = m_textEdit->document()->characterAt(currentPos);

                // 检查是否是匹配的括号对
                if ((charBefore == '(' && charAfter == ')') ||
                    (charBefore == '{' && charAfter == '}') ||
                    (charBefore == '[' && charAfter == ']'))
                {
                    // 先删除后面的闭括号
                    cursor.deleteChar();
                    // 再删除前面的开括号 (光标会自动回到原开括号前)
                    cursor.deletePreviousChar();
                    m_textEdit->setTextCursor(cursor);
                    qDebug() << "Backspace 删除匹配的括号对。";
                    return true; // 事件已处理
                }
            }
        }
    }
    // 对于其他事件或不处理的按键，调用基类的事件过滤器，让其正常处理
    return QObject::eventFilter(obj, event);
}
