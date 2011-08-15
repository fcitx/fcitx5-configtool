#include "ConfigDescManager.h"
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>

ConfigDescManager::ConfigDescManager(QObject* parent) :
    QObject(parent), m_hash(new QHash<QString, ConfigFileDesc*>)
{
    
}

ConfigDescManager::~ConfigDescManager()
{
    QHash<QString, ConfigFileDesc*>::iterator iter;
    for (iter = m_hash->begin();
         iter != m_hash->end();
         iter ++)
    {
        FreeConfigFileDesc(iter.value());
    }
    delete m_hash;
}

ConfigFileDesc* ConfigDescManager::GetConfigDesc(const QString& name)
{
    if (m_hash->count(name) <= 0)
    {
        FILE* fp = GetXDGFileWithPrefix("configdesc", name.toLatin1().constData(), "r", NULL);
        ConfigFileDesc* cfdesc =  ParseConfigFileDescFp(fp);
        if (cfdesc)
            m_hash->insert(name, cfdesc);
        return cfdesc;
    }
    else
        return (*m_hash)[name];
}


#include "moc_ConfigDescManager.cpp"