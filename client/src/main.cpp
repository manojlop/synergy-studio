#include <QApplication> // Needs Qt6::Widgets & Qt6::Gui
#include <QWidget>      // Needs Qt6::Widgets & Qt6::Gui
#include <QDebug>       // Needs Qt6::Core
#include <synergy_protocol/protocol.h> // Needs common_protocol include path

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Tests linking Qt Widgets/Gui
    qDebug() << "SynergyStudioClient starting...";
    SynergyProtocol::Dummy d; // Tests linking common_protocol
    (void)d; // Avoid unused variable warning

    QWidget w; // Tests linking a basic widget class
    // w.show(); // Don't show window for this test

    // Normally you would create MainWindow here
    // return a.exec(); // Don't start event loop

    return 0; // Exit immediately
}