#include <QApplication>
#include <QFile>
#include <QIcon>
#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Strategix");
    app.setApplicationDisplayName("Strategix - Board Games");
    app.setApplicationVersion("2.0.1");
    app.setOrganizationName("Strategix");

    // Load icon if available
    QIcon icon(":/Quarto.ico");
    if (icon.isNull()) {
        icon = QIcon(":/strategix.png");
    }
    if (!icon.isNull()) {
        app.setWindowIcon(icon);
    }

    // Load stylesheet
    QFile qssFile(":/src/style/style.qss");
    if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(qssFile.readAll());
        qssFile.close();
    }

    Strategix::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
