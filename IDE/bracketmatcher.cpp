#include "bracketmatcher.h"
#include <QTextCursor>
#include <QDebug>
#include <QRegularExpression>

BracketMatcher::BracketMatcher(CodeEditor *textEdit, QObject *parent)
    : QObject(parent), m_textEdit(textEdit)
{
    if (m_textEdit) {
        m_textEdit->installEventFilter(this);
        m_textEdit->setFocusPolicy(Qt::StrongFocus);
        qDebug() << "BracketMatcher 已安装事件过滤器到 CodeEditor。";
    } else {
        qDebug() << "警告: BracketMatcher 收到空的 CodeEditor 指针。";
    }
}

BracketMatcher::~BracketMatcher()
{
    if (m_textEdit) {
        m_textEdit->removeEventFilter(this);
        qDebug() << "BracketMatcher 已卸载事件过滤器。";
    }
}

// 判断光标是否在字符串或注释内部
bool BracketMatcher::isInStringOrCommentOptimized(const QTextCursor& cursor)
{
    if (!m_textEdit || !m_textEdit->document()) {
        return false;
    }

    QTextBlock currentBlock = cursor.block();
    QString blockText = currentBlock.text();
    int cursorInBlockPos = cursor.positionInBlock();

    QString textUntilCursor = blockText.left(cursorInBlockPos);

    bool inSingleQuote = false;
    bool inDoubleQuote = false;

    for (int i = 0; i < textUntilCursor.length(); ++i) {
        QChar currentChar = textUntilCursor.at(i);
        if (currentChar == '\\') { // 跳过转义字符
            i++;
            continue;
        }

        if (inSingleQuote) {
            if (currentChar == '\'') inSingleQuote = false;
        } else if (inDoubleQuote) {
            if (currentChar == '"') inDoubleQuote = false;
        } else {
            if (currentChar == '\'') inSingleQuote = true;
            else if (currentChar == '"') inDoubleQuote = true;
        }
    }
    return inSingleQuote || inDoubleQuote;
}

// 检查前一个字符是否是转义字符
bool BracketMatcher::isPreviousCharEscape(const QTextCursor& cursor)
{
    int pos = cursor.position() - 1;
    if (pos < 0) return false;
    return m_textEdit->document()->characterAt(pos) == '\\';
}

// 删除字符功能
void BracketMatcher::deleteCharacter(QTextCursor& cursor, bool forward)
{
    if (forward) cursor.deleteChar();
    else cursor.deletePreviousChar();
}

// 获取当前行文本
QString BracketMatcher::getCurrentLineText(const QTextCursor& cursor)
{
    return cursor.block().text();
}

// 获取当前行的缩进字符串
QString BracketMatcher::getCurrentIndentation(const QTextCursor& cursor)
{
    QString lineText = getCurrentLineText(cursor);
    QRegularExpression re("^[ \\t]*");
    QRegularExpressionMatch match = re.match(lineText);
    return match.captured(0);
}

// 事件过滤器：核心逻辑实现
bool BracketMatcher::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_textEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        QTextCursor cursor = m_textEdit->textCursor();

        // 多重选择时不处理
        if (cursor.hasSelection()) {
            return QObject::eventFilter(obj, event);
        }

        QString typedText = keyEvent->text();
        if (typedText.isEmpty()) {
            return QObject::eventFilter(obj, event);
        }
        QChar typedChar = typedText.at(0);

        // 处理括号自动补齐（保留原有功能）
        if ((typedChar == '(' || typedChar == '{' || typedChar == '[') &&
            !isInStringOrCommentOptimized(cursor)) {
            QChar closingChar;
            if (typedChar == '(') closingChar = ')';
            else if (typedChar == '{') closingChar = '}';
            else closingChar = ']';

            cursor.insertText(typedChar);
            cursor.insertText(closingChar);
            cursor.movePosition(QTextCursor::PreviousCharacter);
            m_textEdit->setTextCursor(cursor);
            return true;
        }

        // 处理引号自动补齐（保留原有功能）
        else if ((typedChar == '\'' || typedChar == '"') &&
                 !isPreviousCharEscape(cursor)) {
            bool inSameQuote = false;
            if (typedChar == '\'') {
                inSameQuote = isInStringOrCommentOptimized(cursor) &&
                              cursor.block().text().left(cursor.positionInBlock()).count('\'') % 2 == 1;
            } else {
                inSameQuote = isInStringOrCommentOptimized(cursor) &&
                              cursor.block().text().left(cursor.positionInBlock()).count('"') % 2 == 1;
            }

            if (!inSameQuote) {
                cursor.insertText(typedChar);
                cursor.insertText(typedChar);
                cursor.movePosition(QTextCursor::PreviousCharacter);
                m_textEdit->setTextCursor(cursor);
                return true;
            }
        }

        // 处理闭括号和引号的自动跳过（保留原有功能）
        else if ((typedChar == ')' || typedChar == '}' || typedChar == ']' ||
                  typedChar == '\'' || typedChar == '"')) {
            int currentPos = cursor.position();
            QTextDocument *doc = m_textEdit->document();
            if (currentPos < doc->characterCount() - 1) {
                QChar charAfterCursor = doc->characterAt(currentPos);
                if (charAfterCursor == typedChar) {
                    if ((typedChar == '\'' || typedChar == '"') &&
                        currentPos > 0 && doc->characterAt(currentPos - 1) == '\\') {
                        return QObject::eventFilter(obj, event);
                    }

                    cursor.movePosition(QTextCursor::NextCharacter);
                    m_textEdit->setTextCursor(cursor);
                    return true;
                }
            }
        }

        // 处理退格键删除匹配对（保留原有功能）
        else if (keyEvent->key() == Qt::Key_Backspace) {
            int currentPos = cursor.position();
            if (currentPos > 0 && currentPos < m_textEdit->document()->characterCount() - 1) {
                QChar charBefore = m_textEdit->document()->characterAt(currentPos - 1);
                QChar charAfter = m_textEdit->document()->characterAt(currentPos);

                bool isMatchingBracket = (charBefore == '(' && charAfter == ')') ||
                                         (charBefore == '{' && charAfter == '}') ||
                                         (charBefore == '[' && charAfter == ']');

                bool isMatchingQuote = (charBefore == '\'' && charAfter == '\'') ||
                                       (charBefore == '"' && charAfter == '"');

                if (isMatchingQuote && currentPos > 1 &&
                    m_textEdit->document()->characterAt(currentPos - 2) == '\\') {
                    isMatchingQuote = false;
                }

                if (isMatchingBracket || isMatchingQuote) {
                    cursor.deleteChar();
                    cursor.deletePreviousChar();
                    m_textEdit->setTextCursor(cursor);
                    return true;
                }
            }
        }

        // 核心修改：处理{后回车，实现{和}对齐且中间空一行
        else if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) &&
                 !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {

            int cursorPos = cursor.position();
            // 检查光标前是否是{，且后面是自动补全的}
            if (cursorPos > 0 && cursorPos < m_textEdit->document()->characterCount() - 1) {
                QChar prevChar = m_textEdit->document()->characterAt(cursorPos - 1);
                QChar nextChar = m_textEdit->document()->characterAt(cursorPos);

                // 仅处理{和}之间的回车
                if (prevChar == '{' && nextChar == '}') {
                    // 获取当前行的缩进（用于对齐{和}）
                    QString currentIndent = getCurrentIndentation(cursor);
                    // 确定缩进风格（制表符或空格）
                    QString indentStep = currentIndent.contains('\t') ? "\t" : "    ";

                    // 开始编辑块（确保操作原子性）
                    cursor.beginEditBlock();

                    // 1. 删除自动补全的}（后续重新插入）
                    cursor.deleteChar();

                    // 2. 在{后插入换行
                    cursor.insertText("\n");

                    // 3. 插入中间空行的缩进（当前缩进+一级）
                    cursor.insertText(currentIndent + indentStep);

                    // 4. 插入第二行换行（用于放置}）
                    cursor.insertText("\n");

                    // 5. 插入}并与{对齐（使用当前缩进）
                    cursor.insertText(currentIndent + "}");

                    // 6. 将光标移动到中间空行的缩进后
                    cursor.movePosition(QTextCursor::Up); // 上移到中间行
                    cursor.movePosition(QTextCursor::EndOfLine); // 移动到行尾

                    // 结束编辑块
                    cursor.endEditBlock();

                    m_textEdit->setTextCursor(cursor);
                    qDebug() << "在{和}之间回车，已对齐并添加空行";
                    return true;
                }
            }

            // 普通回车：保持当前缩进（保留原有功能）
            QString currentIndent = getCurrentIndentation(cursor);
            cursor.insertText("\n" + currentIndent);
            m_textEdit->setTextCursor(cursor);
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
