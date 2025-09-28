#include "CodeEditor.h"
#include "LineNumberArea.h"
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent> // 用于处理 Ctrl+Tab
#include <QStack>
#include <QScrollBar>
#include <QAbstractItemView>
#include <QStringListModel> // 用于QCompleter的模型
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <Qset>
#include <QApplication>

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

    //连接文档内容改变信号，用于更新折叠区域
    connect(this, &QPlainTextEdit::textChanged,
            this, &CodeEditor::updateFoldingRegions);
    //连接 blockCountChanged 信号，用于处理行数变化对折叠区域的影响
    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::handleBlockCountChanged);


    updateLineNumberAreaWidth();
    highlightCurrentLine();
    QFontMetrics metrics(font());
    setTabStopDistance(4 * metrics.horizontalAdvance(' '));
    setupCompleter();
    // 首次加载时更新折叠区域
    updateFoldingRegions();
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::onTextChanged);
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
    // 加上折叠指示器的宽度
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
    // 1. 补全弹窗显示时优先处理选择操作
    if (m_completer && m_completer->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
            e->ignore(); // 让补全器处理这些按键
            return;
        default:
            break;
        }
    }

    // 2. 处理Tab键（替换为4个空格）
    if (e->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        e->accept();
        return;
    }

    // 3. 判断是否为补全快捷键(Ctrl+Space)
    bool isShortcut = (e->modifiers() & Qt::ControlModifier) &&
                      (e->key() == Qt::Key_Space);

    // 4. 处理普通输入（非快捷键时）
    if (!isShortcut) {
        QPlainTextEdit::keyPressEvent(e);
    }

    // 5. 定义补全相关判断条件
    const bool isWordChar = e->text().contains(QRegularExpression("[A-Za-z0-9_]"));
    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    static const QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // 补全结束字符

    // 6. 不需要显示补全的情况
    if (!isShortcut && (!isWordChar || hasModifier || e->text().isEmpty() ||
                        eow.contains(e->text().right(1)) ||
                        textUnderCursor().length() < 2)) {
        if (m_completer) m_completer->popup()->hide();
        return;
    }

    // 7. 更新补全列表
    QString completionPrefix = textUnderCursor();
    if (m_completer) {
        if (completionPrefix != m_completer->completionPrefix()) {
            m_completer->setCompletionPrefix(completionPrefix);
            m_completer->setModel(new QStringListModel(getContextualCompletions(), m_completer));
            m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
        }

        // 8. 使用更精确的方法计算弹窗位置
        // 获取光标矩形（相对于视口）
        QRect cursorRect = this->cursorRect();

        // 转换为全局坐标
        QPoint globalCursorPos = viewport()->mapToGlobal(cursorRect.bottomLeft());

        // 弹窗大小
        int popupWidth = m_completer->popup()->sizeHintForColumn(0) +
                         m_completer->popup()->verticalScrollBar()->sizeHint().width();

        // 根据补全项数量动态调整高度
        int itemCount = m_completer->completionCount();
        int maxVisibleItems = 7;
        int itemHeight = m_completer->popup()->sizeHintForRow(0);
        int popupHeight = qMin(itemCount, maxVisibleItems) * itemHeight +
                          m_completer->popup()->horizontalScrollBar()->sizeHint().height();

        // 计算弹窗位置（光标下方）
        QPoint popupPos(globalCursorPos.x(), globalCursorPos.y() + 2); // 光标底部 + 2px间距

        QRect popupRect(popupPos, QSize(popupWidth, popupHeight));

        // 确保弹窗不超出屏幕边界
        QScreen *screen = QGuiApplication::screenAt(popupPos);
        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }

        if (screen) {
            QRect screenGeometry = screen->availableGeometry();

            // 检查弹窗是否会超出屏幕底部
            if (popupRect.bottom() > screenGeometry.bottom()) {
                // 如果超出，显示在光标上方
                popupRect.moveBottom(globalCursorPos.y() - 2);
            }

            // 检查弹窗是否会超出屏幕右侧
            if (popupRect.right() > screenGeometry.right()) {
                popupRect.moveRight(screenGeometry.right());
            }

            // 检查弹窗是否会超出屏幕左侧
            if (popupRect.left() < screenGeometry.left()) {
                popupRect.moveLeft(screenGeometry.left());
            }
        }

        // 显示补全弹窗
        m_completer->complete(popupRect);
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

// 根据 m_foldingBlocks 应用折叠状态
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

// 更新折叠区域信息
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

// 处理行数变化
void CodeEditor::handleBlockCountChanged(int newBlockCount) {
    // 当行数变化时，需要重新分析折叠区域
    updateFoldingRegions();
}

// 判断某行是否被折叠
bool CodeEditor::isLineFolded(int lineNumber) const {
    for (const FoldingBlock& fb : m_foldingBlocks) {
        if (fb.isFolded && lineNumber > fb.startLine && lineNumber <= fb.endLine) {
            return true;
        }
    }
    return false;
}

// 判断某行是否是已折叠区域的起始行
bool CodeEditor::isLineStartOfFoldedBlock(int lineNumber) const {
    for (const FoldingBlock& fb : m_foldingBlocks) {
        if (fb.isFolded && fb.startLine == lineNumber) {
            return true;
        }
    }
    return false;
}
void CodeEditor::setupCompleter()
{
    // 扩展默认补全词库，包含更多C++关键词和常用函数
    defaultCompletions = {
        // 基本类型和关键词
        "int", "float", "double", "char", "bool", "void", "long", "short",
        "class", "struct", "enum", "union", "namespace", "using", "typedef",
        "public", "private", "protected", "virtual", "override", "final",
        "static", "const", "volatile", "mutable", "extern", "register",
        // 控制流
        "if", "else", "switch", "case", "default", "for", "while", "do",
        "break", "continue", "return", "goto",
        // 常用容器和函数
        "vector", "string", "map", "set", "list", "queue", "stack",
        "cout", "cin", "printf", "scanf", "malloc", "free", "new", "delete",
        "sizeof", "NULL", "nullptr", "this", "true", "false",
        // 常用函数
        "main", "printf", "scanf", "cout<<", "cin>>", "getline", "push_back",
        "pop_back", "size", "length", "begin", "end", "insert", "erase"
    };

    // 补全器初始化（保持不变）
    m_completer = new QCompleter(defaultCompletions, this);
    m_completer->setWidget(this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);

    // 补全弹窗样式（保持不变）
    QAbstractItemView *popup = m_completer->popup();
    popup->setStyleSheet("QListView { font-size: 14px; border: 1px solid #ccc; }"
                         "QListView::item:selected { background-color: #4A86E8; color: white; }");
    popup->setMinimumWidth(150);

    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
}

QStringList CodeEditor::getContextualCompletions() const
{
    QString currentText = textUnderCursor();
    QStringList contextualCompletions = defaultCompletions;

    // 添加变量名、函数名和类名
    contextualCompletions << extractVariables();
    contextualCompletions << extractFunctions();
    contextualCompletions << extractClasses();

    // 过滤与当前输入匹配的补全项
    if (!currentText.isEmpty()) {
        QRegularExpression regex(currentText, QRegularExpression::CaseInsensitiveOption);
        contextualCompletions = contextualCompletions.filter(regex);
    }

    // 语法感知补全（如cout相关）
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    QString lineText = cursor.block().text();
    if (lineText.contains("cout.")) {
        contextualCompletions.append("cout<<");
    }

    // 去重处理（兼容低版本Qt）
    QSet<QString> tempSet;
    for (const QString& item : contextualCompletions) {
        tempSet.insert(item);
    }
    contextualCompletions.clear();
    QSet<QString>::iterator it;
    for (it = tempSet.begin(); it != tempSet.end(); ++it) {
        contextualCompletions.append(*it);
    }

    return contextualCompletions;
}

void CodeEditor::focusInEvent(QFocusEvent *e) {
    if (m_completer) {
        m_completer->setWidget(this);
    }
    QPlainTextEdit::focusInEvent(e);
}

QString CodeEditor::textUnderCursor() const {
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor); // 选中光标下的单词
    return cursor.selectedText();
}

void CodeEditor::insertCompletion(const QString &completion) {
    if (!m_completer) return;

    QTextCursor cursor = textCursor();
    // 获取当前补全前缀的长度（即用户已经输入的部分）
    int prefixLength = m_completer->completionPrefix().length();

    // 删除光标前的前缀内容（避免补全内容与原有内容重叠）
    cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, prefixLength);
    cursor.removeSelectedText();

    // 插入完整的补全内容
    cursor.insertText(completion);

    // 更新光标位置
    setTextCursor(cursor);
}
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
QStringList CodeEditor::extractVariables() const
{
    QStringList variables;
    QTextDocument *doc = document();
    if (!doc) return variables;

    // 正则表达式匹配C++变量定义（基础版本，可根据需要扩展）
    // 匹配规则：类型 + 空格 + 变量名（支持指针和引用）
    QRegularExpression varRegex(
        // 匹配常见类型（可扩展）
        "(int|float|double|char|bool|void|long|short|string|vector|auto)\\s+"
        // 匹配指针(*)、引用(&)或普通变量
        "([*&]?\\s*[a-zA-Z_][a-zA-Z0-9_]*)"
        // 忽略初始化部分(= ...)
        "(\\s*=.*)?;",
        QRegularExpression::CaseInsensitiveOption
        );

    // 遍历所有行提取变量
    for (int i = 0; i < doc->blockCount(); ++i) {
        QString line = doc->findBlockByNumber(i).text();
        QRegularExpressionMatch match = varRegex.match(line);

        if (match.hasMatch()) {
            QString varName = match.captured(2).trimmed();
            // 清理可能的指针/引用符号
            varName.remove(QRegularExpression("[*&]"));
            if (!varName.isEmpty() && !variables.contains(varName)) {
                variables << varName;
            }
        }
    }

    return variables;
}
void CodeEditor::onTextChanged()
{
    // 当文本变化时，如果补全弹窗可见则更新
    if (m_completer && m_completer->popup()->isVisible()) {
        QString completionPrefix = textUnderCursor();
        m_completer->setCompletionPrefix(completionPrefix);
        m_completer->setModel(new QStringListModel(getContextualCompletions(), m_completer));
    }
}

// 1. 提取函数名
QStringList CodeEditor::extractFunctions() const
{
    QStringList functions;
    QTextDocument *doc = document();
    if (!doc) return functions;

    // 匹配函数定义（基础版本）
    // 支持：返回类型 + 函数名 + (参数列表)
    QRegularExpression funcRegex(
        // 匹配返回类型（包含指针和引用）
        "([a-zA-Z_][a-zA-Z0-9_*&\\s:]+)\\s+"
        // 匹配函数名
        "([a-zA-Z_][a-zA-Z0-9_]*)\\s*"
        // 匹配参数列表
        "\\([^\\)]*\\)",
        QRegularExpression::CaseInsensitiveOption
        );

    // 遍历所有行提取函数
    for (int i = 0; i < doc->blockCount(); ++i) {
        QString line = doc->findBlockByNumber(i).text();
        // 跳过注释行
        if (line.trimmed().startsWith("//") || line.trimmed().startsWith("/*"))
            continue;

        QRegularExpressionMatch match = funcRegex.match(line);
        if (match.hasMatch()) {
            QString funcName = match.captured(2).trimmed();
            if (!funcName.isEmpty() && !functions.contains(funcName)) {
                functions << funcName;
            }
        }
    }

    return functions;
}

// 2. 提取类名和结构体名
QStringList CodeEditor::extractClasses() const
{
    QStringList classes;
    QTextDocument *doc = document();
    if (!doc) return classes;

    // 匹配类和结构体定义
    QRegularExpression classRegex(
        "(class|struct)\\s+([a-zA-Z_][a-zA-Z0-9_]*)",
        QRegularExpression::CaseInsensitiveOption
        );

    // 遍历所有行提取类名
    for (int i = 0; i < doc->blockCount(); ++i) {
        QString line = doc->findBlockByNumber(i).text();
        // 跳过注释行
        if (line.trimmed().startsWith("//") || line.trimmed().startsWith("/*"))
            continue;

        QRegularExpressionMatch match = classRegex.match(line);
        if (match.hasMatch()) {
            QString className = match.captured(2).trimmed();
            if (!className.isEmpty() && !classes.contains(className)) {
                classes << className;
            }
        }
    }

    return classes;
}
