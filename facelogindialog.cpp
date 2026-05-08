#include "facelogindialog.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>

#if defined(LEATHER_HAVE_OPENCV)
#include "faceauthmanager.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#endif

bool FaceLoginDialog::isOpenCvAvailable()
{
#if defined(LEATHER_HAVE_OPENCV)
    return true;
#else
    return false;
#endif
}

FaceLoginDialog::FaceLoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
#if defined(LEATHER_HAVE_OPENCV)
    m_statusLabel->setText(QStringLiteral("Cliquez sur Demarrer, puis Se connecter pour verifier le visage."));

    m_timer = new QTimer(this);
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, &FaceLoginDialog::onFrameTick);
    connect(m_btnStart, &QPushButton::clicked, this, &FaceLoginDialog::onStartRecognition);
    connect(m_btnStop, &QPushButton::clicked, this, &FaceLoginDialog::onStopRecognition);
    connect(m_btnConnect, &QPushButton::clicked, this, [this]() {
        if (!m_hasValidFace || m_lastGrayFrame.empty() || m_lastFaceRect.width < 20 || m_lastFaceRect.height < 20) {
            QMessageBox::warning(this, QStringLiteral("Face ID"), QStringLiteral("Aucun visage valide detecte."));
            return;
        }
        double score = 0.0;
        QString err;
        if (!FaceAuthManager::verify(m_lastGrayFrame, m_lastFaceRect, &score, &err)) {
            const QString msg = err.isEmpty() ? QStringLiteral("Visage non reconnu.") : err;
            QMessageBox::warning(this, QStringLiteral("Face ID"), msg);
            return;
        }
        accept();
    });
#else
    m_statusLabel->setText(QStringLiteral(
        "Face ID desactive (OpenCV non configure)."));

    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(false);
    m_btnConnect->setEnabled(false);
#endif
}

FaceLoginDialog::~FaceLoginDialog()
{
    if (m_timer)
        m_timer->stop();
    cleanupCapture();
    closeRfidReader();
}

void FaceLoginDialog::setupUi()
{
    setWindowTitle(QStringLiteral("Connexion"));
    setModal(true);
    resize(520, 260);

    m_videoLabel = new QLabel(this);
    m_videoLabel->setMinimumSize(420, 120);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet(QStringLiteral("QLabel { background: #101010; color: #bbb; }"));
    m_videoLabel->setText(QStringLiteral("Face ID desactive"));

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);

    m_btnStart = new QPushButton(QStringLiteral("Demarrer"), this);
    m_btnStart->setCursor(Qt::PointingHandCursor);
    m_btnStart->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: #1e5a3a; color: #fff; border: none; border-radius: 4px; "
        "padding: 8px 18px; font-weight: bold; }"
        "QPushButton:hover { background-color: #256b46; }"));

    m_btnStop = new QPushButton(QStringLiteral("Arreter"), this);
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

    m_btnCancel = new QPushButton(QStringLiteral("Annuler"), this);
    m_btnCancel->setCursor(Qt::PointingHandCursor);

    auto *btnRow = new QHBoxLayout();
    btnRow->addStretch();
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

bool FaceLoginDialog::initRfidReader(QString *errorMessage)
{
    closeRfidReader();

    m_rfidSerial = new QSerialPort(this);
    m_rfidSerial->setBaudRate(QSerialPort::Baud9600);
    m_rfidSerial->setDataBits(QSerialPort::Data8);
    m_rfidSerial->setParity(QSerialPort::NoParity);
    m_rfidSerial->setStopBits(QSerialPort::OneStop);
    m_rfidSerial->setFlowControl(QSerialPort::NoFlowControl);

    QStringList candidates;
    candidates << QStringLiteral("COM3")
               << QStringLiteral("COM4")
               << QStringLiteral("COM5")
               << QStringLiteral("COM6")
               << QStringLiteral("COM7")
               << QStringLiteral("COM8")
               << QStringLiteral("COM9");
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : ports) {
        const QString haystack =
            (portInfo.description() + QLatin1Char(' ') + portInfo.manufacturer()).toLower();
        if (haystack.contains(QStringLiteral("arduino"))
            || haystack.contains(QStringLiteral("usb"))
            || haystack.contains(QStringLiteral("ch340"))
            || haystack.contains(QStringLiteral("cp210"))) {
            if (!candidates.contains(portInfo.portName()))
                candidates << portInfo.portName();
        }
    }
    for (const QSerialPortInfo &portInfo : ports) {
        if (!candidates.contains(portInfo.portName()))
            candidates << portInfo.portName();
    }

    for (const QString &portName : candidates) {
        m_rfidSerial->setPortName(portName);
        if (m_rfidSerial->open(QIODevice::ReadOnly)) {
            connect(m_rfidSerial, &QSerialPort::readyRead, this, &FaceLoginDialog::onRfidReadyRead);
            qDebug() << "[FaceLogin] Lecteur RFID connecte sur" << portName;
            return true;
        }
    }

    if (errorMessage)
        *errorMessage = QStringLiteral("Lecteur RFID indisponible (port serie non ouvert).");
    qDebug() << "[FaceLogin] Echec ouverture lecteur RFID:" << m_rfidSerial->errorString();
    closeRfidReader();
    return false;
}

void FaceLoginDialog::closeRfidReader()
{
    if (!m_rfidSerial)
        return;
    if (m_rfidSerial->isOpen())
        m_rfidSerial->close();
    m_rfidSerial->deleteLater();
    m_rfidSerial = nullptr;
    m_rfidBuffer.clear();
}

QString FaceLoginDialog::normalizeUidText(const QString &text) const
{
    static const QRegularExpression rx(QStringLiteral("\\b([0-9A-Fa-f]{1,2})\\b"));
    QStringList bytes;
    auto it = rx.globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        const QString byteHex = m.captured(1).toUpper().rightJustified(2, QLatin1Char('0'));
        bytes << byteHex;
    }
    return bytes.join(QStringLiteral(" "));
}

void FaceLoginDialog::updateAuthStateUi()
{
    m_btnConnect->setEnabled(m_hasValidFace);
}

void FaceLoginDialog::onRfidReadyRead()
{
    if (!m_rfidSerial)
        return;

    m_rfidBuffer += m_rfidSerial->readAll();
    while (true) {
        const int nlIdx = m_rfidBuffer.indexOf('\n');
        if (nlIdx < 0)
            break;

        const QByteArray rawLine = m_rfidBuffer.left(nlIdx);
        m_rfidBuffer.remove(0, nlIdx + 1);

        const QString line = QString::fromUtf8(rawLine).trimmed();
        if (line.isEmpty())
            continue;

        const QString uid = normalizeUidText(line);
        if (uid.isEmpty())
            continue;

        if (uid == m_expectedRfidUid)
            qDebug() << "[FaceLogin] UID RFID autorise detecte:" << uid;
        else
            qDebug() << "[FaceLogin] UID RFID refuse detecte:" << uid;
    }
}

bool FaceLoginDialog::startCamera(QString *errorMessage)
{
#if !defined(LEATHER_HAVE_OPENCV)
    Q_UNUSED(errorMessage);
    return false;
#else
    cleanupCapture();

    if (!loadCascadeToTempFile(errorMessage))
        return false;

    auto *cap = new cv::VideoCapture(0);
    if (!cap->isOpened()) {
        delete cap;
        if (errorMessage)
            *errorMessage = QStringLiteral("Impossible d'ouvrir la camera.");
        return false;
    }
    m_capture = cap;

    auto *classifier = new cv::CascadeClassifier;
    if (!classifier->load(m_cascadeTempPath.toStdString())) {
        delete classifier;
        cleanupCapture();
        if (errorMessage)
            *errorMessage = QStringLiteral("Cascade visage introuvable.");
        return false;
    }
    m_classifier = classifier;
    return true;
#endif
}

bool FaceLoginDialog::loadCascadeToTempFile(QString *errorMessage)
{
#if !defined(LEATHER_HAVE_OPENCV)
    Q_UNUSED(errorMessage);
    return false;
#else
    const QString dstDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dstDir);
    const QString dstPath = dstDir + QStringLiteral("/haarcascade_frontalface_alt.xml");

    QFile in(QStringLiteral(":/opencv/haarcascade_frontalface_alt.xml"));
    if (!in.open(QIODevice::ReadOnly)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Ressource cascade OpenCV manquante.");
        return false;
    }
    QFile out(dstPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Impossible de copier la cascade OpenCV.");
        return false;
    }
    out.write(in.readAll());
    out.close();
    m_cascadeTempPath = dstPath;
    return true;
#endif
}

#if defined(LEATHER_HAVE_OPENCV)
void FaceLoginDialog::updateFaceAuthButtons()
{
    updateAuthStateUi();
}
#endif

void FaceLoginDialog::cleanupCapture()
{
#if defined(LEATHER_HAVE_OPENCV)
    if (m_capture) {
        auto *cap = static_cast<cv::VideoCapture *>(m_capture);
        if (cap->isOpened())
            cap->release();
        delete cap;
        m_capture = nullptr;
    }
    if (m_classifier) {
        delete static_cast<cv::CascadeClassifier *>(m_classifier);
        m_classifier = nullptr;
    }
    m_lastGrayFrame.release();
    m_lastFaceRect = cv::Rect();
    m_hasValidFace = false;
    updateAuthStateUi();
#endif
}

void FaceLoginDialog::onStartRecognition()
{
#if !defined(LEATHER_HAVE_OPENCV)
    m_statusLabel->setText(QStringLiteral("Face ID desactive."));
#else
    QString err;
    if (!startCamera(&err)) {
        m_statusLabel->setText(err.isEmpty() ? QStringLiteral("Echec camera.") : err);
        return;
    }
    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_statusLabel->setText(QStringLiteral("Camera active. Regardez la camera."));
    if (m_timer)
        m_timer->start();
#endif
}

void FaceLoginDialog::onStopRecognition()
{
#if !defined(LEATHER_HAVE_OPENCV)
    m_statusLabel->setText(QStringLiteral("Aucune reconnaissance en cours."));
#else
    if (m_timer)
        m_timer->stop();
    cleanupCapture();
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("Camera arretee."));
    m_videoLabel->setText(QStringLiteral("Camera arretee"));
#endif
}

void FaceLoginDialog::onFrameTick()
{
#if !defined(LEATHER_HAVE_OPENCV)
    return;
#else
    if (!m_capture || !m_classifier)
        return;

    auto *cap = static_cast<cv::VideoCapture *>(m_capture);
    auto *classifier = static_cast<cv::CascadeClassifier *>(m_classifier);
    cv::Mat frame;
    if (!cap->read(frame) || frame.empty())
        return;

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    classifier->detectMultiScale(gray, faces, 1.1, 4, 0, cv::Size(80, 80));
    m_hasValidFace = !faces.empty();
    if (m_hasValidFace) {
        m_lastFaceRect = faces.front();
        m_lastGrayFrame = gray;
        cv::rectangle(frame, m_lastFaceRect, cv::Scalar(0, 200, 0), 2);
        m_statusLabel->setText(QStringLiteral("Visage detecte. Cliquez Se connecter."));
    } else {
        m_statusLabel->setText(QStringLiteral("Aucun visage detecte."));
    }
    updateAuthStateUi();

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    const QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
    m_videoLabel->setPixmap(QPixmap::fromImage(img.copy()).scaled(
        m_videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
#endif
}
