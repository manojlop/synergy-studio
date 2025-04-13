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
    qWarning() << "Client: Failed to load server certificate from: " << serverCertPath;
    // TBD : How to handle this?
  } else {
    QSslConfiguration config = m_socket.sslConfiguration(); // get current configuration
    QList<QSslCertificate> caCerts = config.caCertificates();
    caCerts.append(serverCerts.first()); // Add servers cert to the list of trusted CAs
    config.setCaCertificates(caCerts);
    m_socket.setSslConfiguration(config);
    qInfo() << "Client: Configured to trust server's self signed certificate.";
    m_ignoreSslErrors = false; 
  }

  // qWarning() << "Client: Configured to potentially ignore SSL errors (Example mode)."
  //           << "Reccomend adding server certificate to CA list instead (see code comments)";
  // m_ignoreSslErrors = true;

  // protocol client-side coul be set if needed, but usually defaults are fine.
  // QSslConfiguration config = m_socket.sslConfiguration();
  // config.setProtocol(QSsl::TlsV1_2OrLater);
  // m_socket.setSslConfiguration(config);
}

// Initiate connection
void SslClient::connectToServer(const QString &host, quint16 port){
  qInfo() << "Client: Attempting to connect to: " << host << ":" << port << " using SSL...";
  // Initiate TCP connection and start SSL/TLS handshake immediately after TCP connects
  m_socket.connectToHostEncrypted(host, port);
  // If server's name is to be verified agains the certificate:
  // m_socket.connectToHostEncrypted(host, port, host);
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
  sendMessage("Hello Secure Server!");
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
        // Security Critical: Handling Specific Errors
        // Explanation: Check if the error is one we expect due to the self-signed cert.
        if (m_ignoreSslErrors && (error.error() == QSslError::SelfSignedCertificate ||
                error.error() == QSslError::SelfSignedCertificateInChain ||
                error.error() == QSslError::UnableToGetLocalIssuerCertificate ||
                 error.error() == QSslError::CertificateUntrusted) ){
            qWarning() << "Client: --> Ignoring expected self-signed certificate error (Example Mode).";
            ignorableErrors.append(error);
        }
    }

    // Security Critical: Calling ignoreSslErrors 
    // Explanation: If we decided to ignore certain errors (based on our flag and checks above),
    // we MUST call ignoreSslErrors() with the list of errors to ignore. Otherwise,
    // the handshake will fail and the socket will likely disconnect.
    //
    // * THIS IS THE DANGEROUS PART IN PRODUCTION *
    // Blindly ignoring errors defeats the purpose of SSL authentication.
    // Only ignore errors if you have explicitly validated the certificate out-of-band
    // or if you are using the "Add Server Cert as CA" method (Option 1), in which case
    // you shouldn't get these errors and wouldn't need to ignore them.
    if (!ignorableErrors.isEmpty()) {
      m_socket.ignoreSslErrors(ignorableErrors);
      qWarning() << "Client: Called ignoreSslErrors() for specific self-signed related errors.";
    } else if (!errors.isEmpty()) {
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