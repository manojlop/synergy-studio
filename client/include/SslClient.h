#ifndef __SSL_CLIENT_H__
#define __SSL_CLIENT_H__

#include <QObject>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslError>
#include <QHostAddress>
#include <QDebug>
#include <QFile> // For loading ca certificate
#include <QSslCipher>

#include "synergy_protocol/protocol.h"

class SslClient : public QObject {
  Q_OBJECT
public:
  explicit SslClient(QObject *parent = nullptr);
  void connectToServer(const QString &host = QStringLiteral("localhost"), quint16 port = 12345);
  void sendMessage(const QString &message);

private slots:
  void onConnected(); // Standard socket connected signal, before encryption
  void onDisconnected();
  void onReadyRead();
  void onSslErrors(const QList<QSslError> &errors); // SSL errors
  void onEncrypted(); // handshake successful
  void onErrorOccurred(QAbstractSocket::SocketError socketError); // general socket errors

private:
  QSslSocket m_socket;

};

#endif