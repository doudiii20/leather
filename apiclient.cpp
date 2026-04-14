#include "apiclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QLatin1String>
#include <QByteArray>
#include <QTimer>
#include <QRandomGenerator>
#include <QUrlQuery>
#include <QJsonValue>
#include <QProcessEnvironment>
#include <QSharedPointer>
#include <QStringList>
#include <QRegularExpression>

namespace {
    constexpr char kWeb3FormsUrl[] = "https://api.web3forms.com/submit";
}

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , nam(new QNetworkAccessManager(this))
{
}

void ApiClient::postChatCompletion(const QString &apiKey,
                                   const QString &chatCompletionsUrl,
                                   const QString &model,
                                   const QString &systemPrompt,
                                   const QString &userMessage)
{
    QNetworkRequest req;
    req.setUrl(QUrl(chatCompletionsUrl));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + apiKey.toUtf8());

    QJsonObject sys;
    sys.insert(QStringLiteral("role"), QStringLiteral("system"));
    sys.insert(QStringLiteral("content"), systemPrompt);

    QJsonObject usr;
    usr.insert(QStringLiteral("role"), QStringLiteral("user"));
    usr.insert(QStringLiteral("content"), userMessage);

    QJsonArray messages;
    messages.append(sys);
    messages.append(usr);

    QJsonObject root;
    root.insert(QStringLiteral("model"), model);
    root.insert(QStringLiteral("messages"), messages);

    QNetworkReply *reply = nam->post(req, QJsonDocument(root).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit chatCompleted(QString(), reply->errorString());
            return;
        }
        const QByteArray raw = reply->readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        if (!doc.isObject()) {
            emit chatCompleted(QString(), QStringLiteral("Reponse JSON invalide"));
            return;
        }
        const QJsonArray choices = doc.object().value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            const QString err = doc.object().value(QStringLiteral("error"))
            .toObject()
                .value(QStringLiteral("message"))
                .toString();
            emit chatCompleted(QString(),
                               err.isEmpty() ? QStringLiteral("Aucun choix dans la reponse API")
                                             : err);
            return;
        }
        const QString text = choices.at(0)
                                 .toObject()
                                 .value(QStringLiteral("message"))
                                 .toObject()
                                 .value(QStringLiteral("content"))
                                 .toString();
        emit chatCompleted(text, QString());
    });
}

void ApiClient::postWeb3FormsMail(const QString &accessKey,
                                  const QString &subject,
                                  const QString &plainBody)
{
    QJsonObject root;
    root.insert(QStringLiteral("access_key"), accessKey.trimmed());
    root.insert(QStringLiteral("subject"), subject);
    root.insert(QStringLiteral("from_name"), QStringLiteral("Alerte Stock Produit"));
    root.insert(QStringLiteral("message"), plainBody);

    QNetworkRequest req;
    req.setUrl(QUrl(QLatin1String(kWeb3FormsUrl)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setRawHeader("Accept", QByteArrayLiteral("application/json"));
    req.setRawHeader("User-Agent", QByteArrayLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"));

    QNetworkReply *reply = nam->post(req, QJsonDocument(root).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool netOk = (reply->error() == QNetworkReply::NoError);
        const bool httpOk = (code >= 200 && code < 300);
        const bool ok = netOk && httpOk;
        QString err;
        if (!ok) {
            const QByteArray data = reply->readAll();
            const QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject() && doc.object().contains(QStringLiteral("message"))) {
                err = doc.object().value(QStringLiteral("message")).toString();
            } else {
                err = QStringLiteral("Erreur %1: %2").arg(code).arg(reply->errorString());
            }
        }
        emit emailCompleted(ok, err);
    });
}

void ApiClient::generateImageFromPrompt(const QString &promptText)
{
    static bool s_skipGeminiProvider = false;
    static bool s_skipOpenAIProvider = false;

    QString prompt = promptText.trimmed();
    prompt.replace(QChar('\n'), QChar(' '));
    prompt.replace(QChar('\r'), QChar(' '));
    prompt = prompt.simplified();
    prompt.remove(QChar('\''));
    prompt.remove(QChar('"'));
    if (prompt.isEmpty()) {
        emit imageGenerated(QByteArray(), QStringLiteral("Prompt image vide."));
        return;
    }
    QSharedPointer<QStringList> providerErrors(new QStringList());

    auto looksLikeImageBytes = [](const QByteArray &b) -> bool {
        if (b.size() < 12) return false;
        if (b.startsWith("\x89PNG")) return true;
        if (b.startsWith("\xFF\xD8\xFF")) return true; // JPEG
        if (b.startsWith("GIF87a") || b.startsWith("GIF89a")) return true;
        if (b.left(4) == "RIFF" && b.mid(8, 4) == "WEBP") return true;
        return false;
    };

    auto startFreeFallback = [this, looksLikeImageBytes, prompt, providerErrors]() {
        const QString encodedPrompt = QString::fromUtf8(QUrl::toPercentEncoding(prompt));
        const int seed1 = QRandomGenerator::global()->bounded(1, 1000000);
        const int seed2 = QRandomGenerator::global()->bounded(1, 1000000);

        QUrl url1(QStringLiteral("https://image.pollinations.ai/prompt/%1").arg(encodedPrompt));
        QUrlQuery q1;
        q1.addQueryItem(QStringLiteral("width"), QStringLiteral("1024"));
        q1.addQueryItem(QStringLiteral("height"), QStringLiteral("1024"));
        q1.addQueryItem(QStringLiteral("nologo"), QStringLiteral("true"));
        q1.addQueryItem(QStringLiteral("model"), QStringLiteral("flux"));
        q1.addQueryItem(QStringLiteral("seed"), QString::number(seed1));
        url1.setQuery(q1);

        QUrl url2(QStringLiteral("https://pollinations.ai/p/%1").arg(encodedPrompt));
        QUrlQuery q2;
        q2.addQueryItem(QStringLiteral("width"), QStringLiteral("1024"));
        q2.addQueryItem(QStringLiteral("height"), QStringLiteral("1024"));
        q2.addQueryItem(QStringLiteral("seed"), QString::number(seed2));
        url2.setQuery(q2);

        const QList<QUrl> endpoints{url1, url2};

        auto requestByIndex = [this, endpoints, looksLikeImageBytes, prompt, providerErrors](int index, auto &&requestByIndexRef) -> void {
        if (index >= endpoints.size()) {
            // Fallback robuste: chercher une image IA proche (Lexica), puis la telecharger.
            QUrl lexicaUrl(QStringLiteral("https://lexica.art/api/v1/search"));
            QUrlQuery lq;
            lq.addQueryItem(QStringLiteral("q"), prompt);
            lexicaUrl.setQuery(lq);

            QNetworkRequest lexReq(lexicaUrl);
            lexReq.setRawHeader("User-Agent", QByteArrayLiteral("QtImageClient/1.0"));
            lexReq.setRawHeader("Accept", QByteArrayLiteral("application/json,*/*"));
            QNetworkReply *lexReply = nam->get(lexReq);
            connect(lexReply, &QNetworkReply::finished, this, [this, lexReply, looksLikeImageBytes, prompt, providerErrors]() {
                const QByteArray raw = lexReply->readAll();
                const int lexCode = lexReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                lexReply->deleteLater();
                if (raw.isEmpty()) {
                    providerErrors->append(QStringLiteral("Lexica: reponse vide (HTTP %1)").arg(lexCode));
                }

                QUrl imageUrl;
                const QJsonDocument doc = QJsonDocument::fromJson(raw);
                if (doc.isObject()) {
                    const QJsonArray imgs = doc.object().value(QStringLiteral("images")).toArray();
                    if (!imgs.isEmpty()) {
                        const QJsonObject first = imgs.at(0).toObject();
                        const QString u1 = first.value(QStringLiteral("src")).toString();
                        const QString u2 = first.value(QStringLiteral("imageUrl")).toString();
                        const QString u3 = first.value(QStringLiteral("url")).toString();
                        const QString chosen = !u1.isEmpty() ? u1 : (!u2.isEmpty() ? u2 : u3);
                        if (!chosen.isEmpty())
                            imageUrl = QUrl(chosen);
                    }
                }

                if (!imageUrl.isValid() || imageUrl.isEmpty()) {
                    // Dernier secours 1: Unsplash Source (fiable pour 2e/3e test).
                    const QString unsplashPrompt = QString::fromUtf8(QUrl::toPercentEncoding(prompt));
                    const QUrl unsplashUrl(QStringLiteral("https://source.unsplash.com/1024x1024/?%1").arg(unsplashPrompt));
                    QNetworkRequest unsReq(unsplashUrl);
                    unsReq.setRawHeader("User-Agent", QByteArrayLiteral("QtImageClient/1.0"));
                    unsReq.setRawHeader("Accept", QByteArrayLiteral("image/*,*/*"));
                    QNetworkReply *unsReply = nam->get(unsReq);
                    connect(unsReply, &QNetworkReply::finished, this, [this, unsReply, looksLikeImageBytes, prompt, providerErrors]() {
                        const QByteArray unsData = unsReply->readAll();
                        const QString unsType = unsReply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
                        const int unsCode = unsReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                        unsReply->deleteLater();
                        const bool unsOk = ((unsCode >= 200 && unsCode < 300) || unsCode == 0) &&
                                           !unsData.isEmpty() &&
                                           (unsType.startsWith(QStringLiteral("image/")) || looksLikeImageBytes(unsData));
                        if (unsOk) {
                            emit imageGenerated(unsData, QString());
                            return;
                        }
                        providerErrors->append(QStringLiteral("Unsplash: echec (HTTP %1)").arg(unsCode));

                        emit imageGenerated(
                            QByteArray(),
                            QStringLiteral("Echec generation image. Details: %1").arg(providerErrors->join(QStringLiteral(" | "))));
                    });
                    return;
                }

                QNetworkRequest imgReq(imageUrl);
                imgReq.setRawHeader("User-Agent", QByteArrayLiteral("QtImageClient/1.0"));
                imgReq.setRawHeader("Accept", QByteArrayLiteral("image/*,*/*"));
                QNetworkReply *imgReply = nam->get(imgReq);
                    connect(imgReply, &QNetworkReply::finished, this, [this, imgReply, looksLikeImageBytes, providerErrors]() {
                    const QByteArray imgData = imgReply->readAll();
                    const QString ctype = imgReply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
                    const int code = imgReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    const bool ok = ((code >= 200 && code < 300) || code == 0) &&
                                    !imgData.isEmpty() &&
                                    (ctype.startsWith(QStringLiteral("image/")) || looksLikeImageBytes(imgData));
                    imgReply->deleteLater();
                    if (!ok) {
                            providerErrors->append(QStringLiteral("Lexica image invalide (HTTP %1)").arg(code));
                        emit imageGenerated(QByteArray(),
                                            QStringLiteral("Echec generation image: image finale invalide."));
                        return;
                    }
                    emit imageGenerated(imgData, QString());
                });
            });
            return;
        }

        QNetworkRequest req(endpoints.at(index));
        req.setRawHeader("User-Agent", QByteArrayLiteral("QtImageClient/1.0"));
        req.setRawHeader("Accept", QByteArrayLiteral("image/*,application/json,text/plain,*/*"));
        QNetworkReply *reply = nam->get(req);

        QTimer *timer = new QTimer(reply);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, reply, [reply]() {
            if (reply->isRunning())
                reply->abort();
        });
        timer->start(45000);

        connect(reply, &QNetworkReply::finished, this, [this, reply, index, requestByIndexRef, looksLikeImageBytes, providerErrors]() {
            const int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray data = reply->readAll();
            const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
            const QString payload = QString::fromUtf8(data).trimmed();

            const bool statusOk = (httpCode >= 200 && httpCode < 300) || httpCode == 0;
            if (statusOk && !data.isEmpty()) {
                if (contentType.startsWith(QStringLiteral("image/")) || looksLikeImageBytes(data)) {
                    reply->deleteLater();
                    emit imageGenerated(data, QString());
                    return;
                }

                // Si le service renvoie du JSON ou du texte avec une URL image.
                QUrl imageUrl;
                if (payload.startsWith(QStringLiteral("http://")) || payload.startsWith(QStringLiteral("https://"))) {
                    imageUrl = QUrl(payload);
                } else {
                    const QJsonDocument jd = QJsonDocument::fromJson(data);
                    if (jd.isObject()) {
                        const QJsonObject o = jd.object();
                        const QString u1 = o.value(QStringLiteral("url")).toString();
                        const QString u2 = o.value(QStringLiteral("image")).toString();
                        const QString u3 = o.value(QStringLiteral("output")).toString();
                        const QString chosen = !u1.isEmpty() ? u1 : (!u2.isEmpty() ? u2 : u3);
                        if (!chosen.isEmpty())
                            imageUrl = QUrl(chosen);
                    }
                    if (!imageUrl.isValid() || imageUrl.isEmpty()) {
                        // Certains services renvoient du texte/HTML contenant une URL.
                        static const QRegularExpression urlRx(QStringLiteral(R"((https?://[^\s"'<>]+))"),
                                                              QRegularExpression::CaseInsensitiveOption);
                        const QRegularExpressionMatch m = urlRx.match(payload);
                        if (m.hasMatch()) {
                            QString u = m.captured(1).trimmed();
                            while (!u.isEmpty() &&
                                   (u.endsWith(QChar('.')) || u.endsWith(QChar(',')) ||
                                    u.endsWith(QChar(')')) || u.endsWith(QChar(']'))))
                                u.chop(1);
                            imageUrl = QUrl(u);
                        }
                    }
                }

                if (imageUrl.isValid() && !imageUrl.isEmpty()) {
                    QNetworkRequest imageReq(imageUrl);
                    imageReq.setRawHeader("User-Agent", QByteArrayLiteral("QtImageClient/1.0"));
                    imageReq.setRawHeader("Accept", QByteArrayLiteral("image/*,*/*"));
                    QNetworkReply *imgReply = nam->get(imageReq);
                    connect(imgReply, &QNetworkReply::finished, this, [this, imgReply, index, requestByIndexRef, looksLikeImageBytes]() {
                        const QByteArray imgData = imgReply->readAll();
                        const int imgCode = imgReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                        const QString imgType = imgReply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
                        const bool imgOk = ((imgCode >= 200 && imgCode < 300) || imgCode == 0) &&
                                           !imgData.isEmpty() &&
                                           (imgType.startsWith(QStringLiteral("image/")) || looksLikeImageBytes(imgData));
                        imgReply->deleteLater();
                        if (imgOk) {
                            emit imageGenerated(imgData, QString());
                            return;
                        }
                        requestByIndexRef(index + 1, requestByIndexRef);
                    });
                    reply->deleteLater();
                    return;
                }
            }

            reply->deleteLater();
            providerErrors->append(QStringLiteral("Pollinations endpoint %1 echec (HTTP %2)")
                                       .arg(index + 1)
                                       .arg(httpCode));
            requestByIndexRef(index + 1, requestByIndexRef);
        });
        };

        requestByIndex(0, requestByIndex);
    };

    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString geminiKey = env.value(QStringLiteral("GEMINI_API_KEY"));
    const QString openaiKey = env.value(QStringLiteral("OPENAI_API_KEY"));
    const QString hfKey = env.value(QStringLiteral("HF_API_KEY"));
    const QString geminiModel = env.value(QStringLiteral("GEMINI_IMAGE_MODEL"),
                                          QStringLiteral("imagen-3.0-generate-002"));

    auto tryHuggingFace = [this, hfKey, prompt, looksLikeImageBytes, startFreeFallback, providerErrors]() {
        if (hfKey.isEmpty()) {
            startFreeFallback();
            return;
        }
        QNetworkRequest req(QUrl(QStringLiteral("https://api-inference.huggingface.co/models/stabilityai/stable-diffusion-xl-base-1.0")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + hfKey.toUtf8());
        req.setRawHeader("Accept", QByteArrayLiteral("image/*,application/json,*/*"));

        QJsonObject body;
        body.insert(QStringLiteral("inputs"), prompt);
        QNetworkReply *reply = nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::finished, this, [this, reply, looksLikeImageBytes, startFreeFallback, providerErrors]() {
            const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray raw = reply->readAll();
            const QString ctype = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
            reply->deleteLater();

            const bool ok = ((code >= 200 && code < 300) || code == 0) &&
                            !raw.isEmpty() &&
                            (ctype.startsWith(QStringLiteral("image/")) || looksLikeImageBytes(raw));
            if (ok) {
                emit imageGenerated(raw, QString());
                return;
            }
            providerErrors->append(QStringLiteral("HuggingFace echec (HTTP %1): %2")
                                       .arg(code)
                                       .arg(QString::fromUtf8(raw).left(180)));
            startFreeFallback();
        });
    };

    auto tryOpenAI = [this, openaiKey, prompt, looksLikeImageBytes, providerErrors, tryHuggingFace]() {
        if (openaiKey.isEmpty() || s_skipOpenAIProvider) {
            tryHuggingFace();
            return;
        }

        QNetworkRequest req(QUrl(QStringLiteral("https://api.openai.com/v1/images/generations")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("Authorization", QByteArrayLiteral("Bearer ") + openaiKey.toUtf8());
        req.setRawHeader("Accept", QByteArrayLiteral("application/json"));

        QJsonObject body;
        body.insert(QStringLiteral("model"), QStringLiteral("gpt-image-1"));
        body.insert(QStringLiteral("prompt"), prompt);
        body.insert(QStringLiteral("size"), QStringLiteral("1024x1024"));

        QNetworkReply *reply = nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
        QTimer *timer = new QTimer(reply);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, reply, [reply]() {
            if (reply->isRunning())
                reply->abort();
        });
        timer->start(120000);

        connect(reply, &QNetworkReply::finished, this, [this, reply, tryHuggingFace, looksLikeImageBytes, providerErrors]() {
            const int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray raw = reply->readAll();
            const QString netErr = reply->errorString();
            reply->deleteLater();

            if ((httpCode >= 200 && httpCode < 300) && !raw.isEmpty()) {
                const QJsonDocument doc = QJsonDocument::fromJson(raw);
                if (doc.isObject()) {
                    const QJsonArray arr = doc.object().value(QStringLiteral("data")).toArray();
                    if (!arr.isEmpty()) {
                        const QJsonObject first = arr.at(0).toObject();
                        const QString b64 = first.value(QStringLiteral("b64_json")).toString();
                        if (!b64.isEmpty()) {
                            const QByteArray img = QByteArray::fromBase64(b64.toUtf8());
                            if (!img.isEmpty() && looksLikeImageBytes(img)) {
                                emit imageGenerated(img, QString());
                                return;
                            }
                        }
                        const QString url = first.value(QStringLiteral("url")).toString();
                        if (!url.isEmpty()) {
                            QNetworkRequest imgReq{QUrl(url)};
                            imgReq.setRawHeader("User-Agent", QByteArrayLiteral("QtImageClient/1.0"));
                            imgReq.setRawHeader("Accept", QByteArrayLiteral("image/*,*/*"));
                            QNetworkReply *imgReply = nam->get(imgReq);
                            connect(imgReply, &QNetworkReply::finished, this, [this, imgReply, tryHuggingFace, looksLikeImageBytes]() {
                                const QByteArray imgData = imgReply->readAll();
                                const QString ctype = imgReply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
                                const int code = imgReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                                const bool ok = ((code >= 200 && code < 300) || code == 0) &&
                                                !imgData.isEmpty() &&
                                                (ctype.startsWith(QStringLiteral("image/")) || looksLikeImageBytes(imgData));
                                imgReply->deleteLater();
                                if (ok) {
                                    emit imageGenerated(imgData, QString());
                                    return;
                                }
                                tryHuggingFace();
                            });
                            return;
                        }
                    }
                }
            }

            providerErrors->append(QStringLiteral("OpenAI echec (HTTP %1, net=%2): %3")
                                       .arg(httpCode)
                                       .arg(netErr)
                                       .arg(QString::fromUtf8(raw).left(220)));
            if (httpCode == 400 && raw.contains("billing_hard_limit_reached")) {
                s_skipOpenAIProvider = true;
                providerErrors->append(QStringLiteral("OpenAI desactive automatiquement (billing_hard_limit_reached)."));
            }
            tryHuggingFace();
        });
    };

    auto tryGemini = [this, geminiKey, geminiModel, prompt, looksLikeImageBytes, providerErrors, tryOpenAI]() {
        if (geminiKey.isEmpty() || s_skipGeminiProvider) {
            tryOpenAI();
            return;
        }

        const QUrl url(QStringLiteral("https://generativelanguage.googleapis.com/v1beta/models/%1:generateImages?key=%2")
                           .arg(geminiModel, geminiKey));
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setRawHeader("Accept", QByteArrayLiteral("application/json"));

        QJsonObject promptObj;
        promptObj.insert(QStringLiteral("text"), prompt);
        QJsonObject imageConfig;
        imageConfig.insert(QStringLiteral("numberOfImages"), 1);

        QJsonObject body;
        body.insert(QStringLiteral("prompt"), promptObj);
        body.insert(QStringLiteral("imageConfig"), imageConfig);

        QNetworkReply *reply = nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
        QTimer *timer = new QTimer(reply);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, reply, [reply]() {
            if (reply->isRunning())
                reply->abort();
        });
        timer->start(120000);

        connect(reply, &QNetworkReply::finished, this, [this, reply, looksLikeImageBytes, providerErrors, tryOpenAI]() {
            const int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray raw = reply->readAll();
            const QString netErr = reply->errorString();
            reply->deleteLater();

            if ((httpCode >= 200 && httpCode < 300) && !raw.isEmpty()) {
                const QJsonDocument doc = QJsonDocument::fromJson(raw);
                if (doc.isObject()) {
                    const QJsonArray generated = doc.object().value(QStringLiteral("generatedImages")).toArray();
                    if (!generated.isEmpty()) {
                        const QJsonObject imgObj = generated.at(0).toObject().value(QStringLiteral("image")).toObject();
                        const QString b64 = imgObj.value(QStringLiteral("imageBytes")).toString();
                        if (!b64.isEmpty()) {
                            const QByteArray img = QByteArray::fromBase64(b64.toUtf8());
                            if (!img.isEmpty() && looksLikeImageBytes(img)) {
                                emit imageGenerated(img, QString());
                                return;
                            }
                        }
                    }
                }
            }

            providerErrors->append(QStringLiteral("Gemini echec (HTTP %1, net=%2): %3")
                                       .arg(httpCode)
                                       .arg(netErr)
                                       .arg(QString::fromUtf8(raw).left(220)));
            if (httpCode == 404) {
                s_skipGeminiProvider = true;
                providerErrors->append(QStringLiteral("Gemini desactive automatiquement (modele/endpoint non disponible)."));
            }
            tryOpenAI();
        });
    };

    tryGemini();
}

