#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    // 行号区域宽度计算
    int lineNumberAreaWidth();

    // 行号绘制入口
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void updateFont(const QFont &font); //字体更新

protected:
    // 窗口大小变化事件
    void resizeEvent(QResizeEvent *event) override;

    // 确保 slots 声明在正确的访问区域
private slots:
    void updateLineNumberAreaWidth();  // 无参数版本
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    LineNumberArea *lineNumberArea; // 行号组件指针
};

#endif // CODEEDITOR_H
