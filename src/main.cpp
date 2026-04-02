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
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
