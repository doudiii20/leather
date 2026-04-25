#ifndef FOURNISSEURAPISERVICE_H
#define FOURNISSEURAPISERVICE_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class FournisseurApiService : public QObject
{
    Q_OBJECT
public:
    explicit FournisseurApiService(QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    void requestPerformanceAnalystApi(const QString &supplierContext);
    void requestRiskContinuityApi(const QString &supplierContext);

signals:
    void apiReplyReady(const QString &apiName, const QString &content);

private slots:
    void onReplyFinished();

private:
    enum class ApiKind { PerformanceAnalyst, RiskContinuity };
    void requestApi(ApiKind kind, const QString &supplierContext);
    QString promptForKind(ApiKind kind) const;
    QString apiName(ApiKind kind) const;

    QNetworkAccessManager *m_nam = nullptr;
};

#endif // FOURNISSEURAPISERVICE_H
