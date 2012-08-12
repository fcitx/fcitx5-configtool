#include <fcitx-config/hotkey.h>
#include <assert.h>

#include "dummyconfig.h"

namespace Fcitx {

DummyConfig::DummyConfig(FcitxConfigFileDesc* cfdesc) :
    m_cfdesc(cfdesc),
    m_cfile(NULL)
{
    m_config.configFile = NULL;
    /* malloc necessary value */
    HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
        HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
            QString name("%1/%2");
            name = name.arg(cgdesc->groupName, codesc->optionName);
            if (m_dummyValue.contains(name))
                continue;
            void* value = NULL;
            switch (codesc->type)
            {
#define OPTION_TYPE_CASE(NAME, TYPE) \
    case T_##NAME: \
        value = fcitx_utils_new(TYPE); \
        break;
                OPTION_TYPE_CASE(Integer, int);
                OPTION_TYPE_CASE(Boolean, boolean);
                OPTION_TYPE_CASE(Char, char);
                OPTION_TYPE_CASE(Color, FcitxConfigColor);
                OPTION_TYPE_CASE(Enum, int);
                OPTION_TYPE_CASE(File, char*);
                OPTION_TYPE_CASE(Font, char*);
                OPTION_TYPE_CASE(Hotkey, FcitxHotkeys);
                OPTION_TYPE_CASE(String, char*);
                default:
                    break;
            }
            if (value)
                m_dummyValue[name] = value;
        }
    }
}

DummyConfig::~DummyConfig()
{
    FcitxConfigFree(&m_config);
    foreach(void* value, m_dummyValue)
    {
        free(value);
    }
}

FcitxGenericConfig* DummyConfig::genericConfig()
{
    return &m_config;
}

void DummyConfig::load(FILE* fp)
{
    if (!m_config.configFile) {
        m_config.configFile = FcitxConfigParseConfigFileFp(fp, m_cfdesc);

        HASH_FOREACH(cgdesc, m_cfdesc->groupsDesc, FcitxConfigGroupDesc) {
            HASH_FOREACH(codesc, cgdesc->optionsDesc, FcitxConfigOptionDesc) {
                QString name("%1/%2");
                name = name.arg(cgdesc->groupName, codesc->optionName);
                assert(m_dummyValue[name]);
                FcitxConfigBindValue(m_config.configFile, cgdesc->groupName, codesc->optionName, m_dummyValue[name], NULL, NULL);
            }
        }
    }
    else {
        m_config.configFile = FcitxConfigParseIniFp(fp, m_config.configFile);
    }
}

void DummyConfig::bind(char* group, char* option, FcitxSyncFilter filter, void* arg)
{
    if (!m_config.configFile)
        return;
    QString name("%1/%2");
    name = name.arg(group, option);

    if (!m_dummyValue.contains(name))
        return;

    assert(m_dummyValue[name]);
    FcitxConfigBindValue(m_config.configFile, group, option, m_dummyValue[name], filter, arg);
}

bool DummyConfig::isValid()
{
    return (m_config.configFile != NULL);
}

void DummyConfig::sync()
{
    FcitxConfigBindSync(&m_config);
}




}