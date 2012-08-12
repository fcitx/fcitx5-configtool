#include <QApplication>

#include "configwidget.h"
#include "configdescmanager.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    FcitxConfigFileDesc* cfdesc = Fcitx::ConfigDescManager::instance()->GetConfigDesc("config.desc");

    Fcitx::ConfigWidget configPage(cfdesc, "", "config");
    configPage.load();
    configPage.show();
    a.exec();
    return 0;
}