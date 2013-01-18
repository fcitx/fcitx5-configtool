#include <QApplication>

#include "configwidget.h"
#include "global.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    FcitxConfigFileDesc* cfdesc = Fcitx::Global::instance()->GetConfigDesc("config.desc");

    Fcitx::ConfigWidget configPage(cfdesc, "", "config");
    configPage.load();
    configPage.show();
    a.exec();
    return 0;
}