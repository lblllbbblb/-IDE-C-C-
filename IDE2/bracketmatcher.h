#ifndef BRACKETMATCHER_H
#define BRACKETMATCHER_H

#include <QObject>
#include <QTextEdit>
#include <QKeyEvent>
#include <QTextDocument> // 用于文档操作
#include <QTextBlock>    // 用于块（行）操作

class BracketMatcher : public QObject
{
    Q_OBJECT

public:
    explicit BracketMatcher(QTextEdit *textEdit, QObject *parent = nullptr);
    ~BracketMatcher();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QTextEdit *m_textEdit; // 指向被监控的 QTextEdit 控件

    // 辅助函数：判断光标位置是否在字符串或注释内部（优化版本）
    bool isInStringOrCommentOptimized(const QTextCursor& cursor);

    // 辅助函数：删除一个字符，向前或向后
    void deleteCharacter(QTextCursor& cursor, bool forward);

    // 辅助函数：获取当前行文本
    QString getCurrentLineText(const QTextCursor& cursor);
};

#endif // BRACKETMATCHER_H
