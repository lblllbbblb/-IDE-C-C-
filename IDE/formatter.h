#ifndef FORMATTER_H
#define FORMATTER_H

#include <QString>
#include <QTextDocument> // 用于处理文本内容

class Formatter
{
public:
    Formatter();

    // 格式化 C++ 代码的函数
    QString formatCppCode(const QString &code);

private:
    // 辅助函数：根据规则添加或删除空格、换行等
    QString applyFormattingRules(const QString &code);
    QString indentCode(const QString &code);
};

#endif // FORMATTER_H
