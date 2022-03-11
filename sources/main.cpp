#include <QApplication>
#include <QCommandLineParser>
#include "detwindow.h"
#include "mainwidget.h"
#include "task.h"
int main(int argc, char *argv[])
{
//    Q_IMPORT_PLUGIN( qtiff );
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(DetWindow::tr("Image Labeler"));
    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(DetWindow::tr("[file]"), DetWindow::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());

//    MainWindow mainWindow;
//    mainWindow.show();
    Task w;
    w.show();

    return app.exec();
}
