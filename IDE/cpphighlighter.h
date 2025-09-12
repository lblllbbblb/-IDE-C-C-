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

    // 新增：提供一个公共方法来重新高亮整个文档
    void rehighlight();

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
    QTextCharFormat namespaceFormat;
    QTextCharFormat scopeResolutionFormat;
    QTextCharFormat standardTypeFormat;
    QTextCharFormat variableFormat;
};

#endif // CPPHIGHLIGHTER_H
