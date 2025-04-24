#include <QCoreApplication> // Needs Qt6::Core
#include <QDebug>           // Needs Qt6::Core
#include <QDebug>
#include <QSslSocket>
#include <QFile>
#include <iostream>

#include "../include/SslServer.h"

constexpr quint16 _port = 10345;


int main(int argc, char *argv[]){
  QCoreApplication a(argc, argv); 

  qInfo() << "Synergy Studio - SSL Test";
  qInfo() << "Using Qt Version:" << QT_VERSION_STR;
  qInfo() << "Using SSL Library:" << 
  QSslSocket::sslLibraryBuildVersionString();

  // Check for SSL Support
  if (!QSslSocket::supportsSsl()) {
    qCritical() << "SSL is not supported by this Qt build or system configuration. Cannot run.";
    qCritical() << "Build version:" << QSslSocket::sslLibraryBuildVersionString();
    qCritical() << "Runtime version:" << QSslSocket::sslLibraryVersionString();
    // Common issues: OpenSSL libraries not found at runtime, or Qt built without SSL support.
    return 1; // Indicate error
  } else {
      qInfo() << "SSL Support Detected."; 
  }

  // Certificate File Check
  // Safety: Basic check if the cert/key files exist before starting.
  if (!QFile::exists("cert.pem") || !QFile::exists("key.pem")) {
      qCritical() << "Error: 'cert.pem' or 'key.pem' not found in the application directory.";
      qCritical() << "Please generate them using the openssl command provided in the instructions.";
      return 1;
  }

  SslServer server;
  if(!server.startListening(QHostAddress::LocalHost, 12345)) {
    qCritical("Server failed to start. Exiting");
    return 1;
  }

  qInfo() << "Starting event loop...";
  return a.exec(); // Start the Qt event loop
}