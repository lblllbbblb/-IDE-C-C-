#ifndef COLORREFRESHER_H
#define COLORREFRESHER_H

#include <QObject>
#include <QTextDocument>
#include <QDebug> // 用于调试输出

// 前向声明，避免循环依赖
class CppHighlighter;

class ColorRefresher : public QObject
{
    Q_OBJECT
public:
    explicit ColorRefresher(QObject *parent = nullptr);

    // 刷新指定文档的颜色
    void refreshDocumentColors(QTextDocument *document);

private:
    // 存储一个指向CppHighlighter的指针，以便我们可以调用rehighlight()
    // 注意：ColorRefresher不拥有highlighter的生命周期，它只是一个引用
    CppHighlighter *m_highlighter = nullptr;
};

#endif // COLORREFRESHER_H
