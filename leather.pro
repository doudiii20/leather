QT += sql network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


FORMS += \
    mainwindow.ui

RESOURCES += \
    Resources/resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SOURCES += main.cpp \
           mainwindow.cpp \
           client.cpp \
           commercestore.cpp \
           recommendationservice.cpp \
           chatbotservice.cpp \
           fournisseur.cpp \
           fournisseurstats.cpp \
           supplierchartwidgets.cpp

HEADERS += mainwindow.h \
           client.h \
           commercestore.h \
           recommendationservice.h \
           chatbotservice.h \
           fournisseur.h \
           fournisseurstats.h \
           supplierchartwidgets.h

