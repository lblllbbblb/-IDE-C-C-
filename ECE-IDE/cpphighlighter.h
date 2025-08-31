#ifndef CPPHIGHLIGHTER_H
#define CPPHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression> // Qt5及以上推荐使用QRegularExpression

class CppHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CppHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat operatorFormat; // 新增操作符格式
    QTextCharFormat preprocessorFormat; // 预处理指令格式
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat includeFormat; // #include 格式
    QTextCharFormat numberFormat; // 数字格式
};

#endif // CPPHIGHLIGHTER_H
