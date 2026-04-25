#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#ifdef LEATHER_HAVE_WEBENGINE
#include <QMainWindow>
#include <QVBoxLayout>
#include <QtWebEngineWidgets/QWebEngineView>
#endif

/// Charge un fichier .env (UTF-8) : lignes KEY=value, # commentaires.
/// Ne remplace pas une variable deja definie dans l environnement du processus.
static void loadOptionalDotEnvFile(const QString &absolutePath)
{
    QFile f(absolutePath);
    if (!f.exists() || !f.open(QIODevice::ReadOnly))
        return;
    const QByteArray data = f.readAll();
    for (QByteArray rawLine : data.split('\n')) {
        if (rawLine.endsWith('\r'))
            rawLine.chop(1);
        const QString line = QString::fromUtf8(rawLine).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;
        const int eq = line.indexOf(QLatin1Char('='));
        if (eq <= 0)
            continue;
        const QString key = line.left(eq).trimmed();
        QString value = line.mid(eq + 1).trimmed();
        if (value.size() >= 2
            && ((value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')))
                || (value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\''))))) {
            value = value.mid(1, value.size() - 2);
        }
        if (key.isEmpty())
            continue;
        const QByteArray k = key.toUtf8();
        if (!qgetenv(k.constData()).isEmpty())
            continue;
        qputenv(k.constData(), value.toUtf8());
    }
}

/// Cherche .env a cote de l executable puis dans le repertoire de travail courant (ex. projet sous Qt Creator).
static void loadOptionalDotEnv()
{
    loadOptionalDotEnvFile(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral(".env")));
    loadOptionalDotEnvFile(QDir::current().filePath(QStringLiteral(".env")));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    loadOptionalDotEnv();
    a.setOrganizationName(QStringLiteral("RoyalLeatherHouse"));
    a.setApplicationName(QStringLiteral("LeatherERP"));
    a.setApplicationDisplayName(QStringLiteral("Royal Leather House"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Royal Leather House"));
    parser.addHelpOption();
    QCommandLineOption webEngineTestOpt(
        QStringList() << QStringLiteral("webengine-test"),
        QStringLiteral("Ouvre un test minimal QWebEngineView (OpenStreetMap)."));
    parser.addOption(webEngineTestOpt);
    parser.process(a);

#ifdef LEATHER_HAVE_WEBENGINE
    if (parser.isSet(webEngineTestOpt)) {
        auto *win = new QMainWindow();
        win->setWindowTitle(QStringLiteral("WebEngine Test - OpenStreetMap"));
        win->resize(1200, 780);
        auto *central = new QWidget(win);
        auto *layout = new QVBoxLayout(central);
        layout->setContentsMargins(0, 0, 0, 0);
        auto *view = new QWebEngineView(central);
        layout->addWidget(view);
        win->setCentralWidget(central);

        QObject::connect(view, &QWebEngineView::loadFinished, view, [](bool ok) {
            qDebug() << "[WebEngineTest] loadFinished ok =" << ok;
        });
        QObject::connect(view, &QWebEngineView::urlChanged, view, [](const QUrl &u) {
            qDebug() << "[WebEngineTest] urlChanged =" << u;
        });

        view->setUrl(QUrl(QStringLiteral("https://www.openstreetmap.org")));
        win->show();
        return a.exec();
    }
#else
    if (parser.isSet(webEngineTestOpt)) {
        qDebug() << "[WebEngineTest] LEATHER_HAVE_WEBENGINE non défini pour ce kit.";
    }
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
