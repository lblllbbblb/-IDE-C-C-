#ifndef BRACKETMATCHER_H
#define BRACKETMATCHER_H

#include <QObject>
#include <QKeyEvent>
#include <QTextDocument>
#include <QTextBlock>
#include "CodeEditor.h"

class BracketMatcher : public QObject
{
    Q_OBJECT

public:
    explicit BracketMatcher(CodeEditor *textEdit, QObject *parent = nullptr);
    ~BracketMatcher();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    CodeEditor *m_textEdit; // 指向被监控的CodeEditor控件

    // 辅助函数：判断光标位置是否在字符串或注释内部
    bool isInStringOrCommentOptimized(const QTextCursor& cursor);

    // 辅助函数：删除一个字符，向前或向后
    void deleteCharacter(QTextCursor& cursor, bool forward);

    // 辅助函数：获取当前行文本
    QString getCurrentLineText(const QTextCursor& cursor);

    // 辅助函数：获取当前行的缩进字符串
    QString getCurrentIndentation(const QTextCursor& cursor);

    // 新增：检查前一个字符是否是转义字符（用于引号处理）
    bool isPreviousCharEscape(const QTextCursor& cursor);
};

#endif // BRACKETMATCHER_H
#ifndef BRACKETMATCHER_H
#endif // BRACKETMATCHER_H
