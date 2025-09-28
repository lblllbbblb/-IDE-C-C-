#include "DeepSeekChat.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog> // 用于获取 API 密钥
#include <QDebug> // 用于调试输出
#include <QFile>    // 用于文件操作
#include <QFileDialog> // 用于文件对话框
#include <QTextStream> // 用于读写文件
#include <QFrame> // 用于模拟分隔线

DeepSeekChatDialog::DeepSeekChatDialog(
    QWidget *parent,
    const QString &currentFilePath,
    const QString &editorContent,
    const QString &selectedCode,
    int cursorLine,
    int cursorCol,
    int selectionStartLine,
    int selectionEndLine
    )
    : QDialog(parent),
    m_networkManager(new QNetworkAccessManager(this)), // 初始化网络访问管理器
    m_apiUrl("https://api.deepseek.com/chat/completions"), // DeepSeek API 的聊天完成接口 URL
    m_currentFilePath(currentFilePath),
    m_editorContent(editorContent),
    m_selectedCode(selectedCode),
    m_cursorLine(cursorLine),
    m_cursorCol(cursorCol),
    m_selectionStartLine(selectionStartLine),
    m_selectionEndLine(selectionEndLine)
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

    // 新增 UI 元素
    m_fileContentDisplay = new QTextEdit(this);
    m_fileContentDisplay->setReadOnly(true);
    m_fileContentDisplay->setPlaceholderText("当前文件内容或加载的文件内容将显示在此处...");
    m_fileContentDisplay->setStyleSheet("background-color: #e0e0e0; border: 1px solid #aaa; padding: 5px;");
    // 显示当前编辑器的内容
    if (!m_editorContent.isEmpty()) {
        m_fileContentDisplay->setPlainText(m_editorContent);
    } else if (!m_currentFilePath.isEmpty()) {
        m_fileContentDisplay->setPlainText(QString("文件路径: %1\n\n文件内容加载失败或为空。").arg(m_currentFilePath));
    } else {
        m_fileContentDisplay->setPlainText("没有当前打开的文件内容。");
    }


    m_responseCodeDisplay = new QTextEdit(this);
    m_responseCodeDisplay->setReadOnly(true);
    m_responseCodeDisplay->setPlaceholderText("AI 生成的代码片段将显示在此处...");
    m_responseCodeDisplay->setStyleSheet("background-color: #d0d0d0; border: 1px solid #888; padding: 5px;");

    m_insertCodeButton = new QPushButton("插入代码到编辑器", this);
    m_insertCodeButton->setStyleSheet("background-color: #28a745; color: white; padding: 8px 15px; border-radius: 5px;");
    m_insertCodeButton->setEnabled(false); // 初始禁用，只有当有代码生成时才启用

    m_saveFileButton = new QPushButton("保存为新文件", this);
    m_saveFileButton->setStyleSheet("background-color: #ffc107; color: black; padding: 8px 15px; border-radius: 5px;");
    m_saveFileButton->setEnabled(false); // 初始禁用

    m_fileOperationsComboBox = new QComboBox(this);
    m_fileOperationsComboBox->addItem("文件操作...");
    m_fileOperationsComboBox->addItem("加载本地文件...");
    m_fileOperationsComboBox->addItem("保存 AI 生成内容为...");
    m_fileOperationsComboBox->setCurrentIndex(0); // 默认选中提示项

    // 布局管理
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *inputLayout = new QHBoxLayout();
    QHBoxLayout *apiKeyLayout = new QHBoxLayout(); // API 密钥布局
    QHBoxLayout *responseControlsLayout = new QHBoxLayout(); // 响应代码控制布局
    // QHBoxLayout *fileControlsLayout = new QHBoxLayout(); // 文件操作控制布局 // 这一行似乎是多余的，因为文件操作ComboBox已经放在responseControlsLayout里了

    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);

    apiKeyLayout->addWidget(m_apiKeyInput);
    apiKeyLayout->addWidget(m_toggleApiKeyButton);
    apiKeyLayout->addWidget(m_setApiKeyButton); // 将设置API密钥按钮也放在这一行

    responseControlsLayout->addWidget(m_insertCodeButton);
    responseControlsLayout->addWidget(m_saveFileButton);
    responseControlsLayout->addWidget(m_fileOperationsComboBox);

    mainLayout->addWidget(m_chatDisplay);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(apiKeyLayout);
    mainLayout->addWidget(m_statusLabel); // 添加状态标签

    // 使用 QFrame 模拟分隔线
    QFrame *separator1 = new QFrame(this);
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator1);

    mainLayout->addWidget(new QLabel("当前文件/加载的文件内容:", this));
    mainLayout->addWidget(m_fileContentDisplay);

    // 使用 QFrame 模拟分隔线
    QFrame *separator2 = new QFrame(this);
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator2);

    mainLayout->addWidget(new QLabel("AI 生成的代码/文件内容:", this));
    mainLayout->addWidget(m_responseCodeDisplay);
    mainLayout->addLayout(responseControlsLayout);


    // 连接信号和槽
    connect(m_sendButton, &QPushButton::clicked, this, &DeepSeekChatDialog::sendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &DeepSeekChatDialog::sendMessage); // 按回车键发送
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &DeepSeekChatDialog::handleNetworkReply);
    connect(m_setApiKeyButton, &QPushButton::clicked, this, &DeepSeekChatDialog::setApiKey);
    connect(m_toggleApiKeyButton, &QPushButton::toggled, this, &DeepSeekChatDialog::toggleApiKeyVisibility);

    // 文件操作和代码插入的连接
    connect(m_insertCodeButton, &QPushButton::clicked, this, &DeepSeekChatDialog::insertCodeFromResponse);
    connect(m_saveFileButton, &QPushButton::clicked, this, &DeepSeekChatDialog::saveFileContent);
    connect(m_fileOperationsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DeepSeekChatDialog::handleFileOperation);


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
                m_insertCodeButton->setEnabled(false);
                m_saveFileButton->setEnabled(false);
            } else if (jsonObj.contains("choices") && jsonObj["choices"].isArray()) {
                QJsonArray choices = jsonObj["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject firstChoice = choices.first().toObject();
                    if (firstChoice.contains("message") && firstChoice["message"].isObject()) {
                        QJsonObject messageObj = firstChoice["message"].toObject();
                        if (messageObj.contains("content")) {
                            QString assistantResponse = messageObj["content"].toString();
                            appendMessage("DeepSeek AI", assistantResponse); // 在聊天显示区添加 AI 回复

                            // 提取代码片段并显示在 m_responseCodeDisplay，使用 QRegularExpression 替代 QRegExp
                            QRegularExpression codeBlockRegex("```[a-zA-Z]*\\n([\\s\\S]*?)\\n```");
                            QRegularExpressionMatch match = codeBlockRegex.match(assistantResponse);

                            if (match.hasMatch()) { // 检查是否有匹配
                                QString code = match.captured(1).trimmed(); // 捕获第一个括号内的内容
                                m_responseCodeDisplay->setPlainText(code);
                                m_insertCodeButton->setEnabled(true);
                                m_saveFileButton->setEnabled(true);
                            } else {
                                // 如果没有代码块，显示整个响应
                                m_responseCodeDisplay->setPlainText(assistantResponse);
                                m_insertCodeButton->setEnabled(true); // 即使不是代码块，也允许插入
                                m_saveFileButton->setEnabled(true);
                            }
                        }
                    }
                }
            } else {
                appendMessage("DeepSeek (解析错误)", "无法从响应中解析出有效内容。");
                qWarning() << "Failed to parse DeepSeek response:" << responseData;
                m_insertCodeButton->setEnabled(false);
                m_saveFileButton->setEnabled(false);
            }
        } else {
            appendMessage("DeepSeek (JSON 错误)", "DeepSeek API 返回的不是有效的 JSON 对象。");
            qWarning() << "DeepSeek API response is not a JSON object:" << responseData;
            m_insertCodeButton->setEnabled(false);
            m_saveFileButton->setEnabled(false);
        }
    } else {
        // 处理网络错误
        appendMessage("DeepSeek (网络错误)", reply->errorString());
        qWarning() << "Network Error:" << reply->errorString();
        m_insertCodeButton->setEnabled(false);
        m_saveFileButton->setEnabled(false);
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

QByteArray DeepSeekChatDialog::buildRequestJson(const QString &userMessage)
{
    QJsonArray messagesArray;

    // 添加系统消息，提供上下文信息
    QString contextMessage = "你是一个C/C++编程助手。";
    if (!m_currentFilePath.isEmpty()) {
        contextMessage += QString("当前文件路径: %1. ").arg(m_currentFilePath);
    }
    if (!m_editorContent.isEmpty()) {
        contextMessage += "当前文件内容:\n```cpp\n" + m_editorContent + "\n```\n";
    }
    if (!m_selectedCode.isEmpty()) {
        contextMessage += QString("当前选中代码 (行 %1-%2):\n```cpp\n%3\n```\n")
                              .arg(m_selectionStartLine).arg(m_selectionEndLine).arg(m_selectedCode);
    }
    if (m_cursorLine != -1) {
        contextMessage += QString("当前光标位置: 行 %1, 列 %2. ").arg(m_cursorLine).arg(m_cursorCol);
    }
    contextMessage += "请根据用户的问题和提供的上下文进行回答，如果涉及代码，请使用Markdown代码块格式返回。";


    QJsonObject systemMessageObj;
    systemMessageObj["role"] = "system";
    systemMessageObj["content"] = contextMessage;
    messagesArray.append(systemMessageObj);

    // 添加用户消息
    QJsonObject userMessageObj;
    userMessageObj["role"] = "user"; // 消息角色为用户
    userMessageObj["content"] = userMessage; // 消息内容
    messagesArray.append(userMessageObj);

    QJsonObject requestObj;
    requestObj["model"] = "deepseek-chat"; // 指定 DeepSeek 聊天模型
    requestObj["messages"] = messagesArray;
    requestObj["temperature"] = 0.7; // 设置生成温度，控制回答的随机性
    requestObj["stream"] = false; // 不使用流式传输，一次性获取完整响应

    QJsonDocument jsonDoc(requestObj);
    return jsonDoc.toJson(); // 返回 JSON 文档的字节数组形式
}

// 处理文件操作下拉菜单的选择
void DeepSeekChatDialog::handleFileOperation(int index)
{
    switch (index) {
    case 1: // 加载本地文件...
        loadFileIntoChat();
        break;
    case 2: // 保存 AI 生成内容为...
        saveFileContent();
        break;
    default:
        break;
    }
    // 重置下拉菜单到默认项，以便再次选择
    m_fileOperationsComboBox->setCurrentIndex(0);
}

// 将 DeepSeek 生成的代码插入到编辑器
void DeepSeekChatDialog::insertCodeFromResponse()
{
    QString codeToInsert = m_responseCodeDisplay->toPlainText();
    if (codeToInsert.isEmpty()) {
        QMessageBox::warning(this, "插入代码", "没有 AI 生成的代码可供插入。");
        return;
    }

    // 发射信号给 My_IDE 主窗口，让它处理代码插入
    // 如果有选中代码，则替换选中部分；否则在光标处插入
    if (!m_selectedCode.isEmpty() && m_selectionStartLine != -1 && m_selectionEndLine != -1) {
        emit insertCodeRequested(codeToInsert, m_selectionStartLine, m_selectionEndLine);
    } else if (m_cursorLine != -1) {
        // 如果没有选中，但在已知光标位置，可以在光标行插入
        emit insertCodeRequested(codeToInsert, m_cursorLine, m_cursorLine); // 简单起见，替换光标所在行
    } else {
        // 如果什么信息都没有，就让主窗口在当前光标处插入
        emit insertCodeRequested(codeToInsert, -1, -1);
    }

    QMessageBox::information(this, "插入代码", "代码插入请求已发送到主编辑器。");
}

// 将 DeepSeek 生成的文件内容保存为新文件
void DeepSeekChatDialog::saveFileContent()
{
    QString contentToSave = m_responseCodeDisplay->toPlainText();
    if (contentToSave.isEmpty()) {
        QMessageBox::warning(this, "保存文件", "没有 AI 生成的内容可供保存。");
        return;
    }

    QString defaultFileName = "untitled.txt";
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        defaultFileName = fileInfo.fileName();
        if (defaultFileName.contains('.')) {
            // 尝试保留扩展名，或建议新的
            defaultFileName.insert(defaultFileName.lastIndexOf('.'), "_ai_generated");
        } else {
            defaultFileName += "_ai_generated.txt";
        }
    }


    QString filePath = QFileDialog::getSaveFileName(this, "保存 AI 生成内容",
                                                    QDir::currentPath() + "/" + defaultFileName,
                                                    "All Files (*);;Text Files (*.txt);;C/C++ Files (*.c *.cpp *.h)");

    if (filePath.isEmpty()) {
        return; // 用户取消
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << contentToSave;
        file.close();
        QMessageBox::information(this, "保存成功", QString("AI 生成内容已保存到:\n%1").arg(filePath));
    } else {
        QMessageBox::critical(this, "保存失败", QString("无法保存文件:\n%1\n错误: %2").arg(filePath, file.errorString()));
    }
}

// 加载本地文件内容到聊天上下文
void DeepSeekChatDialog::loadFileIntoChat()
{
    QString filePath = QFileDialog::getOpenFileName(this, "加载本地文件", QDir::currentPath(),
                                                    "All Files (*);;Text Files (*.txt);;C/C++ Files (*.c *.cpp *.h)");

    if (filePath.isEmpty()) {
        return; // 用户取消
    }

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString fileContent = in.readAll();
        file.close();

        m_fileContentDisplay->setPlainText(fileContent); // 在对话框中显示加载的文件内容
        m_editorContent = fileContent; // 更新上下文，以便下次发送消息时包含
        m_currentFilePath = filePath; // 更新文件路径上下文
        m_selectedCode = ""; // 清除选中代码，因为是加载整个文件
        m_selectionStartLine = -1;
        m_selectionEndLine = -1;

        QMessageBox::information(this, "文件加载成功", QString("文件 '%1' 已加载到 AI 助手上下文。").arg(QFileInfo(filePath).fileName()));
    } else {
        QMessageBox::critical(this, "文件加载失败", QString("无法打开文件:\n%1\n错误: %2").arg(filePath, file.errorString()));
    }
}
