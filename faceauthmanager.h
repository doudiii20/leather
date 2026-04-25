#ifndef FACEAUTHMANAGER_H
#define FACEAUTHMANAGER_H

#include <QString>
#include <opencv2/core.hpp>

/// Stockage et comparaison du visage « administrateur » pour la connexion caméra.
/// — Si le projet est lié à opencv_face (LEATHER_HAVE_OPENCV_FACE) : LBPH (train / predict).
/// — Sinon : comparaison d’histogrammes (corrélation) sur le visage redimensionné (sans module contrib).
namespace FaceAuthManager {

QString dataDirectory();
QString enrollmentHistogramPath();
QString enrollmentLbphPath();
QString defaultFaceModelPath();

bool isEnrolled();

/// Enregistre le visage admin à partir d’une image niveaux de gris et du rectangle du visage.
bool enroll(const cv::Mat &grayFullFrame, const cv::Rect &faceRect, QString *errorMessage = nullptr);

/// Retourne true si le visage correspond au modèle enregistré. \a confidenceOut : score (plus bas = meilleur pour LBPH ; corrélation pour le mode histogramme).
bool verify(const cv::Mat &grayFullFrame, const cv::Rect &faceRect, double *confidenceOut, QString *errorMessage = nullptr);

/// Supprime les données enregistrées (tests / réinitialisation).
bool clearEnrollment(QString *errorMessage = nullptr);

} // namespace FaceAuthManager

#endif
