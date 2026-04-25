#ifndef FACELOGINDIALOG_H
#define FACELOGINDIALOG_H

#include <QDialog>

#if defined(LEATHER_HAVE_OPENCV)
#include <opencv2/core.hpp>
#endif

class QLabel;
class QPushButton;
class QTimer;

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

private:
    void setupUi();
    bool loadCascadeToTempFile(QString *errorMessage);
    bool startCamera(QString *errorMessage = nullptr);
    void cleanupCapture();
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

#ifdef LEATHER_HAVE_OPENCV
    void *m_capture = nullptr;  // cv::VideoCapture*
    void *m_classifier = nullptr; // cv::CascadeClassifier*
    cv::Mat m_lastGrayFrame;
    cv::Rect m_lastFaceRect;
    bool m_hasValidFace = false;
#endif

    QString m_cascadeTempPath;
    int m_stableFaceFrames = 0;
    int m_verifyCooldownFrames = 0;
};

#endif
