#ifndef FACELOGINDIALOG_H
#define FACELOGINDIALOG_H

#include <QDialog>

#if defined(LEATHER_HAVE_OPENCV)
#include <opencv2/core.hpp>
#endif

class QLabel;
class QPushButton;
class QTimer;
class QSerialPort;

/// Dialogue de connexion par camera : detection de visage (OpenCV + Haar cascade).
/// Si le projet n'est pas compile avec LEATHER_HAVE_OPENCV, le dialogue affiche un message d'erreur.
class FaceLoginDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit FaceLoginDialog(QWidget *parent = nullptr);
    ~FaceLoginDialog() override;

    static bool isOpenCvAvailable();

private slots:
    void onFrameTick();
    void onStartRecognition();
    void onStopRecognition();
    void onRfidReadyRead();

private:
    void setupUi();
    bool loadCascadeToTempFile(QString *errorMessage);
    bool startCamera(QString *errorMessage = nullptr);
    void cleanupCapture();
    bool initRfidReader(QString *errorMessage = nullptr);
    void closeRfidReader();
    QString normalizeUidText(const QString &text) const;
    void updateAuthStateUi();
#if defined(LEATHER_HAVE_OPENCV)
    void updateFaceAuthButtons();
#endif

    QLabel *m_videoLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_btnStart = nullptr;
    QPushButton *m_btnStop = nullptr;
    QPushButton *m_btnConnect = nullptr;
    QPushButton *m_btnCancel = nullptr;
#if defined(LEATHER_HAVE_OPENCV)
    QPushButton *m_btnEnroll = nullptr;
    QPushButton *m_btnClearModel = nullptr;
#endif
    QTimer *m_timer = nullptr;
    QSerialPort *m_rfidSerial = nullptr;
    QByteArray m_rfidBuffer;

#ifdef LEATHER_HAVE_OPENCV
    void *m_capture = nullptr;  // cv::VideoCapture*
    void *m_classifier = nullptr; // cv::CascadeClassifier*
    cv::Mat m_lastGrayFrame;
    cv::Rect m_lastFaceRect;
    bool m_hasValidFace = false;
#endif

    QString m_cascadeTempPath;
    QString m_expectedRfidUid = QStringLiteral("85 2E 1C 06");
    bool m_faceVerified = false;
    bool m_rfidAuthorized = false;
    int m_stableFaceFrames = 0;
    int m_verifyCooldownFrames = 0;
};

#endif
