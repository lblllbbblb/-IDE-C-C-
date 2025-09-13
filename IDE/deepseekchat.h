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

// 定义 DeepSeekChatDialog 类，继承自 QDialog
class DeepSeekChatDialog : public QDialog
{
    Q_OBJECT // 宏，允许类使用 Qt 的元对象系统，支持信号和槽

public:
    explicit DeepSeekChatDialog(QWidget *parent = nullptr); // 构造函数
    ~DeepSeekChatDialog(); // 析构函数

private slots:
    void sendMessage(); // 发送消息到 DeepSeek API
    void handleNetworkReply(QNetworkReply *reply); // 处理 DeepSeek API 的网络回复
    void setApiKey(); // 设置 DeepSeek API 密钥
    void toggleApiKeyVisibility(); // 切换 API 密钥的显示/隐藏

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

    void loadApiKey(); // 从设置中加载 API 密钥
    void saveApiKey(const QString &key); // 保存 API 密钥到设置
    void appendMessage(const QString &sender, const QString &message, bool isUser = false); // 向对话显示区域添加消息

    // 构建 DeepSeek API 请求的 JSON 数据
    QByteArray buildRequestJson(const QString &message);
};

#endif // DEEPSEEKCHAT_H
