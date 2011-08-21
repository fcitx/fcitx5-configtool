#ifndef CONFIGDESCMANAGER_H
#define CONFIGDESCMANAGER_H

#include <QObject>
#include <QHash>
#include <fcitx-config/fcitx-config.h>

namespace Fcitx
{

    class ConfigDescManager : public QObject
    {
        Q_OBJECT

    public:
        ConfigDescManager ( QObject* parent = NULL );
        virtual ~ConfigDescManager();
        ConfigFileDesc* GetConfigDesc ( const QString& name );

    private:
        QHash<QString, ConfigFileDesc*>* m_hash;
    };

}

#endif
