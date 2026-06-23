#include "OpenAIClient.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>

OpenAIClient::OpenAIClient(QObject* parent) : QObject(parent) {
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &OpenAIClient::onReplyFinished);
}

void OpenAIClient::setApiKey(const QString& apiKey) {
    m_apiKey = apiKey;
}

void OpenAIClient::analyzeCode(const QString& code) {
    if (m_apiKey.isEmpty()) {
        emit analysisFailed("API Key is missing.");
        return;
    }

    QUrl url("https://api.groq.com/openai/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

    QJsonObject message;
    message["role"] = "user";
    message["content"] = QString(
        "You are an elite algorithm visualization architect specializing in cinematic, interactive, educational visualizations.\n\n"
        "Analyze the following Python algorithm and generate highly detailed JSON metadata for a modern visualization engine.\n\n"
        "The goal is to create visually impressive, intuitive, animation-rich algorithm visualizations similar to premium educational platforms.\n\n"
        "IMPORTANT VISUAL DESIGN RULES:\n"
        "- Avoid generic boxes unless semantically necessary.\n"
        "- Use meaningful visual metaphors based on the algorithm type.\n"
        "- Make animations smooth, modern, and aesthetically pleasing.\n"
        "- Prioritize educational clarity and visual hierarchy.\n"
        "- Active elements should feel alive using glow, pulse, scaling, or motion.\n"
        "- Sorting algorithms should feel dynamic and kinetic.\n"
        "- Graph algorithms should emphasize traversal flow and connectivity.\n"
        "- Recursive algorithms should emphasize depth and branching.\n"
        "- Tree algorithms should feel spatially balanced.\n"
        "- Pointer movement should be visually expressive.\n"
        "- The visualization should look premium, futuristic, and polished.\n\n"
        "VISUAL METAPHOR EXAMPLES:\n"
        "- sorting → animated vertical bars, flowing cards, orbiting blocks\n"
        "- recursion → expanding recursive tree\n"
        "- graph traversal → glowing connected network\n"
        "- linked list → flowing chain\n"
        "- stack/queue → layered container flow\n"
        "- dynamic programming → heatmap grid\n"
        "- heap → animated binary tree\n\n"
        "Return ONLY raw JSON.\n"
        "Do NOT include markdown.\n"
        "Do NOT include explanations.\n\n"
        "Required JSON schema:\n"
        "{\n"
        "  \"algorithm_class\": \"sorting|graph|tree|dp|recursion|searching|linked_list|greedy|generic\",\n"
        "\n"
        "  \"algorithm_description\": \"short human readable explanation\",\n"
        "\n"
        "  \"primary_structure\": {\n"
        "      \"name\": \"main structure variable name\",\n"
        "      \"type\": \"array|graph|tree|matrix|linked_list|stack|queue\"\n"
        "  },\n"
        "\n"
        "  \"pointers\": {\n"
        "      \"left\": \"\",\n"
        "      \"right\": \"\",\n"
        "      \"mid\": \"\",\n"
        "      \"current\": \"\",\n"
        "      \"i\": \"\",\n"
        "      \"j\": \"\",\n"
        "      \"pivot\": \"\",\n"
        "      \"low\": \"\",\n"
        "      \"high\": \"\"\n"
        "  },\n"
        "\n"
        "  \"visual_metaphor\": {\n"
        "      \"style\": \"tower-bars|flowing-cards|glowing-network|recursive-tree|heatmap-grid|particle-flow|glass-panels|cyber-grid\",\n"
        "      \"reason\": \"why this metaphor fits the algorithm\"\n"
        "  },\n"
        "\n"
        "  \"visualization_style\": {\n"
        "      \"layout\": \"horizontal|vertical|radial|tree|force-directed|grid\",\n"
        "      \"element_shape\": \"rounded-rect|glass-card|circle|pill|node\",\n"
        "      \"theme_mode\": \"dark|neon|glassmorphism|cyberpunk|minimal\",\n"
        "      \"spacing\": \"compact|comfortable|wide\",\n"
        "      \"depth_effect\": true,\n"
        "      \"glow_active\": true,\n"
        "      \"show_indices\": true,\n"
        "      \"show_values\": true,\n"
        "      \"show_pointer_labels\": true,\n"
        "      \"background_style\": \"gradient|grid|particles|minimal\"\n"
        "  },\n"
        "\n"
        "  \"animations\": {\n"
        "      \"comparison\": \"glow-pulse|bounce|flash|wave\",\n"
        "      \"swap\": \"arc-swap|smooth-slide|crossfade|teleport\",\n"
        "      \"pointer_move\": \"smooth-follow|beam-track|elastic\",\n"
        "      \"completion\": \"cascade-glow|success-wave|confetti|fade-settle\",\n"
        "      \"transition_speed\": \"slow|medium|fast\"\n"
        "  },\n"
        "\n"
        "  \"camera_behavior\": {\n"
        "      \"focus_tracking\": true,\n"
        "      \"zoom_on_active\": true,\n"
        "      \"pan_strategy\": \"follow-active|static|smooth-center\"\n"
        "  },\n"
        "\n"
        "  \"semantic_roles\": {\n"
        "      \"comparators\": [],\n"
        "      \"iterators\": [],\n"
        "      \"boundaries\": [],\n"
        "      \"temporaries\": []\n"
        "  },\n"
        "\n"
        "  \"theme\": {\n"
        "      \"primary\": \"#\",\n"
        "      \"secondary\": \"#\",\n"
        "      \"highlight\": \"#\",\n"
        "      \"active\": \"#\",\n"
        "      \"sorted\": \"#\",\n"
        "      \"background\": \"#\",\n"
        "      \"text\": \"#\"\n"
        "  }\n"
        "}\n\n"
        "Choose visualization decisions that maximize beauty, motion quality, semantic clarity, and educational value.\n\n"
        "Code:\n"
    ) + code;

    QJsonArray messages;
    messages.append(message);

    QJsonObject body;
    body["model"] = "llama-3.3-70b-versatile";
    body["messages"] = messages;
    body["temperature"] = 0.1;

    m_manager->post(request, QJsonDocument(body).toJson());
}

void OpenAIClient::onReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit analysisFailed(reply->errorString() + ": " + reply->readAll());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject responseObj = doc.object();

    QJsonArray choices = responseObj["choices"].toArray();
    if (choices.isEmpty()) {
        emit analysisFailed("No choices returned from OpenAI.");
        reply->deleteLater();
        return;
    }

    QString content = choices[0].toObject()["message"].toObject()["content"].toString().trimmed();
    
    // Sometimes the model might still return markdown backticks
    if (content.startsWith("```json")) {
        content = content.mid(7);
        if (content.endsWith("```")) {
            content.chop(3);
        }
    } else if (content.startsWith("```")) {
        content = content.mid(3);
        if (content.endsWith("```")) {
            content.chop(3);
        }
    }
    
    QJsonParseError parseError;
    QJsonDocument metadataDoc = QJsonDocument::fromJson(content.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit analysisFailed("Failed to parse JSON from AI: " + parseError.errorString() + "\nRaw:\n" + content);
        reply->deleteLater();
        return;
    }

    emit analysisComplete(metadataDoc.object());
    reply->deleteLater();
}
