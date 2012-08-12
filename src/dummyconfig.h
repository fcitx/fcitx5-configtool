#include <fcitx-config/fcitx-config.h>
#include <stdio.h>
#include <QMap>
#include <QString>

namespace Fcitx {

class DummyConfig
{
public:
    DummyConfig(FcitxConfigFileDesc* cfdesc);
    ~DummyConfig();

    FcitxGenericConfig* genericConfig();
    void load(FILE* fp);
    void bind(char* group, char* option, FcitxSyncFilter filter = NULL, void* arg = NULL);
    bool isValid();
    void sync();
private:
    QMap<QString, void*> m_dummyValue;
    FcitxConfigFileDesc* m_cfdesc;
    FcitxConfigFile* m_cfile;
    FcitxGenericConfig m_config;
};

}