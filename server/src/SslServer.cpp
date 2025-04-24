#include "../include/SslServer.h"

#include <QCoreApplication> // for error checking

SslServer::SslServer(QObject *parent) : QSslServer(parent) {
  // Setting up SSL configuration -> defining rules for ssl connections
  m_sslConfiguration = QSslConfiguration::defaultConfiguration();

  /*
  ------------------------------------------------------------
  ------------------- Critical for security ------------------
  - Loading certificate and key
  We load server's identity(certificate) and the secret key 
  that is needed to prove that identity and enable encryption
  Safety: Ensure 'cert.perm' and 'key.perm' are redable by app
  but 'key.perm' should have strict file permissions in real
  scenario
  Crucial error handling
  To generate for local testing:
  openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes
  ------------------------------------------------------------
  */
  if(!loadCertAndKey("cert.pem", "key.pem")){
    qCritical() << "Failed to load certificate or key file. Server cannot start securely.";
    // Exit and disable SSL feature
    // QCoreApplication::exit(1);
    // TBD : Two phase init or return with factory, as object wont be constructed
    throw std::runtime_error("SSL init failed");
    return;
  }

  // QSslConfiguration config = QSslConfiguration::defaultConfiguration();
  // config.setLocalCertificate(m_sslConfiguration.localCertificate()); // use loaded certificate
  // config.setPrivateKey(m_sslConfiguration.privateKey()); // use loaded key

  // Setting allowed SSL/TLS protocol
  // Using newer (1.2 or 1.3) prevents known vulnerabilites
  m_sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);

  // TODO: Consider explicitly setting strong cipher suites if needed later.
  // m_sslConfiguration.setCiphers(...);

  QSslConfiguration::setDefaultConfiguration(m_sslConfiguration);

  qInfo() << "Server SSL configuration prepared";
}

bool SslServer::loadCertAndKey(const QString &certPath, const QString &keyPath){
  // Load cerificate
  QFile certFile(certPath);
  if (!certFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Could not open certificate file: " << certPath << certFile.errorString();
    return false;
  }  

  QSslCertificate certificate(&certFile, QSsl::Pem);
  // Todo : RAII with custom file wrapper
  certFile.close(); // Close the file as soon as possible!!! 

  if(certificate.isNull()){
    qWarning() << "Certificate file is invalid or empty" << certPath;
    return false;
  }

  // Load private key
  QFile keyFile(keyPath);
  // Todo : In production, key file permissions should be highly restricted
  if(!keyFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Could not open private key file: " << keyPath << keyFile.errorString();
    return false;
  }

  /*
  Load the key here.
  We are using RSA, as we generated RSA key
  QSsl::Pem is file format. Null password handler used as key isn't encrypted
  If key was password protected, pass a labmda/function here to provide a password
  Avoid hardcoding passwords (Secure storage or prompts)
  Todo : Implement with secure storage
  */
  QSslKey privateKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, QByteArray()); // empty password
  keyFile.close();

  if (privateKey.isNull()){
    qWarning() << "Private key file is invalid or empty:" << keyPath;
    return false;
  }

  // Storing them in our member configuration object
  m_sslConfiguration.setLocalCertificate(certificate);
  m_sslConfiguration.setPrivateKey(privateKey);
  qInfo() << "Successfully opened certificate and private key";
  return true;
}

bool SslServer::startListening(const QHostAddress &address, quint16 port){
  if(!this->listen(address, port)) {
    qCritical() << "Server failed to start listening:" << errorString();
    return false;
  }
  qInfo() << "Server listening on: " << serverAddress().toString() << ":" << serverPort();
  return true;
}

/*
------------------------------------------------------------------
------------------ Handling incoming connections -----------------
Called by QSslServer when a new client tries to connect
We override it to create a QSslSocket instead of plain QTcpSocket
------------------------------------------------------------------
*/
void SslServer::incomingConnection(qintptr socketDescriptor){
  qInfo() << "Incoming connection, creating QSslSocket...";

  // Ssl socket for this specific connection
  QSslSocket *sslSocket = new QSslSocket(this); // parent is this for basic object ownership

  // Setting socket descriptor obtained from base class
  // Associates OS-level socket with our QSslSocket object
  if(!sslSocket->setSocketDescriptor(socketDescriptor)){
    qWarning() << "Failed to set socket descriptor: " << sslSocket->errorString();
    return;
  }

  /* Security critial: Applying SSL configration & starting handshake
  Apply SSL setting (cert, key, protocol) we prepared earlier
  We can also set specific configuration per-socket if needed
  */
  sslSocket->setSslConfiguration(m_sslConfiguration); // usually not needed if default is set

  // Connect signals from new socket to our server slots before starting encryption, so we can handle errors/events during handshake
  connect(sslSocket, &QSslSocket::readyRead, this, &SslServer::onReadyRead);
  connect(sslSocket, &QAbstractSocket::disconnected, this, &SslServer::onDisconnected);
  // Crucial signal for handling SSl-specific errors durng handshake or later
  connect(sslSocket, &QSslSocket::sslErrors, this, &SslServer::onSslErrors);
  // Signal emitted when SSL handshake is successfully completed
  connect(sslSocket, &QSslSocket::encrypted, this, &SslServer::onEncrypted);
  
  /*
  Start server-side SSL handshake. 
  Qt will now handle complex back-and-forth negotiation with client using configured settings
  This is asynch operation, 'encrypted' or 'sslErrors' signal will follow
  */
  sslSocket->startServerEncryption();

  // Add socket to our list after setup seems okay
  m_clients.insert(socketDescriptor, sslSocket);

  qInfo() << "QSslSocket created for descriptor: " << socketDescriptor << "starting encryption...";
}

// Slots - Data Recieved
void SslServer::onReadyRead(){
  // Get socket that emitted signal
  QSslSocket *clientSocket = qobject_cast<QSslSocket*>(sender()); // use qobject_cast for safe casting
    if (!clientSocket) return; // Should not happen in connected correctly

  // Read all available data from encrypted socket
  QByteArray data = clientSocket->readAll();
  qInfo() << "Recieved from client: " << data;
  auto json_doc = QJsonDocument::fromJson(data);
  if(json_doc.isNull()){
    qDebug()<<"Failed to create JSON doc.";
    exit(2);
  }
  if(!json_doc.isObject()){
    qDebug()<<"JSON is not an object.";
    exit(3);
  }

  std::unique_ptr<SynergyProtocol::Message_Base> message 
    = SynergyProtocol::MessageFactory::instance().createMessage(json_doc.object());

  if(message) {
    qInfo() << "Server received message type:" << SynergyProtocol::messageTypeToString(message->type());
    // Echo data back to client
    QString response = "Server recieved command: " + SynergyProtocol::messageTypeToString(message->type()) + " from " + ((message->toJSon())["payload"].toObject()["username"].toString());
    // Write data back. Qt handles encryption automatically
    clientSocket->write(response.toUtf8());
  } else {
    qWarning() << "Factory failed to create message object or parse JSON payload from client:" << clientSocket->peerAddress();
  }

}

// Slots - Client Disconnected
void SslServer::onDisconnected(){
  QSslSocket *clientSocket = qobject_cast<QSslSocket*>(sender());
    if (!clientSocket) return;

  qInfo() << "Client disconnected: " << clientSocket->peerAddress() << ":" << clientSocket->peerPort();

  // Remove socket from tracking list
  m_clients.remove(clientSocket->socketDescriptor());

  // Use deleteLater to safely remove QObject from within a slot connected to one of its signals
  // This schedules deletion after the event loop returns
  clientSocket->deleteLater();
  // #endif
}

// Slot SSl Error occured
// Catches errors during SSL handshake or later encryption issues
// Vital for security
void SslServer::onSslErrors(const QList<QSslError> &errors){
  QSslSocket *clientSocket = qobject_cast<QSslSocket*>(sender());
    if (!clientSocket) return;
  
  qWarning() << "SSL errors occured for a client connection: ";
  for(const QSslError &error : errors){
    qWarning() << "-" << error.errorString();
  }

  /* Security: Handling Self-Signed certificates
  For this example, using self-signed certificate, the client will likely report an 
  "Unable to get local issuer certificate" or "Certificate untrusted" error
  because certificate isn't signed bt a known CA

  In production, we CANNOT ignore the errors
  Proper handling invlolves:
  1. Using certificates signed by a trusted CA
  2. Configuring the client to explicitly trust you self-signed CA or specific server certificate
  3. Carefully evaluate which errors are acceptable to ingore(if any)

  For localhost example, we can chose to ignore self-signed error on client side. 
  Server typically doesn't ingore errors unless doing client certificate auth 
  Here, we just log errors, if they occur, handshake might fail and 'encrypted' signal won't be emitted
  */

  // clientSocket->abort(); // To disconnect on any error, forcefully close connection
}

// Slot: Connection encrypted
void SslServer::onEncrypted(){
  QSslSocket *clientSocket = qobject_cast<QSslSocket*>(sender());
    if (!clientSocket) return;

  qInfo() << "Connection successfully encrypted for:" << clientSocket->peerAddress() << ":" << clientSocket->peerPort();

  // Connection is now secure, ready for application data exchange.
  // Sending a welcome message
  clientSocket->write("Welcome! Connection is secure.\n");
}
