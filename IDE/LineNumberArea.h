#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include <QPlainTextEdit>

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(QPlainTextEdit *editor);

    // 动态调整宽度
    QSize sizeHint() const override;

protected:
    // 绘制事件委托给编辑器
    void paintEvent(QPaintEvent *event) override;

private:
    QPlainTextEdit *editor; // 关联的编辑器对象
};

#endif // LINENUMBERAREA_H
