#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application info
    QApplication::setApplicationName("DOT Graph Viewer");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("DotViewer");
    
    // Use Fusion style for consistent look across platforms
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    MainWindow* window;
    
    // If a file is passed as argument, load it directly
    if (argc > 1) {
        window = new MainWindow(QString::fromLocal8Bit(argv[1]));
    } else {
        window = new MainWindow();
    }
    
    window->show();
    
    return app.exec();
}
