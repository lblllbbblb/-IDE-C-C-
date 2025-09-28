#include "colorrefresher.h"
#include "cpphighlighter.h" // 包含CppHighlighter头文件

ColorRefresher::ColorRefresher(QObject *parent)
    : QObject(parent)
{
    // 构造函数中不做太多事情，因为高亮器在initializeEditor中创建
}

void ColorRefresher::refreshDocumentColors(QTextDocument *document)
{
    if (!document) {
        qDebug() << "Error: Document is null, cannot refresh colors.";
        return;
    }

    // 获取文档关联的语法高亮器
    // 为了实现刷新，我们调用现有高亮器的 rehighlight() 方法
    // QSyntaxHighlighter是QObject的子类
    for (QObject *child : document->children()) {
        m_highlighter = qobject_cast<CppHighlighter*>(child);
        if (m_highlighter) {
            break; // 找到高亮器
        }
    }

    if (m_highlighter) {
        m_highlighter->rehighlight();
        qDebug() << "Document colors refreshed.";
    } else {
        qDebug() << "Error: CppHighlighter not found for the document.";
    }
}
