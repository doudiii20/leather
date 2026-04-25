#include "faceauthmanager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#if defined(LEATHER_HAVE_OPENCV_FACE) && defined(__has_include)
#  if __has_include(<opencv2/face.hpp>)
#    include <opencv2/face.hpp>
#    define LEATHER_HAS_OPENCV_FACE_HEADER 1
#  endif
#endif

#include <cmath>

namespace {

constexpr int kFaceTrainSize = 200;
constexpr double kHistCorrelMin = 0.55; ///< Similarité minimale histogramme (tolérante).
constexpr double kFaceCorrelMin = 0.45; ///< Similarité minimale forme/texture (NCC).

#ifdef LEATHER_HAS_OPENCV_FACE_HEADER
constexpr double kLbphMaxConfidence = 75.0; ///< LBPH : distance interne (plus bas = plus sûr).
#endif

static cv::Mat preprocessFaceRoi(const cv::Mat &grayFull, const cv::Rect &r)
{
    cv::Mat roi = grayFull(r).clone();
    cv::Mat resized;
    cv::resize(roi, resized, cv::Size(kFaceTrainSize, kFaceTrainSize));
    cv::equalizeHist(resized, resized);
    return resized;
}

static cv::Mat computeHist256(const cv::Mat &gray200)
{
    cv::Mat hist;
    const int histSize = 256;
    const float range[] = {0.f, 256.f};
    const float *ranges[] = {range};
    const int channels[] = {0};
    cv::calcHist(&gray200, 1, channels, cv::Mat(), hist, 1, &histSize, ranges, true, false);
    cv::normalize(hist, hist, 0, 1, cv::NORM_MINMAX);
    return hist;
}

static bool saveHistogramYaml(const cv::Mat &hist, const QString &path)
{
    cv::FileStorage fs(path.toStdString(), cv::FileStorage::WRITE);
    if (!fs.isOpened())
        return false;
    fs << "hist" << hist;
    fs.release();
    return true;
}

static bool loadHistogramYaml(const QString &path, cv::Mat &outHist)
{
    cv::FileStorage fs(path.toStdString(), cv::FileStorage::READ);
    if (!fs.isOpened())
        return false;
    fs["hist"] >> outHist;
    fs.release();
    return !outHist.empty();
}

static bool saveFaceModelYaml(const cv::Mat &hist, const cv::Mat &face, const QString &path)
{
    cv::FileStorage fs(path.toStdString(), cv::FileStorage::WRITE);
    if (!fs.isOpened())
        return false;
    fs << "hist" << hist;
    fs << "face" << face;
    fs.release();
    return true;
}

static bool loadFaceModelYaml(const QString &path, cv::Mat &outHist, cv::Mat &outFace)
{
    cv::FileStorage fs(path.toStdString(), cv::FileStorage::READ);
    if (!fs.isOpened())
        return false;
    fs["hist"] >> outHist;
    fs["face"] >> outFace;
    fs.release();
    return !outHist.empty();
}

} // namespace

namespace FaceAuthManager {

QString dataDirectory()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dir = base + QStringLiteral("/RoyalLeather/face");
    QDir().mkpath(dir);
    return dir;
}

QString enrollmentHistogramPath()
{
    return dataDirectory() + QStringLiteral("/admin_face_hist.yml");
}

QString enrollmentLbphPath()
{
    return dataDirectory() + QStringLiteral("/admin_lbph.yml");
}

QString defaultFaceModelPath()
{
    return dataDirectory() + QStringLiteral("/face_model.xml");
}

bool isEnrolled()
{
#ifdef LEATHER_HAS_OPENCV_FACE_HEADER
    if (QFile::exists(defaultFaceModelPath()))
        return true;
    if (QFile::exists(enrollmentLbphPath()))
        return true;
#endif
    return QFile::exists(enrollmentHistogramPath());
}

bool clearEnrollment(QString *errorMessage)
{
    bool ok = true;
    if (QFile::exists(enrollmentHistogramPath()) && !QFile::remove(enrollmentHistogramPath()))
        ok = false;
#ifdef LEATHER_HAS_OPENCV_FACE_HEADER
    if (QFile::exists(enrollmentLbphPath()) && !QFile::remove(enrollmentLbphPath()))
        ok = false;
    if (QFile::exists(defaultFaceModelPath()) && !QFile::remove(defaultFaceModelPath()))
        ok = false;
#endif
    if (!ok && errorMessage)
        *errorMessage = QStringLiteral("Impossible de supprimer un fichier de modele.");
    return ok;
}

bool enroll(const cv::Mat &grayFullFrame, const cv::Rect &faceRect, QString *errorMessage)
{
    if (grayFullFrame.empty() || faceRect.width < 30 || faceRect.height < 30) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Visage trop petit ou image vide.");
        return false;
    }

    const cv::Mat face = preprocessFaceRoi(grayFullFrame, faceRect);

#ifdef LEATHER_HAS_OPENCV_FACE_HEADER
    try {
        std::vector<cv::Mat> imgs = {face};
        std::vector<int> labels = {0};
        cv::Ptr<cv::face::LBPHFaceRecognizer> rec = cv::face::LBPHFaceRecognizer::create(1, 8, 8, 8, 200.);
        rec->train(imgs, labels);
        rec->write(enrollmentLbphPath().toStdString());
        rec->write(defaultFaceModelPath().toStdString());
    } catch (const cv::Exception &e) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8(e.what());
        return false;
    }
#endif

    const cv::Mat hist = computeHist256(face);
    if (!saveFaceModelYaml(hist, face, enrollmentHistogramPath())) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Ecriture du fichier modele impossible.");
        return false;
    }

    return true;
}

bool verify(const cv::Mat &grayFullFrame, const cv::Rect &faceRect, double *confidenceOut, QString *errorMessage)
{
    if (grayFullFrame.empty() || faceRect.width < 30 || faceRect.height < 30) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Visage trop petit ou image vide.");
        return false;
    }

    const cv::Mat face = preprocessFaceRoi(grayFullFrame, faceRect);

#ifdef LEATHER_HAS_OPENCV_FACE_HEADER
    QString lbphPath = defaultFaceModelPath();
    if (!QFile::exists(lbphPath))
        lbphPath = enrollmentLbphPath();
    if (QFile::exists(lbphPath)) {
        try {
            cv::Ptr<cv::face::LBPHFaceRecognizer> rec = cv::face::LBPHFaceRecognizer::create();
            rec->read(lbphPath.toStdString());
            int label = -1;
            double conf = 0.;
            rec->predict(face, label, conf);
            if (confidenceOut)
                *confidenceOut = conf;
            if (label == 0 && conf < kLbphMaxConfidence)
                return true;
            if (errorMessage)
                *errorMessage = QStringLiteral("Le visage ne correspond pas au modele enregistre (LBPH).");
            return false;
        } catch (const cv::Exception &e) {
            if (errorMessage)
                *errorMessage = QString::fromUtf8(e.what());
            return false;
        }
    }
#endif

    cv::Mat refHist;
    cv::Mat refFace;
    const bool modelLoaded = loadFaceModelYaml(enrollmentHistogramPath(), refHist, refFace);
    if ((!modelLoaded && !loadHistogramYaml(enrollmentHistogramPath(), refHist)) || refHist.empty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Aucun modele enregistre. Utilisez d'abord « Enregistrer le visage ».");
        return false;
    }

    const cv::Mat curHist = computeHist256(face);
    const double histCorrel = cv::compareHist(refHist, curHist, cv::HISTCMP_CORREL);

    double faceCorrel = -1.0;
    if (!refFace.empty() && refFace.size() == face.size() && refFace.type() == face.type()) {
        cv::Mat corr;
        cv::matchTemplate(refFace, face, corr, cv::TM_CCOEFF_NORMED);
        faceCorrel = corr.at<float>(0, 0);
    }

    if (confidenceOut)
        *confidenceOut = 1.0 - qMax(histCorrel, faceCorrel);

    if (histCorrel >= kHistCorrelMin || faceCorrel >= kFaceCorrelMin)
        return true;

    if (errorMessage) {
        *errorMessage = QStringLiteral(
            "Visage non reconnu (hist=%1 < %2, face=%3 < %4). "
            "Repositionnez-vous ou re-enregistrez le modele.")
                            .arg(QString::number(histCorrel, 'f', 3))
                            .arg(QString::number(kHistCorrelMin, 'f', 3))
                            .arg(QString::number(faceCorrel, 'f', 3))
                            .arg(QString::number(kFaceCorrelMin, 'f', 3));
    }
    return false;
}

} // namespace FaceAuthManager
