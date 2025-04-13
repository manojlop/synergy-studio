#ifndef __SSL_SERVER_H__
#define __SSL_SERVER_H__

#include <QSslServer>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslCertificate>
#include <QList>
#include <QFile>
#include <QDebug>

class SslServer : public QSslServer {
  Q_OBJECT
public:
  explicit SslServer(QObject *parent = nullptr);
  bool startListening(const QHostAddress &address = QHostAddress::LocalHost, quint16 port = 12345); // Todo : change this into actual parameters

protected:
  // Override incomngConnection to handle SSL sockets
  void incomingConnection(qintptr socketDescriptor) override;

private slots:
  void onReadyRead();
  void onDisconnected();
  void onSslErrors(const QList<QSslError> &errors);
  void onEncrypted(); // Slot notified when handshake is complete

private:
  QSslConfiguration m_sslConfiguration;
  QList<QSslSocket*> m_clients; // Keep track of connected clients

  bool loadCertAndKey(const QString &certPath, const QString &keyPath);
};

#endif