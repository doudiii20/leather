#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include <QObject>
#include <QImage>

class QNetworkAccessManager;

class ImageManager : public QObject
{
    Q_OBJECT

public:
    explicit ImageManager(QObject *parent = nullptr);
    void generateImage(const QString &prompt);

signals:
    void requestStarted(const QString &label);
    void imageReady(const QImage &image);
    void descriptionReady(const QString &text);
    void errorOccurred(const QString &message);
    void requestFinished();

private:
    QNetworkAccessManager *m_networkManager = nullptr;
};

#endif // IMAGEMANAGER_H
