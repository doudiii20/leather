#include "facelogindialog.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <vector>

#ifdef LEATHER_HAVE_OPENCV
#include "faceauthmanager.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#endif

namespace {
constexpr int kMinStableFrames = 12;
constexpr int kVideoWidth = 640;
constexpr int kVideoHeight = 480;

static QStringList cascadeCandidatePaths(const QString &resourceExtractedPath)
{
    QStringList paths;
    if (!resourceExtractedPath.isEmpty())
        paths << resourceExtractedPath;

    const QString appDir = QCoreApplication::applicationDirPath();
    paths << (appDir + QStringLiteral("/haarcascade_frontalface_alt.xml"));
    paths << (appDir + QStringLiteral("/haarcascade_frontalface_default.xml"));
    return paths;
}
} // namespace

bool FaceLoginDialog::isOpenCvAvailable()
{
#ifdef LEATHER_HAVE_OPENCV
    return true;
#else
    return false;
#endif
}

FaceLoginDialog::FaceLoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();

#ifndef LEATHER_HAVE_OPENCV
    m_statusLabel->setText(
        QStringLiteral("OpenCV n'est pas active dans ce build. Definissez OPENCV_DIR dans leather.pro "
                       "et recompilez (voir commentaires dans le fichier .pro)."));
    m_btnConnect->setEnabled(false);
    return;
#else
    QString cascadeErr;
    if (!loadCascadeToTempFile(&cascadeErr)) {
        m_statusLabel->setText(QStringLiteral("Erreur cascade : %1").arg(cascadeErr));
        m_btnConnect->setEnabled(false);
        m_btnStart->setEnabled(false);
        m_btnStop->setEnabled(false);
        return;
    }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FaceLoginDialog::onFrameTick);
    connect(m_btnStart, &QPushButton::clicked, this, &FaceLoginDialog::onStartRecognition);
    connect(m_btnStop, &QPushButton::clicked, this, &FaceLoginDialog::onStopRecognition);
    m_btnStop->setEnabled(false);

    if (FaceAuthManager::isEnrolled())
        m_statusLabel->setText(QStringLiteral(
            "Modele existant detecte. Cliquez sur Demarrer reconnaissance, "
            "ou Re-enregistrer le visage admin pour recalibrer."));
    else
        m_statusLabel->setText(QStringLiteral(
            "Aucun modele enregistre : centrez votre visage, attendez la stabilisation, puis cliquez sur "
            "« Enregistrer le visage admin »."));

    connect(m_btnConnect, &QPushButton::clicked, this, [this]() {
        QString err;
        double conf = 0.;
        if (!m_hasValidFace || m_lastGrayFrame.empty()) {
            QMessageBox::warning(this, QStringLiteral("Reconnaissance faciale"),
                                 QStringLiteral("Aucun visage stable detecte. Repositionnez-vous."));
            return;
        }
        if (!FaceAuthManager::verify(m_lastGrayFrame, m_lastFaceRect, &conf, &err)) {
            QMessageBox::warning(this, QStringLiteral("Reconnaissance faciale"), err);
            return;
        }
        accept();
    });

    connect(m_btnEnroll, &QPushButton::clicked, this, [this]() {
        QString err;
        if (!m_hasValidFace || m_lastGrayFrame.empty()) {
            QMessageBox::warning(this, QStringLiteral("Enregistrement"),
                                 QStringLiteral("Aucun visage stable detecte. Repositionnez-vous."));
            return;
        }
        if (FaceAuthManager::enroll(m_lastGrayFrame, m_lastFaceRect, &err)) {
            QMessageBox::information(this, QStringLiteral("Enregistrement"),
                                    QStringLiteral("Visage administrateur enregistre. Vous pouvez vous connecter."));
            m_statusLabel->setText(QStringLiteral("Modele pret — placez votre visage puis cliquez sur Se connecter."));
        } else {
            QMessageBox::warning(this, QStringLiteral("Enregistrement"), err);
        }
        updateFaceAuthButtons();
    });

    connect(m_btnClearModel, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(
                this,
                QStringLiteral("Supprimer le modèle"),
                QStringLiteral("Supprimer le visage enregistré sur cette machine ?\n"
                               "Vous devrez en enregistrer un nouveau pour vous connecter par la caméra."),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No)
            != QMessageBox::Yes) {
            return;
        }
        QString err;
        if (!FaceAuthManager::clearEnrollment(&err)) {
            QMessageBox::warning(this, QStringLiteral("Supprimer le modèle"), err);
            return;
        }
        QMessageBox::information(this, QStringLiteral("Supprimer le modèle"),
                                 QStringLiteral("Modèle supprimé. Démarrez la reconnaissance et enregistrez un nouveau visage admin."));
        m_statusLabel->setText(QStringLiteral(
            "Aucun modèle : démarrez la caméra, stabilisez le visage, puis cliquez sur « Enregistrer le visage admin »."));
        updateFaceAuthButtons();
    });

    updateFaceAuthButtons();
#endif
}

FaceLoginDialog::~FaceLoginDialog()
{
#ifdef LEATHER_HAVE_OPENCV
    cleanupCapture();
#endif
}

void FaceLoginDialog::setupUi()
{
    setWindowTitle(QStringLiteral("Connexion — reconnaissance faciale"));
    setModal(true);
    resize(680, 560);

    m_videoLabel = new QLabel(this);
    m_videoLabel->setMinimumSize(kVideoWidth, kVideoHeight);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet(QStringLiteral("QLabel { background: #101010; color: #bbb; }"));
    m_videoLabel->setText(QStringLiteral("Flux video"));

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);

    m_btnStart = new QPushButton(QStringLiteral("Démarrer reconnaissance"), this);
    m_btnStart->setCursor(Qt::PointingHandCursor);
    m_btnStart->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #1e5a3a; color: #fff; border: none; border-radius: 4px; "
        "padding: 8px 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #256b46; }"));

    m_btnStop = new QPushButton(QStringLiteral("Arrêter"), this);
    m_btnStop->setCursor(Qt::PointingHandCursor);
    m_btnStop->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #7b2f2f; color: #fff; border: none; border-radius: 4px; "
        "padding: 8px 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #8f3a3a; }"
        "QPushButton:disabled { background-color: #b8a0a0; color: #ede6e6; }"));

    m_btnConnect = new QPushButton(QStringLiteral("Se connecter"), this);
    m_btnConnect->setEnabled(false);
    m_btnConnect->setCursor(Qt::PointingHandCursor);
    m_btnConnect->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #5d2e06; color: #fff; border: none; border-radius: 4px; "
        "padding: 8px 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #70380a; }"
        "QPushButton:disabled { background-color: #b8a894; color: #ede6d7; }"));

#ifdef LEATHER_HAVE_OPENCV
    m_btnEnroll = new QPushButton(QStringLiteral("Enregistrer le visage admin"), this);
    m_btnEnroll->setEnabled(false);
    m_btnEnroll->setCursor(Qt::PointingHandCursor);
    m_btnEnroll->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #1e5a3a; color: #fff; border: none; border-radius: 4px; "
        "padding: 8px 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #256b46; }"
        "QPushButton:disabled { background-color: #a8b8ae; color: #eef4ef; }"));

    m_btnClearModel = new QPushButton(QStringLiteral("Supprimer le modèle"), this);
    m_btnClearModel->setEnabled(false);
    m_btnClearModel->setCursor(Qt::PointingHandCursor);
    m_btnClearModel->setToolTip(QStringLiteral("Supprime le visage enregistré sur ce PC. Vous pourrez en enregistrer un nouveau."));
    m_btnClearModel->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #fff5f0; color: #8a2b0a; border: 1px solid #d4a090; border-radius: 4px; "
        "padding: 8px 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #ffe8dd; border-color: #b87055; }"
        "QPushButton:disabled { background-color: #e8e4e0; color: #b0a8a0; border-color: #d0ccc8; }"));
#endif

    m_btnCancel = new QPushButton(QStringLiteral("Annuler"), this);
    m_btnCancel->setCursor(Qt::PointingHandCursor);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
#ifdef LEATHER_HAVE_OPENCV
    btnRow->addWidget(m_btnEnroll);
    btnRow->addWidget(m_btnClearModel);
#endif
    btnRow->addWidget(m_btnStart);
    btnRow->addWidget(m_btnStop);
    btnRow->addWidget(m_btnCancel);
    btnRow->addWidget(m_btnConnect);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(m_videoLabel);
    mainLay->addWidget(m_statusLabel);
    mainLay->addLayout(btnRow);

    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

bool FaceLoginDialog::startCamera(QString *errorMessage)
{
#ifndef LEATHER_HAVE_OPENCV
    Q_UNUSED(errorMessage);
    return false;
#else
    cleanupCapture();
    m_capture = new cv::VideoCapture();
    m_classifier = new cv::CascadeClassifier();

    auto *cap = static_cast<cv::VideoCapture *>(m_capture);
    cap->open(0, cv::CAP_ANY);
    if (!cap->isOpened()) {
        qDebug() << "[FaceLogin] Camera non detectee (index 0).";
        if (errorMessage)
            *errorMessage = QStringLiteral("Impossible d'ouvrir la camera (index 0).");
        delete static_cast<cv::CascadeClassifier *>(m_classifier);
        m_classifier = nullptr;
        delete cap;
        m_capture = nullptr;
        return false;
    }
    cap->set(cv::CAP_PROP_FRAME_WIDTH, kVideoWidth);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, kVideoHeight);

    auto *cc = static_cast<cv::CascadeClassifier *>(m_classifier);
    bool cascadeLoaded = false;
    QString loadedCascadePath;
    const QStringList candidates = cascadeCandidatePaths(m_cascadeTempPath);
    for (const QString &path : candidates) {
        if (path.isEmpty() || !QFile::exists(path))
            continue;
        if (cc->load(path.toStdString())) {
            cascadeLoaded = true;
            loadedCascadePath = path;
            break;
        }
    }
    if (!cascadeLoaded) {
        qDebug() << "[FaceLogin] Echec chargement cascade. Candidats:" << candidates;
        if (errorMessage)
            *errorMessage = QStringLiteral("Chargement du classificateur Haar impossible (fichier XML introuvable/invalide).");
        cap->release();
        delete cap;
        m_capture = nullptr;
        delete cc;
        m_classifier = nullptr;
        return false;
    }
    qDebug() << "[FaceLogin] Cascade chargee depuis:" << loadedCascadePath;

    m_stableFaceFrames = 0;
    m_verifyCooldownFrames = 0;
    m_hasValidFace = false;
    if (m_timer)
        m_timer->start(33);
    return true;
#endif
}

bool FaceLoginDialog::loadCascadeToTempFile(QString *errorMessage)
{
    QDir().mkpath(QDir::tempPath());

    QFile f(QStringLiteral(":/opencv/haarcascade_frontalface_alt.xml"));
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "[FaceLogin] Ressource cascade introuvable :/opencv/haarcascade_frontalface_alt.xml";
        if (errorMessage)
            *errorMessage = QStringLiteral("ressource introuvable :/opencv/haarcascade_frontalface_alt.xml");
        return false;
    }
    const QByteArray data = f.readAll();
    f.close();

    QStringList targetDirs;
    const QString appDataRoot = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataRoot.isEmpty())
        targetDirs << (appDataRoot + QStringLiteral("/RoyalLeather/cache"));
    targetDirs << (QDir::tempPath() + QStringLiteral("/RoyalLeather/cache"));

    for (const QString &dirPath : targetDirs) {
        QDir dir(dirPath);
        if (!dir.exists() && !QDir().mkpath(dirPath))
            continue;

        const QString cascadePath = dir.filePath(QStringLiteral("haarcascade_frontalface_alt.xml"));
        QFile out(cascadePath);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate))
            continue;

        const qint64 written = out.write(data);
        out.close();
        if (written != data.size())
            continue;

        m_cascadeTempPath = cascadePath;
        qDebug() << "[FaceLogin] Cascade extraite vers:" << m_cascadeTempPath;
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("impossible d'ecrire le fichier cascade (verifiez droits d'ecriture AppData/TEMP)");
    }
    qDebug() << "[FaceLogin] Impossible d'ecrire la cascade dans AppData/TEMP";
    return false;
}

#if defined(LEATHER_HAVE_OPENCV)
void FaceLoginDialog::updateFaceAuthButtons()
{
    const bool enrolled = FaceAuthManager::isEnrolled();
    if (m_btnEnroll) {
        m_btnEnroll->setVisible(true);
        m_btnEnroll->setText(
            enrolled ? QStringLiteral("Re-enregistrer le visage admin")
                     : QStringLiteral("Enregistrer le visage admin"));
    }
    const bool stable = m_stableFaceFrames >= kMinStableFrames;
    if (m_btnEnroll)
        m_btnEnroll->setEnabled(stable && m_hasValidFace);
    if (m_btnClearModel)
        m_btnClearModel->setEnabled(enrolled);
    m_btnConnect->setEnabled(enrolled && stable && m_hasValidFace);
}
#endif

void FaceLoginDialog::cleanupCapture()
{
#ifdef LEATHER_HAVE_OPENCV
    if (m_timer) {
        m_timer->stop();
    }
    if (m_capture) {
        static_cast<cv::VideoCapture *>(m_capture)->release();
        delete static_cast<cv::VideoCapture *>(m_capture);
        m_capture = nullptr;
    }
    if (m_classifier) {
        delete static_cast<cv::CascadeClassifier *>(m_classifier);
        m_classifier = nullptr;
    }
#endif
}

void FaceLoginDialog::onStartRecognition()
{
#ifndef LEATHER_HAVE_OPENCV
    return;
#else
    QString err;
    if (!startCamera(&err)) {
        m_statusLabel->setText(err);
        m_btnConnect->setEnabled(false);
        return;
    }
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_statusLabel->setText(FaceAuthManager::isEnrolled()
                               ? QStringLiteral("Reconnaissance en cours...")
                               : QStringLiteral("Modele absent : enregistrez d'abord le visage admin."));
    updateFaceAuthButtons();
#endif
}

void FaceLoginDialog::onStopRecognition()
{
#ifndef LEATHER_HAVE_OPENCV
    return;
#else
    cleanupCapture();
    m_videoLabel->setText(QStringLiteral("Flux arrêté"));
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
    m_btnConnect->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("Reconnaissance arrêtée."));
    m_stableFaceFrames = 0;
    m_hasValidFace = false;
    updateFaceAuthButtons();
#endif
}

void FaceLoginDialog::onFrameTick()
{
#ifndef LEATHER_HAVE_OPENCV
    return;
#else
    auto *cap = static_cast<cv::VideoCapture *>(m_capture);
    auto *cc = static_cast<cv::CascadeClassifier *>(m_classifier);
    if (!cap || !cc || !cap->isOpened())
        return;

    cv::Mat frame;
    if (!cap->read(frame) || frame.empty())
        return;

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    cc->detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(48, 48));

    const bool enrolled = FaceAuthManager::isEnrolled();

    if (!faces.empty()) {
        const cv::Rect best = *std::max_element(
            faces.begin(), faces.end(),
            [](const cv::Rect &a, const cv::Rect &b) { return a.area() < b.area(); });
        for (const auto &r : faces)
            cv::rectangle(frame, r, cv::Scalar(60, 60, 60), 1);
        cv::rectangle(frame, best, cv::Scalar(80, 220, 120), 2);

        ++m_stableFaceFrames;
        m_lastGrayFrame = gray.clone();
        m_lastFaceRect = best;
        m_hasValidFace = true;

        if (m_stableFaceFrames < kMinStableFrames) {
            m_statusLabel->setText(QStringLiteral("Visage detecte (%1/%2)...")
                                       .arg(qMin(m_stableFaceFrames, kMinStableFrames))
                                       .arg(kMinStableFrames));
        } else if (enrolled) {
            m_statusLabel->setText(QStringLiteral("Reconnaissance prete — vérification automatique..."));
            if (m_verifyCooldownFrames <= 0) {
                QString err;
                double conf = 0.;
                if (FaceAuthManager::verify(m_lastGrayFrame, m_lastFaceRect, &conf, &err)) {
                    m_statusLabel->setText(QStringLiteral("Utilisateur reconnu. Connexion automatique..."));
                    accept();
                    return;
                }
                m_statusLabel->setText(QStringLiteral("Utilisateur non reconnu."));
                m_verifyCooldownFrames = 20;
            }
        } else {
            m_statusLabel->setText(QStringLiteral("Visage stable — cliquez sur Enregistrer le visage admin."));
        }
    } else {
        m_stableFaceFrames = 0;
        m_hasValidFace = false;
        m_statusLabel->setText(QStringLiteral("Aucun visage detecte — centrez-vous face a la camera."));
    }

    if (m_verifyCooldownFrames > 0)
        --m_verifyCooldownFrames;

    updateFaceAuthButtons();

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
    m_videoLabel->setPixmap(QPixmap::fromImage(img.copy()));
#endif
}
