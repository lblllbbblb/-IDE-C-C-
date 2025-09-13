#ifndef DEEPSEEKCHAT_H
#define DEEPSEEKCHAT_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSettings> // 用于保存和加载API密钥
#include <QComboBox> // 新增：用于文件操作的下拉菜单
#include <QRegularExpression> // 【修复】新增：用于正则表达式匹配，替代QRegExp

// 定义 DeepSeekChatDialog 类，继承自 QDialog
class DeepSeekChatDialog : public QDialog
{
    Q_OBJECT // 宏，允许类使用 Qt 的元对象系统，支持信号和槽

public:
    explicit DeepSeekChatDialog(
        QWidget *parent = nullptr,
        const QString &currentFilePath = "",
        const QString &editorContent = "",
        const QString &selectedCode = "",
        int cursorLine = -1,
        int cursorCol = -1,
        int selectionStartLine = -1,
        int selectionEndLine = -1
        ); // 构造函数，新增上下文参数
    ~DeepSeekChatDialog(); // 析构函数

signals:
    // 新增信号：请求将代码插入到主 IDE 的编辑器中
    void insertCodeRequested(const QString &code, int startLine, int endLine);

private slots:
    void sendMessage(); // 发送消息到 DeepSeek API
    void handleNetworkReply(QNetworkReply *reply); // 处理 DeepSeek API 的网络回复
    void setApiKey(); // 设置 DeepSeek API 密钥
    void toggleApiKeyVisibility(); // 切换 API 密钥的显示/隐藏

    // 新增槽函数，用于文件操作和代码插入
    void handleFileOperation(int index); // 处理文件操作下拉菜单的选择
    void insertCodeFromResponse(); // 将 DeepSeek 生成的代码插入到编辑器
    void saveFileContent(); // 将 DeepSeek 生成的文件内容保存为新文件
    void loadFileIntoChat(); // 加载本地文件内容到聊天上下文

private:
    QNetworkAccessManager *m_networkManager; // 网络访问管理器，用于发送 HTTP 请求
    QString m_apiKey; // DeepSeek API 密钥
    QString m_apiUrl; // DeepSeek API 的 URL

    // UI 元素
    QTextEdit *m_chatDisplay; // 显示对话历史的文本框
    QLineEdit *m_messageInput; // 用户输入消息的单行文本框
    QPushButton *m_sendButton; // 发送消息按钮
    QPushButton *m_setApiKeyButton; // 设置 API 密钥按钮
    QLabel *m_statusLabel; // 显示状态信息（如错误）
    QLineEdit *m_apiKeyInput; // 输入 API 密钥的文本框
    QPushButton *m_toggleApiKeyButton; // 切换 API 密钥可见性的按钮

    // 新增 UI 元素和上下文信息
    QTextEdit *m_fileContentDisplay; // 显示当前文件内容的文本框
    QTextEdit *m_responseCodeDisplay; // 显示 DeepSeek 返回的代码片段
    QPushButton *m_insertCodeButton; // 插入代码按钮
    QPushButton *m_saveFileButton; // 保存文件按钮 (用于 DeepSeek 生成的文件)
    QComboBox *m_fileOperationsComboBox; // 文件操作下拉菜单

    QString m_currentFilePath;   // 当前在 IDE 中打开的文件路径
    QString m_editorContent;     // 当前 IDE 编辑器的完整内容
    QString m_selectedCode;      // 当前 IDE 编辑器中选中的代码
    int m_cursorLine;            // 当前光标行号
    int m_cursorCol;             // 当前光标列号
    int m_selectionStartLine;    // 选中代码的起始行号
    int m_selectionEndLine;      // 选中代码的结束行号

    void loadApiKey(); // 从设置中加载 API 密钥
    void saveApiKey(const QString &key); // 保存 API 密钥到设置
    void appendMessage(const QString &sender, const QString &message, bool isUser = false); // 向对话显示区域添加消息

    // 构建 DeepSeek API 请求的 JSON 数据，现在需要包含更多上下文
    QByteArray buildRequestJson(const QString &userMessage);
};

#endif // DEEPSEEKCHAT_H
