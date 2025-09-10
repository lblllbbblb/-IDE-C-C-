#include "CodeEditor.h"
#include "LineNumberArea.h"
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent> // 用于处理 Ctrl+Tab
#include <QStack>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent), m_foldingIndicatorWidth(15) { // 初始化折叠指示器宽度
    lineNumberArea = new LineNumberArea(this);

    // 信号连接
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    // 新增：连接文档内容改变信号，用于更新折叠区域
    connect(this, &QPlainTextEdit::textChanged,
            this, &CodeEditor::updateFoldingRegions);
    // 新增：连接 blockCountChanged 信号，用于处理行数变化对折叠区域的影响
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::handleBlockCountChanged);


    updateLineNumberAreaWidth();
    highlightCurrentLine();
    QFontMetrics metrics(font());
    setTabStopDistance(4 * metrics.horizontalAdvance(' '));

    // 首次加载时更新折叠区域
    updateFoldingRegions();
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
    // 新增：加上折叠指示器的宽度
    return margin + fm.horizontalAdvance(QLatin1Char('9')) * digits + m_foldingIndicatorWidth;
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
        selection.format.setBackground(QColor(150, 150, 150, 80).lighter(120));
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

    // 行号区域左侧绘制折叠指示器
    int foldingIndicatorLeft = 0; // 折叠指示器在行号区域的左侧
    int lineNumberTextLeft = m_foldingIndicatorWidth; // 行号文本在折叠指示器右侧

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);

            // 绘制行号
            painter.drawText(lineNumberTextLeft, top,
                             lineNumberArea->width() - m_foldingIndicatorWidth, // 减去指示器宽度
                             fontMetrics().height(),
                             Qt::AlignRight, number);

            // 绘制折叠指示器
            // 只有当行是可折叠区域的起始行时才绘制
            for (const FoldingBlock& fb : m_foldingBlocks) {
                if (fb.startLine == blockNumber) {
                    painter.setPen(Qt::darkGray);
                    // 绘制方框
                    painter.drawRect(foldingIndicatorLeft + 2, top + (fontMetrics().height() - 10) / 2, 10, 10);
                    // 绘制加号或减号
                    if (fb.isFolded) {
                        painter.drawLine(foldingIndicatorLeft + 5, top + (fontMetrics().height() - 10) / 2 + 5,
                                         foldingIndicatorLeft + 9, top + (fontMetrics().height() - 10) / 2 + 5); // 横线
                        painter.drawLine(foldingIndicatorLeft + 7, top + (fontMetrics().height() - 10) / 2 + 3,
                                         foldingIndicatorLeft + 7, top + (fontMetrics().height() - 10) / 2 + 7); // 竖线
                    } else {
                        painter.drawLine(foldingIndicatorLeft + 5, top + (fontMetrics().height() - 10) / 2 + 5,
                                         foldingIndicatorLeft + 9, top + (fontMetrics().height() - 10) / 2 + 5); // 横线
                    }
                    break; // 找到即退出
                }
            }
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

// 新增：处理行号区域点击事件
void CodeEditor::lineNumberAreaClicked(int y) {
    // 将 Y 坐标转换为行号
    QTextBlock block = firstVisibleBlock();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int blockNumber = block.blockNumber();

    while (block.isValid() && top <= y) {
        int bottom = top + static_cast<int>(blockBoundingRect(block).height());
        if (y >= top && y < bottom) {
            // 点击的行号是 blockNumber
            // 判断是否点击了折叠指示器区域
            if (mapFromGlobal(QCursor::pos()).x() < m_foldingIndicatorWidth + viewportMargins().left()) {
                toggleFolding(blockNumber);
                break;
            }
        }
        block = block.next();
        top = bottom;
        blockNumber++;
    }
}

// 新增：切换指定行号的折叠状态
void CodeEditor::toggleFolding(int lineNumber) {
    for (FoldingBlock& fb : m_foldingBlocks) {
        if (fb.startLine == lineNumber) {
            fb.isFolded = !fb.isFolded;
            applyFolding(); // 应用折叠状态
            lineNumberArea->update(); // 更新行号区域显示
            break;
        }
    }
}

// 新增：根据 m_foldingBlocks 应用折叠状态
void CodeEditor::applyFolding() {
    // 遍历所有文本块
    QTextBlock block = document()->begin();
    while (block.isValid()) {
        int currentLine = block.blockNumber();
        bool shouldBeVisible = true;

        // 检查当前行是否在某个已折叠的区域内
        for (const FoldingBlock& fb : m_foldingBlocks) {
            if (fb.isFolded && currentLine > fb.startLine && currentLine <= fb.endLine) {
                shouldBeVisible = false;
                break;
            }
        }
        // 设置块的可见性
        block.setVisible(shouldBeVisible);
        block = block.next();
    }
    // 强制重新布局文本
    document()->markContentsDirty(0, document()->characterCount());
    viewport()->update();
}

// 新增：更新折叠区域信息
void CodeEditor::updateFoldingRegions() {
    m_foldingBlocks.clear(); // 清空原有折叠区域

    int braceCount = 0; // 用于匹配大括号
    QStack<int> braceStack; // 存储 '{' 出现的行号

    QTextBlock block = document()->begin();
    while (block.isValid()) {
        int currentLine = block.blockNumber();
        QString text = block.text();

        // 简陋的C++风格代码块检测（基于大括号）
        // 实际IDE会进行更复杂的AST解析，这里只做简单的匹配
        for (int i = 0; i < text.length(); ++i) {
            if (text.at(i) == '{') {
                braceStack.push(currentLine);
            } else if (text.at(i) == '}') {
                if (!braceStack.isEmpty()) {
                    int startLine = braceStack.pop();
                    // 确保折叠块至少包含一行内容，且起始行和结束行不同
                    if (startLine < currentLine) {
                        FoldingBlock fb;
                        fb.startLine = startLine;
                        fb.endLine = currentLine;
                        fb.isFolded = false; // 默认不折叠
                        m_foldingBlocks.append(fb);
                    }
                }
            }
        }
        block = block.next();
    }

    // 根据新的折叠区域信息重新应用折叠状态
    applyFolding();
    lineNumberArea->update(); // 强制行号区域重绘
}

// 新增：处理行数变化
void CodeEditor::handleBlockCountChanged(int newBlockCount) {
    // 当行数变化时，需要重新分析折叠区域
    updateFoldingRegions();
}

// 新增：判断某行是否被折叠
bool CodeEditor::isLineFolded(int lineNumber) const {
    for (const FoldingBlock& fb : m_foldingBlocks) {
        if (fb.isFolded && lineNumber > fb.startLine && lineNumber <= fb.endLine) {
            return true;
        }
    }
    return false;
}

// 新增：判断某行是否是已折叠区域的起始行
bool CodeEditor::isLineStartOfFoldedBlock(int lineNumber) const {
    for (const FoldingBlock& fb : m_foldingBlocks) {
        if (fb.isFolded && fb.startLine == lineNumber) {
            return true;
        }
    }
    return false;
}
