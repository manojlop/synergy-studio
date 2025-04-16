#include "../include/SslClient.h"

SslClient::SslClient(QObject *parent) : QObject(parent) {
  // Connect socket's signals to our slots before connecting
  // This ensures we catch all events, including errors during connection/handshake
  connect(&m_socket, &QAbstractSocket::connected, this, &SslClient::onConnected);
  connect(&m_socket, &QAbstractSocket::disconnected, this, &SslClient::onDisconnected);
  connect(&m_socket, &QSslSocket::readyRead, this, &SslClient::onReadyRead);
  connect(&m_socket, &QSslSocket::sslErrors, this, &SslClient::onSslErrors);
  connect(&m_socket, &QSslSocket::encrypted, this, &SslClient::onEncrypted);
  // Catch generat errors like 'Connection refused'
  connect(&m_socket, &QAbstractSocket::errorOccurred, this, &SslClient::onErrorOccurred);

  /* Security: We have two options in regards to certifications
  - Add server certification as CA
    Instead of ingoring errors, explicitly tell the client to trust the server's specific 
    self-signed certificate.
    Much safer than ignoring all errors
  - Prepare to ignore errors
    Set a flag to ignore specific SSL errors later in the onSslErrors slot -> NOT RECCOMENDED
  */
  
  QString serverCertPath = "cert.pem"; // Path to server certification path
  QList<QSslCertificate> serverCerts = QSslCertificate::fromPath(serverCertPath, QSsl::Pem);
  if(serverCerts.isEmpty()){
    qCritical() << "Client: CRITICAL FAILURE - Could not load server certificate from:" << serverCertPath;
    qCritical() << "Client: Cannot proceed securely. Please ensure cert.pem exists and is valid.";
    // TBD : How to handle this?
    // Option 1: Prevent connection attempts later
    m_socket.setProperty("initializationFailed", true); // Use a dynamic property
    // Option 2: Throw (if appropriate for app structure)
    // throw std::runtime_error("Failed to load server CA cert");
  } else {
    // Todo : cleanup
    QSslConfiguration config = m_socket.sslConfiguration(); // get current configuration
    QList<QSslCertificate> caCerts = config.caCertificates();
    caCerts.append(serverCerts.first()); // Add servers cert to the list of trusted CAs
    config.setCaCertificates(caCerts);
    config.setProtocol(QSsl::TlsV1_2OrLater);
    m_socket.setSslConfiguration(config);
    qInfo() << "Client: Configured to trust server's self signed certificate.";
  }
}

// Initiate connection
void SslClient::connectToServer(const QString &host, quint16 port){
  if (m_socket.property("initializationFailed").toBool()) {
    qCritical() << "Client: Cannot connect, initialization failed (CA cert missing?).";
    return;
  }
  qInfo() << "Client: Attempting to connect to: " << host << ":" << port << " using SSL...";
  // Initiate TCP connection and start SSL/TLS handshake immediately after TCP connects
  // m_socket.connectToHostEncrypted(host, port);
  // ^ Doesn't strictly verify that the hostname/IP in the certificate's Common Name (CN)
  // or Subject Alternative Name (SAN) matches the host connected to
  // If server's name is to be verified agains the certificate:
  // Ensure self-signed certificate is generated with the correct CN or SAN
  m_socket.connectToHostEncrypted(host, port, host);
}

// Slot: Connected (TCP level)
void SslClient::onConnected(){
  // Emitted before SSL handshake is completed, 'encrypted' indicates successfull secure connection
  qInfo() << "Client: TCP connection established to" << m_socket.peerName() << ":" << m_socket.peerPort();
  qInfo() << "Client: Waiting for SSL handshake to complete...";
}

// Slot: Encrypted (SSL handshake completed)
void SslClient::onEncrypted(){
  qInfo() << "Client: SSL Handshake successful! Connection is now encrypted.";
  qInfo() << "Client: Cipher used:" << m_socket.sessionCipher().name();
  // Now it's safe to send application data securely.
  // sendMessage("Hello Secure Server!");
  SynergyProtocol::Message_Join_Session_Request join_msg {QString(""), true, QString("Client")};
  sendMessage(QJsonDocument(join_msg.toJSon()).toJson());
}

// Slot: Disconnected
void SslClient::onDisconnected(){
  qInfo() << "Client: Disconnected from server.";
  // TBD : Recconection or notify the user
}

// Slot: Data Receive
void SslClient::onReadyRead(){
  // Explanation: Read encrypted data. Qt decrypts it automatically.
  QByteArray data = m_socket.readAll();
  qInfo() << "Client: Received from server:" << data;

  // Example: Send another message after receiving
  // static int count = 0;
  // if(count++ < 3) {
  //     QTimer::singleShot(1000, this, [this](){ sendMessage("Another message!"); });
  // } else {
  //     m_socket.close();
  // }
}

// Slot: SSL Errors
// Explanation: This is where we handle certificate validation errors, etc.
void SslClient::onSslErrors(const QList<QSslError> &errors){
    qWarning() << "Client: SSL Errors occurred:";
    QList<QSslError> ignorableErrors; // List to hold errors we might ignore

    for (const QSslError &error : errors) {
        qWarning() << "-" << error.errorString();
    }

    if (!errors.isEmpty()) {
      // If there were other errors we didn't choose to ignore
      qWarning() << "Client: Unignored SSL errors occurred. Connection will likely fail.";
      // m_socket.abort(); // Optionally force close immediately
    }
}

// Slot: General Socket Errors
void SslClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
  // This catches errors *not* related to SSL specifically, like connection refused.
  Q_UNUSED(socketError); // Mark parameter as unused if not needed directly
  qCritical() << "Client: Socket error occurred:" << m_socket.errorString();
}

void SslClient::sendMessage(const QString &message){
  if(m_socket.state() == QAbstractSocket::ConnectedState && m_socket.isEncrypted()) {
    qInfo() << "Client: Sending message: " << message;
    // Qt handles encryption
    m_socket.write(message.toUtf8() + "\n");
    // m_socket.flush(); // Usually not required
  } else {
    qWarning() << "Client: Cannot send message, socket not connected or not encrypted.";
  }
}