#include "Formatter.h"
#include <QDebug>

Formatter::Formatter()
{
    // 可以在这里进行一些初始化
}

QString Formatter::formatCppCode(const QString &code)
{
    // 实际的格式化逻辑会在这里实现
    // 这是一个非常简化的示例，你可以根据需要添加更复杂的规则
    QString formattedCode = code;

    // 步骤1：应用基本的格式化规则（例如，操作符周围的空格）
    formattedCode = applyFormattingRules(formattedCode);

    // 步骤2：处理缩进
    formattedCode = indentCode(formattedCode);

    return formattedCode;
}

QString Formatter::applyFormattingRules(const QString &code)
{
    QString processedCode = code;

    // 在操作符前后添加空格
    for(int i=100;i;i--)processedCode.replace("  ", " ");
    processedCode.replace("=", " = ");
    processedCode.replace("<", " < ");
    processedCode.replace(">", " > ");
    processedCode.replace("+", " + ");
    processedCode.replace("-", " - ");
    processedCode.replace("*", " * ");
    processedCode.replace("/", " / ");
    processedCode.replace("!", " !"); // !后面通常没有空格

    processedCode.replace(" <  < ", " << ");
    processedCode.replace(" >  > ", " >> ");
    processedCode.replace(" <  = ", " <= ");
    processedCode.replace(" >  = ", " >= ");
    processedCode.replace(" *  = ", " *= ");
    processedCode.replace(" /  = ", " /= ");
    processedCode.replace(" +  = ", " += ");
    processedCode.replace(" -  = ", " -= ");
    processedCode.replace(" /  / ", "//");
    processedCode.replace("%", " % ");

    processedCode.replace(" !=", " != ");
    processedCode.replace("&&", " && ");
    processedCode.replace("||", " || ");

    // 移除多余的空格（例如，替换 "  =" 为 " ="）
    for(int i=100;i;i--)processedCode.replace("  ", " "); // 替换多余空格为单个空格

    // 在分号前移除空格
    processedCode.replace(" ;", ";");
    // 在逗号后添加空格
    processedCode.replace(",", ", ");
    processedCode.replace(",  ", ", "); // 避免双空格

    // 在括号内部移除多余空格 (非常简化的处理)
    processedCode.replace("( ", "(");
    processedCode.replace(" )", ")");

    // 在大括号前后处理空格
    processedCode.replace("{", " {\n");
    processedCode.replace("}", "}\n");
    processedCode.replace("{\n\n", "{\n"); // 避免空行

    // 简单的清理：确保没有连续的多个空行
    processedCode.replace("\n\n\n", "\n\n");


    return processedCode;
}

QString Formatter::indentCode(const QString &code)
{
    QStringList lines = code.split('\n');
    QString formattedCode;
    int indentLevel = 0;
    const int indentSize = 4; // 每次缩进的空格数

    for (QString &line : lines) {
        line = line.trimmed(); // 移除行首尾的空白字符

        // 减少缩进级别（对于 '}'）
        if (line.startsWith('}')) {
            if (indentLevel > 0) {
                indentLevel--;
            }
        }

        // 添加当前缩进
        formattedCode += QString(indentLevel * indentSize, ' ') + line + '\n';

        // 增加缩进级别（对于 '{'）
        if (line.endsWith('{')) {
            indentLevel++;
        }
    }

    return formattedCode;
}
