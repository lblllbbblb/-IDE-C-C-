#include "DeepSeekChat.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog> // 用于获取 API 密钥
#include <QDebug> // 用于调试输出

DeepSeekChatDialog::DeepSeekChatDialog(QWidget *parent)
    : QDialog(parent),
    m_networkManager(new QNetworkAccessManager(this)), // 初始化网络访问管理器
    m_apiUrl("https://api.deepseek.com/chat/completions") // DeepSeek API 的聊天完成接口 URL
{
    setWindowTitle("DeepSeek AI 助手"); // 设置对话框标题
    setWindowIcon(QIcon(":/icons/deepseek.png")); // 设置对话框图标，你需要添加一个 deepseek.png 到资源文件

    // 初始化 UI 元素
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true); // 设置为只读，用于显示聊天内容
    m_chatDisplay->setPlaceholderText("在这里与 DeepSeek AI 对话...");
    m_chatDisplay->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc; padding: 5px;");

    m_messageInput = new QLineEdit(this);
    m_messageInput->setPlaceholderText("输入你的消息...");
    m_messageInput->setStyleSheet("padding: 5px; border: 1px solid #ccc;");

    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px 15px; border-radius: 5px;");

    m_setApiKeyButton = new QPushButton("设置 API 密钥", this);
    m_setApiKeyButton->setStyleSheet("background-color: #008CBA; color: white; padding: 8px 15px; border-radius: 5px;");

    m_statusLabel = new QLabel("就绪", this); // 状态标签，显示操作状态或错误信息
    m_statusLabel->setStyleSheet("color: gray; font-size: 10px;");

    // API 密钥输入行和切换按钮
    m_apiKeyInput = new QLineEdit(this);
    m_apiKeyInput->setPlaceholderText("输入 DeepSeek API 密钥");
    m_apiKeyInput->setEchoMode(QLineEdit::Password); // 默认密码模式隐藏密钥
    m_apiKeyInput->setStyleSheet("padding: 5px; border: 1px solid #ccc;");

    m_toggleApiKeyButton = new QPushButton("显示", this);
    m_toggleApiKeyButton->setCheckable(true); // 使按钮可切换状态
    m_toggleApiKeyButton->setStyleSheet("padding: 5px; border: 1px solid #ccc;");


    // 布局管理
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *inputLayout = new QHBoxLayout();
    QHBoxLayout *apiKeyLayout = new QHBoxLayout(); // API 密钥布局

    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);

    apiKeyLayout->addWidget(m_apiKeyInput);
    apiKeyLayout->addWidget(m_toggleApiKeyButton);
    apiKeyLayout->addWidget(m_setApiKeyButton); // 将设置API密钥按钮也放在这一行

    mainLayout->addWidget(m_chatDisplay);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(apiKeyLayout);
    mainLayout->addWidget(m_statusLabel); // 添加状态标签

    // 连接信号和槽
    connect(m_sendButton, &QPushButton::clicked, this, &DeepSeekChatDialog::sendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &DeepSeekChatDialog::sendMessage); // 按回车键发送
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &DeepSeekChatDialog::handleNetworkReply);
    connect(m_setApiKeyButton, &QPushButton::clicked, this, &DeepSeekChatDialog::setApiKey);
    connect(m_toggleApiKeyButton, &QPushButton::toggled, this, &DeepSeekChatDialog::toggleApiKeyVisibility);

    // 加载之前保存的 API 密钥
    loadApiKey();
}

DeepSeekChatDialog::~DeepSeekChatDialog()
{
    // QNetworkAccessManager 和其他子部件会在父对象析构时自动删除
}

void DeepSeekChatDialog::loadApiKey()
{
    QSettings settings("MyIDE", "DeepSeekChat"); // 应用程序名称和组织名称
    m_apiKey = settings.value("apiKey").toString();
    m_apiKeyInput->setText(m_apiKey); // 在输入框中显示已加载的密钥（可能被隐藏）
    if (m_apiKey.isEmpty()) {
        m_statusLabel->setText("请设置 DeepSeek API 密钥。");
    } else {
        m_statusLabel->setText("API 密钥已加载。");
    }
}

void DeepSeekChatDialog::saveApiKey(const QString &key)
{
    QSettings settings("MyIDE", "DeepSeekChat");
    settings.setValue("apiKey", key);
    m_apiKey = key;
    m_apiKeyInput->setText(m_apiKey); // 更新输入框
    m_statusLabel->setText("API 密钥已保存。");
}

void DeepSeekChatDialog::setApiKey()
{
    // 允许用户直接在输入框修改密钥，或者弹出一个对话框
    QString newKey = m_apiKeyInput->text().trimmed();
    if (newKey.isEmpty()) {
        QMessageBox::warning(this, "设置 API 密钥", "API 密钥不能为空。");
        return;
    }
    saveApiKey(newKey);
    QMessageBox::information(this, "设置 API 密钥", "API 密钥已更新。");
}

void DeepSeekChatDialog::toggleApiKeyVisibility()
{
    if (m_toggleApiKeyButton->isChecked()) {
        m_apiKeyInput->setEchoMode(QLineEdit::Normal); // 显示密钥
        m_toggleApiKeyButton->setText("隐藏");
    } else {
        m_apiKeyInput->setEchoMode(QLineEdit::Password); // 隐藏密钥
        m_toggleApiKeyButton->setText("显示");
    }
}


void DeepSeekChatDialog::sendMessage()
{
    QString message = m_messageInput->text().trimmed(); // 获取用户输入并去除两端空格
    if (message.isEmpty()) {
        QMessageBox::warning(this, "发送消息", "消息不能为空。");
        return;
    }

    if (m_apiKey.isEmpty()) {
        QMessageBox::warning(this, "发送消息", "DeepSeek API 密钥未设置，请先设置密钥。");
        return;
    }

    appendMessage("你", message, true); // 在聊天显示区添加用户消息
    m_messageInput->clear(); // 清空输入框
    m_statusLabel->setText("正在发送消息...");

    QNetworkRequest request(m_apiUrl); // 创建网络请求
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); // 设置内容类型为 JSON
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8()); // 设置授权头

    QByteArray postData = buildRequestJson(message); // 构建 POST 请求的 JSON 数据

    m_networkManager->post(request, postData); // 发送 POST 请求
}

void DeepSeekChatDialog::handleNetworkReply(QNetworkReply *reply)
{
    m_statusLabel->setText("就绪"); // 重置状态标签

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll(); // 读取所有响应数据
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData); // 将数据解析为 JSON 文档

        if (jsonDoc.isObject()) {
            QJsonObject jsonObj = jsonDoc.object();
            // 检查是否有错误
            if (jsonObj.contains("error")) {
                QJsonObject errorObj = jsonObj["error"].toObject();
                QString errorMessage = errorObj["message"].toString();
                appendMessage("DeepSeek (错误)", errorMessage);
                qWarning() << "DeepSeek API Error:" << errorMessage;
            } else if (jsonObj.contains("choices") && jsonObj["choices"].isArray()) {
                QJsonArray choices = jsonObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject firstChoice = choices.first().toObject();
                    if (firstChoice.contains("message") && firstChoice["message"].isObject()) {
                        QJsonObject messageObj = firstChoice["message"].toObject();
                        if (messageObj.contains("content")) {
                            QString assistantResponse = messageObj["content"].toString();
                            appendMessage("DeepSeek AI", assistantResponse); // 在聊天显示区添加 AI 回复
                        }
                    }
                }
            } else {
                appendMessage("DeepSeek (解析错误)", "无法从响应中解析出有效内容。");
                qWarning() << "Failed to parse DeepSeek response:" << responseData;
            }
        } else {
            appendMessage("DeepSeek (JSON 错误)", "DeepSeek API 返回的不是有效的 JSON 对象。");
            qWarning() << "DeepSeek API response is not a JSON object:" << responseData;
        }
    } else {
        // 处理网络错误
        appendMessage("DeepSeek (网络错误)", reply->errorString());
        qWarning() << "Network Error:" << reply->errorString();
    }
    reply->deleteLater(); // 释放 QNetworkReply 对象
}

void DeepSeekChatDialog::appendMessage(const QString &sender, const QString &message, bool isUser)
{
    QString formattedMessage;
    if (isUser) {
        // 用户消息显示为蓝色，靠右对齐
        formattedMessage = QString("<div style='text-align: right; margin-bottom: 5px;'><b style='color: blue;'>%1:</b> %2</div>")
                               .arg(sender, message.toHtmlEscaped());
    } else {
        // AI 消息显示为绿色，靠左对齐
        formattedMessage = QString("<div style='text-align: left; margin-bottom: 5px;'><b style='color: green;'>%1:</b> %2</div>")
                               .arg(sender, message.toHtmlEscaped());
    }

    m_chatDisplay->append(formattedMessage); // 追加消息到文本框
    // 滚动到底部，确保最新消息可见
    QTextCursor cursor = m_chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_chatDisplay->setTextCursor(cursor);
}

QByteArray DeepSeekChatDialog::buildRequestJson(const QString &message)
{
    QJsonObject messageObj;
    messageObj["role"] = "user"; // 消息角色为用户
    messageObj["content"] = message; // 消息内容

    QJsonArray messagesArray;
    messagesArray.append(messageObj); // 将用户消息添加到消息数组

    QJsonObject requestObj;
    requestObj["model"] = "deepseek-chat"; // 指定 DeepSeek 聊天模型
    requestObj["messages"] = messagesArray;
    requestObj["temperature"] = 0.7; // 设置生成温度，控制回答的随机性

    QJsonDocument jsonDoc(requestObj);
    return jsonDoc.toJson(); // 返回 JSON 文档的字节数组形式
}
