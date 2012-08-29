#include <QCoreApplication>
#include <QTimer>

#include "dummyconfig.h"
#include "configdescmanager.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    FcitxConfigFileDesc* cfdesc = Fcitx::ConfigDescManager::instance()->GetConfigDesc("config.desc");
    Fcitx::DummyConfig* dummyConfig = new Fcitx::DummyConfig(cfdesc);
    dummyConfig->load(NULL);
    delete dummyConfig;

    Fcitx::ConfigDescManager::deInit();

    QTimer::singleShot(0, &a, SLOT(quit()));
    a.exec();

    return 0;
}