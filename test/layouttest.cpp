#include <QApplication>
#include <QMainWindow>
#include "KeyboardLayoutWidget.h"
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QMainWindow mainWindow;
    KeyboardLayoutWidget widget;

    mainWindow.setCentralWidget(&widget);
    mainWindow.show();
    //widget.setLayout("de", QString());
    app.exec();
    return 0;
}