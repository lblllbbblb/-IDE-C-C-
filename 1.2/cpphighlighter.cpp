#include "cpphighlighter.h"
#include <QBrush>
#include <QFont>

CppHighlighter::CppHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // 1. 关键字 (Keywords)
    keywordFormat.setForeground(QColor("#0000FF")); // 蓝色
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bextern\\b" << "\\bfloat\\b" << "\\bfor\\b"
                    << "\\bfriend\\b" << "\\bgoto\\b" << "\\binline\\b"
                    << "\\bint\\b" << "\\blong\\b" << "\\bnamespace\\b"
                    << "\\boperator\\b" << "\\bprivate\\b" << "\\bprotected\\b"
                    << "\\bpublic\\b" << "\\bshort\\b" << "\\bsigned\\b"
                    << "\\bsizeof\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\bthis\\b" << "\\btypedef\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bwhile\\b"
                    << "\\bbreak\\b" << "\\bcase\\b" << "\\bcontinue\\b"
                    << "\\bdefault\\b" << "\\bdo\\b" << "\\belse\\b"
                    << "\\bif\\b" << "\\breturn\\b" << "\\bswitch\\b"
                    << "\\bcase\\b" << "\\bdefault\\b" << "\\btry\\b"
                    << "\\bcatch\\b" << "\\bthrow\\b" << "\\busing\\b"
                    << "\\btypename\\b" << "\\bauto\\b" << "\\bdecltype\\b"
                    << "\\bnullptr\\b" << "\\bnew\\b" << "\\bdelete\\b"
                    << "\\btrue\\b" << "\\bfalse\\b"; // C++11/14/17 关键字
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 2. 操作符 (Operators) - 更具体的匹配，防止与关键字冲突
    operatorFormat.setForeground(QColor("#FF0000")); // 红色
    QStringList operatorPatterns;
    operatorPatterns << "\\b(and|or|not|xor|bitand|bitor|compl|not_eq|or_eq|xor_eq)\\b" // C++ 文本操作符
                     << "[!%&\\*\\+\\-\\./:<=>\\?\\^\\|~]"; // 常见符号操作符
    for (const QString &pattern : operatorPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }


    // 3. 预处理指令 (Preprocessor Directives)
    preprocessorFormat.setForeground(QColor("#800080")); // 紫色
    preprocessorFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^\\s*#.*$"); // 匹配以 # 开头的一整行
    rule.format = preprocessorFormat;
    highlightingRules.append(rule);

    // 4. #include (特殊处理，如果不想和预处理指令混淆)
    includeFormat.setForeground(QColor("#FF1493")); // 深粉色
    rule.pattern = QRegularExpression("#include\\s*<[^>]+>|#include\\s*\"[^\"]+\"");
    rule.format = includeFormat;
    highlightingRules.append(rule);

    // 5. 字符串 (Strings)
    stringFormat.setForeground(QColor("#A31515")); // 深红色
    rule.pattern = QRegularExpression("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"|'[^'\\\\]*(\\\\.[^'\\\\]*)*'"); // 匹配 "..." 和 '...'
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // 6. 函数 (Functions) - 识别后跟括号的单词
    functionFormat.setForeground(QColor("#B22222")); // 砖红色
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 7. 数字 (Numbers) - 整数、浮点数、十六进制等
    numberFormat.setForeground(QColor("#1C00CF")); // 深蓝色
    rule.pattern = QRegularExpression("\\b[0-9]+\\b|\\b0[xX][0-9a-fA-F]+\\b|\\b[0-9]*\\.[0-9]+([eE][+-]?[0-9]+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 8. 单行注释 (Single-line Comments)
    commentFormat.setForeground(QColor("#008000")); // 绿色
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // 9. 多行注释 (Multi-line Comments)
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void CppHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // 处理多行注释
    setCurrentBlockState(0); // 重置当前块状态，0表示非多行注释

    int startIndex = 0;
    if (previousBlockState() != 1) { // 如果前一个块不是多行注释，则从头开始找
        startIndex = text.indexOf(commentStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) { // 没找到结束符，说明本行及之后都是多行注释
            setCurrentBlockState(1); // 标记当前块为多行注释
            commentLength = text.length() - startIndex;
        } else { // 找到结束符
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength); // 继续寻找下一个多行注释
    }
}
