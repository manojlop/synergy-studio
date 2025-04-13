#include <QApplication> // Needs Qt6::Widgets & Qt6::Gui
#include <QWidget>      // Needs Qt6::Widgets & Qt6::Gui
#include <QDebug>       // Needs Qt6::Core

#include <QDebug>
#include <QSslSocket>
#include <QFile>
#include <iostream>
#include <QTimer>

#include "../include/SslClient.h"

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
  if (!QFile::exists("cert.pem")) {
      qCritical() << "Error: 'cert.pem' not found in the application directory.";
      qCritical() << "Please generate them using the openssl command provided in the instructions.";
      return 1;
  }

  SslClient client;

  // Connect Client (with a slight delay)
  // Using QTimer::singleShot to delay the client's connection attempt slightly.
  // This gives the server a moment to fully start up and begin listening.
  // In more complex apps, you might have better synchronization mechanisms.
  QTimer::singleShot(500, [&client]() { // 500 ms delay
      client.connectToServer("localhost", 12345);
  });

  // Send another message from client after a few seconds
  QTimer::singleShot(4000, [&client]() { // 4 seconds delay
      client.sendMessage("Second message from client!");
  });


  // Close down after some time for automated testing
  // QTimer::singleShot(10000, &a, &QCoreApplication::quit); // Quit after 10 seconds


  qInfo() << "Starting event loop...";
  return a.exec(); // Start the Qt event loop
}