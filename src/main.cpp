#include <QApplication>
#include "MainWindow.h"


int main(int argc, char *argv[])
{
    // 1. The Manager: QApplication
    // It creates the "Event Loop" (the heartbeat of the app).
    // It handles mouse clicks, window resizing, etc.
    QApplication app(argc, argv);

    // Create and show our new class
    MainWindow w;
    w.show();

    // 5. Enter the Loop
    // This pauses main() here. It won't return until the window is closed.
    return app.exec();
}