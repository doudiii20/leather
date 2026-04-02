#include "smtp.h"
#include <QFileInfo>
#include <QByteArray>

Smtp::Smtp(const QString &user, const QString &pass, const QString &host, int port, int timeout)
{    
    this->user = user;
    this->pass = pass;
    this->host = host;
    this->port = port;
    this->timeout = timeout;

    socket = new QSslSocket(this);

    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(errorReceived(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void Smtp::sendMail(const QString &from, const QString &to, const QString &subject, const QString &body)
{
    this->from = from;
    rcpt = to;
    message = "To: " + to + "\n";
    message.append("From: " + from + "\n");
    message.append("Subject: " + subject + "\n\n");
    message.append(body);
    message.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
    message.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));

    state = Init;
    socket->connectToHostEncrypted(host, port);
    if (!socket->waitForConnected(timeout)) {
        emit status("Echec de connexion au serveur.");
    }
}

Smtp::~Smtp()
{
    delete t;
    delete socket;
}

void Smtp::stateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "Smtp stateChanged " << socketState;
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Smtp error " << socketError;
}

void Smtp::disconnected()
{
    qDebug() << "Smtp disconnected";
    qDebug() << "Email sent context reset";
}

void Smtp::connected()
{    
    qDebug() << "Smtp connected";
}

void Smtp::readyRead()
{
    qDebug() << "Smtp readyRead";
    QString responseLine;
    do {
        responseLine = socket->readLine();
        response += responseLine;
    } while (socket->canReadLine() && responseLine[3] != ' ');

    responseLine.truncate(3);

    qDebug() << "Server response: " << response;

    if (state == Init && responseLine == "220") {
        t = new QTextStream(socket);
        *t << "EHLO localhost" << "\r\n";
        t->flush();
        state = HandShake;
    }
    else if (state == HandShake && responseLine == "250") {
        *t << "AUTH LOGIN" << "\r\n";
        t->flush();
        state = Auth;
    }
    else if (state == Auth && responseLine == "334") {
        *t << QByteArray().append(user.toUtf8()).toBase64() << "\r\n";
        t->flush();
        state = Pass;
    }
    else if (state == Pass && responseLine == "334") {
        *t << QByteArray().append(pass.toUtf8()).toBase64() << "\r\n";
        t->flush();
        state = Mail;
    }
    else if (state == Mail && responseLine == "235") {
        qDebug() << "Authentification reussie !";
        *t << "MAIL FROM:<" << from << ">" << "\r\n";
        t->flush();
        state = Rcpt;
    }
    else if (state == Rcpt && responseLine == "250") {
        qDebug() << "Mail FROM accepte !";
        *t << "RCPT TO:<" << rcpt << ">" << "\r\n";
        t->flush();
        state = Data;
    }
    else if (state == Data && responseLine == "250") {
        qDebug() << "Rcpt TO accepte !";
        *t << "DATA\r\n";
        t->flush();
        state = Body;
    }
    else if (state == Body && responseLine == "354") {
        qDebug() << "Envoi du corps du message...";
        *t << message << "\r\n.\r\n";
        t->flush();
        state = Quit;
    }
    else if (state == Quit && responseLine == "250") {
        qDebug() << "Message accepte par le serveur !";
        *t << "QUIT\r\n";
        t->flush();
        emit status("Message envoye avec succes.");
        state = Close;
    }
    else if (state == Close) {
        socket->close();
    }
    else {
        qDebug() << "ERROR or unexpected response in state: " << state << " Response: " << response;
        state = Close;
        emit status("Erreur SMTP : " + response);
    }
    response = "";
}
