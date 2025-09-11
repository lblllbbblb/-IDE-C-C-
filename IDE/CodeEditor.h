#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QVector> // 新增：用于存储折叠区域信息
#include <QCompleter>
#include <QStringList>
#include <QAbstractItemView>

class LineNumberArea;

// 新增：折叠区域结构体
struct FoldingBlock {
    int startLine; // 折叠区域的起始行号（0-based）
    int endLine;   // 折叠区域的结束行号（0-based）
    bool isFolded; // 当前是否处于折叠状态
};

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    // 行号区域宽度计算
    int lineNumberAreaWidth();

    // 行号绘制入口
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void updateFont(const QFont &font); //字体更新

    // 新增：处理行号区域点击事件的槽函数
    void lineNumberAreaClicked(int y);
    void insertCompletion(const QString &completion);


protected:
    // 窗口大小变化事件
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;


    // 确保 slots 声明在正确的访问区域
private slots:
    void updateLineNumberAreaWidth();  // 无参数版本
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void updateFoldingRegions(); // 新增：更新折叠区域信息
    void handleBlockCountChanged(int newBlockCount); // 新增：处理行数变化

    QString textUnderCursor() const;
private:
    LineNumberArea *lineNumberArea; // 行号组件指针

    QVector<FoldingBlock> m_foldingBlocks; // 新增：存储所有可折叠区域
    int m_foldingIndicatorWidth; // 新增：折叠指示器的宽度

    // 新增：辅助函数
    void toggleFolding(int lineNumber); // 切换指定行号的折叠状态
    void applyFolding(); // 根据 m_foldingBlocks 应用折叠状态
    bool isLineFolded(int lineNumber) const; // 判断某行是否被折叠
    bool isLineStartOfFoldedBlock(int lineNumber) const; // 判断某行是否是已折叠区域的起始行
    QCompleter *m_completer; // 自动补全器
    QStringList defaultCompletions; // 添加这行
    // 设置自动补全的单词列表
    void setupCompleter();
    QStringList getContextualCompletions() const;
    QStringList extractVariables() const;
    void onTextChanged();
    QStringList extractFunctions() const;    // 提取函数名
    QStringList extractClasses() const;      // 提取类名


};

#endif // CODEEDITOR_H
