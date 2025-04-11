#include <QCoreApplication> // Needs Qt6::Core
#include <QDebug>           // Needs Qt6::Core
#include <synergy_protocol/protocol.h> // Needs common_protocol include path

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv); // Tests linking Qt Core
    qDebug() << "SynergyStudioServer starting...";
    SynergyProtocol::Dummy d; // Tests linking common_protocol
    (void)d; // Avoid unused variable warning

    // Normally you would start the server logic here
    // return a.exec(); // Don't start event loop for this test

    return 0; // Exit immediately after testing includes/linking
}