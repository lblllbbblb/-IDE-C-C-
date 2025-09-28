#ifndef CPPHIGHLIGHTER_H
#define CPPHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

// C++ 语法高亮器类
class CppHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    // 构造函数
    CppHighlighter(QTextDocument *parent = nullptr);

    // 重新高亮整个文档
    void rehighlight();

protected:
    // 高亮文本块
    void highlightBlock(const QString &text) override;

private:
    // 高亮规则结构体
    struct HighlightingRule
    {
        QRegularExpression pattern; // 匹配模式
        QTextCharFormat format;     // 文本格式
    };
    QVector<HighlightingRule> highlightingRules; // 高亮规则列表

    QRegularExpression commentStartExpression; // 多行注释开始
    QRegularExpression commentEndExpression;   // 多行注释结束

    QTextCharFormat keywordFormat;        // 关键字格式
    QTextCharFormat operatorFormat;       // 操作符格式
    QTextCharFormat preprocessorFormat;   // 预处理指令格式
    QTextCharFormat stringFormat;         // 字符串格式
    QTextCharFormat commentFormat;        // 注释格式
    QTextCharFormat functionFormat;       // 函数格式
    QTextCharFormat includeFormat;        // #include 格式
    QTextCharFormat numberFormat;         // 数字格式
    QTextCharFormat namespaceFormat;      // 命名空间格式
    QTextCharFormat scopeResolutionFormat; // 作用域解析符格式
    QTextCharFormat standardTypeFormat;   // 标准类型格式
    QTextCharFormat variableFormat;       // 变量格式
};

#endif // CPPHIGHLIGHTER_H
