#ifndef OPENAICLIENT_H
#define OPENAICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QString>

class OpenAIClient : public QObject {
    Q_OBJECT
public:
    explicit OpenAIClient(QObject* parent = nullptr);
    void setApiKey(const QString& apiKey);
    void analyzeCode(const QString& code);

signals:
    void analysisComplete(const QJsonObject& metadata);
    void analysisFailed(const QString& errorString);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_manager;
    QString m_apiKey;
};

#endif // OPENAICLIENT_H
