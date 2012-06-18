#include <QApplication>
#include "KeyboardLayoutWidget.h"
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    KeyboardLayoutWidget widget;
    widget.show();
    app.exec();
    return 0;
}