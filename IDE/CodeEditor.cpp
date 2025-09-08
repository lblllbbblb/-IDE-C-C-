#include "CodeEditor.h"
#include "LineNumberArea.h"
#include <QPainter>
#include <QTextBlock>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);

    // 信号连接
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth();
    highlightCurrentLine();
    QFontMetrics metrics(font());
    setTabStopDistance(4 * metrics.horizontalAdvance(' '));
}

int CodeEditor::lineNumberAreaWidth() {
    // 宽度计算逻辑（同原实现）
    QFontMetrics fm(font());

    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // 添加一些边距，确保数字不会太靠近边缘
    int margin = 5;
    return margin + fm.horizontalAdvance(QLatin1Char('9')) * digits;
}
// 添加以下三个函数的实现
void CodeEditor::updateLineNumberAreaWidth() {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    updateGeometry();
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy != 0)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
}

void CodeEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(Qt::yellow).lighter(160));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}
void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                      lineNumberAreaWidth(), cr.height()));
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);

}
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(lineNumberArea);
    painter.setFont(font()); // 设置与编辑器相同的字体
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block)
                                   .translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top,
                             lineNumberArea->width(),
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
void CodeEditor::updateFont(const QFont &font)
{
    setFont(font);

    // 强制重新计算字体度量
    QFontMetrics fm(font);

    // 重新计算并更新行号区域宽度
    updateLineNumberAreaWidth();

    // 强制立即调整行号区域大小
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                      lineNumberAreaWidth(), cr.height()));

    // 强制行号区域重绘
    lineNumberArea->update();

    // 更新视口布局
    updateGeometry();

    // 重新高亮当前行
    highlightCurrentLine();

    // 强制整个编辑器重绘
    viewport()->update();
}

void CodeEditor::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Tab) {
        // 插入四个空格而不是制表符
        insertPlainText("    ");
        return;
    }

    // 对于其他按键，使用默认处理
    QPlainTextEdit::keyPressEvent(e);
}
