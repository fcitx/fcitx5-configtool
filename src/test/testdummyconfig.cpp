#include <QCoreApplication>
#include <QTimer>

#include "dummyconfig.h"
#include "global.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    FcitxConfigFileDesc* cfdesc = Fcitx::Global::instance()->GetConfigDesc("config.desc");
    Fcitx::DummyConfig* dummyConfig = new Fcitx::DummyConfig(cfdesc);
    dummyConfig->load(NULL);
    delete dummyConfig;

    Fcitx::Global::deInit();

    QTimer::singleShot(0, &a, SLOT(quit()));
    a.exec();

    return 0;
}
