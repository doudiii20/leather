QT += sql network printsupport charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

qtHaveModule(location) {
    QT += location positioning quickwidgets qml quick
    DEFINES += LEATHER_HAVE_LOCATION_MAP
    message("Qt Location disponible : Smart Map dynamique active.")
} else {
    message("Qt Location indisponible : Smart Map en mode statique.")
}

qtHaveModule(webenginewidgets) {
    QT += webenginewidgets
    DEFINES += LEATHER_HAVE_WEBENGINE
    message("Qt WebEngine disponible : Leaflet map active.")
} else {
    message("Qt WebEngine indisponible : Leaflet map desactivee.")
}

qtHaveModule(webenginequick) {
    QT += webenginequick
    message("Qt WebEngine Quick disponible.")
}

CONFIG += c++17
INCLUDEPATH += $$PWD/../..

# --- OpenCV (reconnaissance faciale) ---
# Sans OPENCV_DIR, le projet compile sans LEATHER_HAVE_OPENCV (message « OpenCV non disponible » au clic).
#
# Option A — Variable d'environnement (recommandé) :
#   Windows : Panneau de config > Variables d'environnement > OPENCV_DIR = dossier build (contient include/ et x64/)
#   Qt Creator : Projet (panneau gauche) > Build > Environnement de build > Ajouter OPENCV_DIR
#   Puis : Exécuter qmake, puis Reconstruire.
#
# Option B — Fichier opencv_local.pri (recommandé si la variable d'environnement ne marche pas avec Qt Creator) :
#   Copiez opencv_local.pri.example vers opencv_local.pri dans ce dossier, ouvrez-le, mettez votre chemin OPENCV_DIR.
#   Puis : Exécuter qmake, Reconstruire. Un message « OpenCV actif: ... » doit apparaître dans la sortie de compilation.
#
# Option C — Ligne directement ci-dessous : décommentez OPENCV_DIR = ... et adaptez le chemin.
#
OPENCV_DIR = $$(OPENCV_DIR)
OPENCV_WORLD_LIB = $$(OPENCV_WORLD_LIB)
isEmpty(OPENCV_WORLD_LIB) {
    win32: OPENCV_WORLD_LIB = opencv_world4120
}
# Surcharge locale (chemins machine) — doit rester après les valeurs par défaut ci-dessus.
exists($$PWD/opencv_local.pri) {
    include($$PWD/opencv_local.pri)
}

# Copie des fichiers d’identifiants OAuth à côté de l’exécutable (Google Calendar).
exists($$PWD/google_credentials.json) {
    copy_google_creds.files = $$PWD/google_credentials.json
    copy_google_creds.path = $$OUT_PWD
    COPIES += copy_google_creds
}
exists($$PWD/client_secret.json) {
    copy_client_secret.files = $$PWD/client_secret.json
    copy_client_secret.path = $$OUT_PWD
    COPIES += copy_client_secret
}
# OPENCV_DIR = C:/opencv/build
!isEmpty(OPENCV_DIR) {
    INCLUDEPATH += $$OPENCV_DIR/include

    # MinGW ne peut pas lier les .lib MSVC. On n'active LEATHER_HAVE_OPENCV que si des libs MinGW sont disponibles.
    OPENCV_LINK_OK = 1
    win32-msvc* {
        OPENCV_LINK_OK = 0
        # Dossier lib MSVC : vc17 (VS2022), vc16 (VS2019), vc15 (VS2017) selon le paquet OpenCV Windows.
        # Surcharge possible dans opencv_local.pri : OPENCV_MSVC_LIBDIR = C:/chemin/vers/lib
        isEmpty(OPENCV_MSVC_LIBDIR) {
            exists($$OPENCV_DIR/x64/vc17/lib) {
                OPENCV_MSVC_LIBDIR = $$OPENCV_DIR/x64/vc17/lib
            } else:exists($$OPENCV_DIR/x64/vc16/lib) {
                OPENCV_MSVC_LIBDIR = $$OPENCV_DIR/x64/vc16/lib
            } else:exists($$OPENCV_DIR/x64/vc15/lib) {
                OPENCV_MSVC_LIBDIR = $$OPENCV_DIR/x64/vc15/lib
            }
        }
        !isEmpty(OPENCV_MSVC_LIBDIR) {
            exists($$OPENCV_MSVC_LIBDIR/$${OPENCV_WORLD_LIB}.lib) {
                OPENCV_LINK_OK = 1
            }
            exists($$OPENCV_MSVC_LIBDIR/$${OPENCV_WORLD_LIB}d.lib) {
                OPENCV_LINK_OK = 1
            }
        }
    }
    win32-g++ {
        OPENCV_LINK_OK = 0
        exists($$OPENCV_DIR/x64/mingw/lib) {
            OPENCV_LINK_OK = 1
        }
        !isEmpty(OPENCV_MINGW_LIBDIR) {
            OPENCV_LINK_OK = 1
        }
        !isEmpty(OPENCV_MINGW_LDFLAGS) {
            OPENCV_LINK_OK = 1
        }
    }

    equals(OPENCV_LINK_OK, 1) {
        message("OpenCV lie au binaire: $$OPENCV_DIR")
        win32-msvc*:!isEmpty(OPENCV_MSVC_LIBDIR) {
            message("OpenCV MSVC libdir: $$OPENCV_MSVC_LIBDIR")
        }
        DEFINES += LEATHER_HAVE_OPENCV
        win32-msvc* {
            CONFIG(debug, debug|release) {
                LIBS += -L$$OPENCV_MSVC_LIBDIR -l$${OPENCV_WORLD_LIB}d
            } else {
                LIBS += -L$$OPENCV_MSVC_LIBDIR -l$$OPENCV_WORLD_LIB
            }
        }
        win32-g++ {
            !isEmpty(OPENCV_MINGW_LDFLAGS) {
                LIBS += $$OPENCV_MINGW_LDFLAGS
            } else {
                !isEmpty(OPENCV_MINGW_LIBDIR) {
                    LIBS += -L$$OPENCV_MINGW_LIBDIR -l$${OPENCV_WORLD_LIB}
                } else {
                    LIBS += -L$$OPENCV_DIR/x64/mingw/lib -l$${OPENCV_WORLD_LIB}
                }
            }
        }
        OPENCV_FACE_LIB = $$(OPENCV_FACE_LIB)
        !isEmpty(OPENCV_FACE_LIB) {
            DEFINES += LEATHER_HAVE_OPENCV_FACE
            win32-msvc* {
                CONFIG(debug, debug|release) {
                    LIBS += -l$${OPENCV_FACE_LIB}d
                } else {
                    LIBS += -l$$OPENCV_FACE_LIB
                }
            }
            win32-g++ {
                LIBS += -l$$OPENCV_FACE_LIB
            }
        }
        SOURCES += faceauthmanager.cpp
        HEADERS += faceauthmanager.h

        # Runtime Windows/MinGW: copie automatique des DLL OpenCV a cote de l'exe.
        win32-g++ {
            OPENCV_MINGW_BINDIR = $$OPENCV_DIR/x64/mingw/bin
            !isEmpty(OPENCV_MINGW_LIBDIR) {
                OPENCV_MINGW_BINDIR = $$OPENCV_MINGW_LIBDIR/../bin
            }

            exists($$OPENCV_MINGW_BINDIR) {
                message("OpenCV DLL bindir: $$OPENCV_MINGW_BINDIR")
                QMAKE_POST_LINK += cmd /c copy /y \"$$OPENCV_MINGW_BINDIR\\libopencv*.dll\" \"$$OUT_PWD\\debug\\\" $$escape_expand(\\n\\t)
                QMAKE_POST_LINK += cmd /c copy /y \"$$OPENCV_MINGW_BINDIR\\libopencv*.dll\" \"$$OUT_PWD\\release\\\" $$escape_expand(\\n\\t)
                QMAKE_POST_LINK += cmd /c copy /y \"$$OPENCV_MINGW_BINDIR\\opencv_videoio_ffmpeg*.dll\" \"$$OUT_PWD\\debug\\\" $$escape_expand(\\n\\t)
                QMAKE_POST_LINK += cmd /c copy /y \"$$OPENCV_MINGW_BINDIR\\opencv_videoio_ffmpeg*.dll\" \"$$OUT_PWD\\release\\\" $$escape_expand(\\n\\t)
            } else {
                message("Attention: dossier DLL OpenCV introuvable: $$OPENCV_MINGW_BINDIR")
            }
        }
        # Runtime Windows/MSVC : copie opencv_world*.dll (sinon erreur au lancement : DLL introuvable).
        win32-msvc* {
            OPENCV_MSVC_BINDIR = $$replace(OPENCV_MSVC_LIBDIR, /lib, /bin)
            exists($$OPENCV_MSVC_BINDIR) {
                message("OpenCV MSVC DLL bindir: $$OPENCV_MSVC_BINDIR")
                QMAKE_POST_LINK += cmd /c copy /y \"$$OPENCV_MSVC_BINDIR\\opencv_world*.dll\" \"$$OUT_PWD\\debug\\\" $$escape_expand(\\n\\t)
                QMAKE_POST_LINK += cmd /c copy /y \"$$OPENCV_MSVC_BINDIR\\opencv_world*.dll\" \"$$OUT_PWD\\release\\\" $$escape_expand(\\n\\t)
            } else {
                message("Attention: dossier DLL OpenCV MSVC introuvable: $$OPENCV_MSVC_BINDIR — copiez manuellement depuis x64/vcXX/bin vers le dossier de leather.exe.")
            }
        }
    } else {
        win32-g++ {
            message("OpenCV: kit MinGW sans libs OpenCV MinGW — reconnaissance faciale desactivee (pas de erreur link). Utilisez le kit Qt MSVC 64-bit avec cette OpenCV, ou definissez OPENCV_MINGW_LIBDIR.")
        }
        win32-msvc* {
            message("OpenCV MSVC introuvable : verifiez OPENCV_DIR (dossier build avec include/) et OPENCV_WORLD_LIB (nom du .lib sans extension dans x64/vc17|vc16|vc15/lib). Dossier teste: $$OPENCV_MSVC_LIBDIR")
        }
    }
}

FORMS += \
    mainwindow.ui \
    settingswindow.ui

RESOURCES += \
    Resources/resources.qrc

# Copie le formulaire d'avis à côté de l'exécutable (déploiement / test local).
copy_satisfaction.files = $$PWD/Resources/satisfaction_avis_client.html \
    $$PWD/Resources/leather_survey_url.example.txt \
    $$PWD/leather_survey_url.txt
copy_satisfaction.path = $$OUT_PWD
COPIES += copy_satisfaction

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SOURCES += main.cpp \
           ../../qrcodegen.cpp \
           facelogindialog.cpp \
           assistantwindow.cpp \
           busyseasoncalendardialog.cpp \
           busyseasonservice.cpp \
           settingswindow.cpp \
           smtpsender.cpp \
           forgotpassworddialog.cpp \
           chatbotwindow.cpp \
           catalogueproduitswidget.cpp \
           produitcardwidget.cpp \
           produitdetaildialog.cpp \
           chatmanager.cpp \
           employe.cpp \
           imagemanager.cpp \
           mainwindow.cpp \
           GoogleCalendarService.cpp \
           matierepremiere.cpp \
           clientnotificationservice.cpp \
           whatsappbusinessservice.cpp \
           produit.cpp \
           client.cpp \
           commercestore.cpp \
           recommendationservice.cpp \
           chatbotservice.cpp \
           fournisseurapiservice.cpp \
           fournisseur.cpp \
           fournisseurstats.cpp \
           supplierchartwidgets.cpp

HEADERS += mainwindow.h \
           chatbotreplyformat.h \
           ../../qrcodegen.hpp \
           GoogleCalendarService.h \
           facelogindialog.h \
           assistantwindow.h \
           busyseasoncalendardialog.h \
           busyseasonevent.h \
           busyseasonservice.h \
           settingswindow.h \
           smtpsender.h \
           forgotpassworddialog.h \
           chatbotwindow.h \
           catalogueproduitswidget.h \
           catalogueproduitmodel.h \
           produitcardwidget.h \
           produitdetaildialog.h \
           chatmanager.h \
           employe.h \
           imagemanager.h \
           matierepremiere.h \
           clientnotificationservice.h \
           whatsappbusinessservice.h \
           produit.h \
           client.h \
           commercestore.h \
           recommendationservice.h \
           chatbotservice.h \
           fournisseurapiservice.h \
           fournisseur.h \
           fournisseurstats.h \
           supplierchartwidgets.h

## OpenCV local forcé désactivé:
## utilisez plutôt OPENCV_DIR / opencv_local.pri ci-dessus.

TRANSLATIONS += translations/leather_en.ts translations/leather_ar.ts

DISTFILES += \
    ../../../../Downloads/QR-Code-generator-master.zip