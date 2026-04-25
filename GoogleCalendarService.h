#ifndef GOOGLECALENDARSERVICE_H
#define GOOGLECALENDARSERVICE_H

/**
 * @file GoogleCalendarService.h
 * @brief OAuth 2.0 **flux Application de bureau (Desktop)** + Google Calendar API.
 *
 * Conforme au modèle Google « Desktop app » : client téléchargé (`client_secret.json` avec
 * section \c installed), **redirect loopback** sur localhost, navigateur système pour le login.
 *
 * **Technologies Qt utilisées**
 * - \c QDesktopServices::openUrl() — ouverture du navigateur vers la page d’autorisation Google.
 * - \c QNetworkAccessManager — échange du code contre les jetons (POST token) et appels Calendar (GET/POST).
 * - \c QTcpServer / \c QTcpSocket — réception du \c authorization_code sur loopback
 *   \c http://127.0.0.1:<port>/ (port **éphémère**, \c listen(..., 0) — flux Desktop / RFC 8252).
 *
 * **Flux (résumé)**
 * 1. Lire \c client_secret.json (prioritaire) ou \c google_credentials.json — extraire \c client_id,
 *    \c client_secret, \c auth_uri, \c token_uri depuis \c installed ou \c web.
 * 2. Démarrer le serveur loopback (port choisi par l’OS), construire \c redirect_uri identique pour l’auth et le token.
 * 3. Ouvrir le navigateur (\c QDesktopServices::openUrl) — URL OAuth avec \c response_type=code, \c scope, \c state.
 * 4. Google redirige vers \c http://127.0.0.1:<port>/?code=...&state=... — lecture HTTP minimale sur le socket.
 * 5. \c QNetworkAccessManager : POST \c application/x-www-form-urlencoded vers \c token_uri (échange code → jetons).
 * 6. Sauvegarde locale \c tokens.json (config utilisateur ; voir le .cpp).
 * 7. \c syncCalendar() : GET Calendar avec \c Authorization: Bearer ; signal \a eventsReady → UI Qt.
 *
 * Scope : \c https://www.googleapis.com/auth/calendar.events
 *
 * Client Google Cloud : **Application de bureau** — pas d’URI de redirection fixe à saisir à la main
 * pour le flux loopback à port dynamique (comportement attendu pour ce type de client).
 */

#include <QDate>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>

class QTcpServer;
class QTcpSocket;

/// Ligne pour affichage dans une QListWidget (texte + couleur chargé/libre).
struct GoogleCalendarEventRow
{
    QString displayText;
    bool isBusy = true;
    /// Jour local de début (pour QCalendarWidget) ; invalide si l’API ne fournit pas de date exploitable.
    QDate startDate;
};

class GoogleCalendarService : public QObject
{
    Q_OBJECT

public:
    explicit GoogleCalendarService(QObject *parent = nullptr);

    /// Flux Desktop : \c QDesktopServices + localhost, puis échange code → jetons (\c QNetworkAccessManager).
    void connectToGoogle();

    /// Rafraîchit le token si besoin, puis \c GET sur l’API Calendar (calendrier \c primary).
    /// La liste d’événements est renvoyée de façon asynchrone via le signal \a eventsReady
    /// (modèle Qt : QNetworkAccessManager ne bloque pas le thread UI).
    void syncCalendar();

    /**
     * Crée un événement sur le calendrier « primary ».
     * @param summary Titre visible dans Google Calendar
     * @param startUtc début (UTC recommandé ; transmis en ISO8601 avec Z)
     * @param endUtc fin (doit être > début)
     * @param description texte optionnel
     */
    void createEvent(const QString &summary, const QDateTime &startUtc, const QDateTime &endUtc,
                     const QString &description = QString());

signals:
    void authenticationFinished(bool success, const QString &message);
    void eventsReady(const QVector<GoogleCalendarEventRow> &rows);
    void eventCreated(bool success, const QString &messageOrEventId);
    void errorOccurred(const QString &message);

private slots:
    void onTokenReplyFinished();
    void onEventsReplyFinished();
    void onCreateEventReplyFinished();
    void onRedirectServerNewConnection();

private:
    bool loadCredentials(QString *errorMessage);
    QString resolveCredentialsJsonPath() const;
    void saveTokensFromJson(const QJsonObject &obj);
    bool loadStoredTokens();
    bool accessTokenNeedsRefresh() const;
    void exchangeCodeForTokens(const QString &code, const QString &redirectUri);
    void requestEventsWithAccessToken();
    void closeRedirectServer();
    bool pickLoopbackPort(QString *redirectUriOut, QString *errorMessage);
    void parseEventsJson(const QByteArray &body, QVector<GoogleCalendarEventRow> *out) const;

    QNetworkAccessManager m_nam;
    QPointer<QTcpServer> m_redirectServer;
    /// True once the OAuth redirect has been handled (success path started or OAuth error response processed).
    bool m_oauthLoopbackHandled = false;

    QString m_clientId;
    QString m_clientSecret;
    QString m_tokenUri;
    QString m_authUri;
    QString m_redirectUri;

    QString m_accessToken;
    QString m_refreshToken;
    qint64 m_accessTokenExpirySecs = 0;

    QString m_oauthState;
};

#endif // GOOGLECALENDARSERVICE_H
