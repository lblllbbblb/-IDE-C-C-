#include "CodeFormatter.h"
#include <QRegularExpression>

QString CodeFormatter::formatCode(const QString &code)
{
    QString formattedCode = code;

    // 1. 移除运算符周围的额外空格，但保留一个空格
    // 示例: "a + b" -> "a + b"
    // 示例: "a  +  b" -> "a + b"
    // 示例: "a+b" -> "a+b" （保持原样，因为要求运算符之间不增加空格）
    formattedCode.replace(QRegularExpression("\\s*([+\\-*/%&|^=<>!])\\s*"), "\\1");

    // 2. 在逗号和分号后添加一个空格（如果后面没有空格）
    // 示例: "func(a,b);" -> "func(a, b);"
    formattedCode.replace(QRegularExpression("([;,])(?=\\S)"), "\\1 ");

    // 3. 移除括号内部紧邻的空格
    // 示例: "func( a )" -> "func(a)"
    formattedCode.replace(QRegularExpression("(\\()\\s*"), "\\1");
    formattedCode.replace(QRegularExpression("\\s*(\\))"), "\\1");

    // 4. 移除方括号内部紧邻的空格
    // 示例: "arr[ i ]" -> "arr[i]"
    formattedCode.replace(QRegularExpression("(\\[)\\s*"), "\\1");
    formattedCode.replace(QRegularExpression("\\s*(\\])"), "\\1");

    // 5. 移除大括号内部紧邻的空格
    // 示例: "{ int x; }" -> "{int x;}"
    formattedCode.replace(QRegularExpression("(\\{)\\s*"), "\\1");
    formattedCode.replace(QRegularExpression("\\s*(\\})"), "\\1");

    // 6. 确保在关键字（如if, for, while）和左括号之间有一个空格
    // 示例: "if(condition)" -> "if (condition)"
    formattedCode.replace(QRegularExpression("\\b(if|for|while|switch|catch)\\("), "\\1 (");

    // 7. 移除行尾的空白字符
    // 注意：这将移除所有行尾空格，包括制表符和空格。
    formattedCode.replace(QRegularExpression("[ \t]+$", QRegularExpression::MultilineOption), "");

    // 8. 确保在运算符之前没有空格，例如 `a = 1` 变成 `a=1`
    formattedCode.replace(QRegularExpression("([+\\-*/%&|^=<>!])\\s+"), "\\1");

    // 9. 确保在运算符之后没有空格，例如 `a =1` 变成 `a=1`
    formattedCode.replace(QRegularExpression("\\s+([+\\-*/%&|^=<>!])"), "\\1");

    return formattedCode;
}
