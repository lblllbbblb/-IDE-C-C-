#include "LineNumberArea.h"
#include "CodeEditor.h"  // 需要访问 CodeEditor 的 lineNumberAreaWidth()

LineNumberArea::LineNumberArea(QPlainTextEdit *editor)
    : QWidget(editor), editor(editor) {}

QSize LineNumberArea::sizeHint() const {
    // 动态宽度由 CodeEditor 计算
    return QSize(static_cast<CodeEditor*>(editor)->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    // 调用 CodeEditor 的绘制方法
    static_cast<CodeEditor*>(editor)->lineNumberAreaPaintEvent(event);
}

// 鼠标按下事件处理
void LineNumberArea::mousePressEvent(QMouseEvent *event) {
    // 将点击事件传递给 CodeEditor 处理
    static_cast<CodeEditor*>(editor)->lineNumberAreaClicked(event->y());
    QWidget::mousePressEvent(event);
}
