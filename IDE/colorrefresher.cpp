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
    // QSyntaxHighlighter的document()方法返回其关联的文档
    // 但是我们没有直接获取highlighter实例的方法，通常高亮器是文档的子对象
    // 更好的方式是在创建高亮器时就将其引用传递给ColorRefresher或者通过遍历文档属性查找
    // 这里我们假设CppHighlighter是直接设置为document的highlighter的
    // 由于QSyntaxHighlighter没有直接的document()->highlighter()方法
    // 我们需要一种方式来获取已经存在的高亮器实例
    // 最直接的方法是当highlighter被创建时，就将其传递给ColorRefresher
    // 或者 ColorRefresher 知道如何创建和管理 highlighter

    // 为了实现“刷新”，我们实际上需要调用现有高亮器的 rehighlight() 方法
    // QSyntaxHighlighter是QObject的子类，我们可以尝试在文档的子对象中找到它
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
        // 如果没有找到高亮器，可能需要重新创建一个
        // 但更好的做法是确保高亮器在initializeEditor中正确设置并可被ColorRefresher引用
    }
}
