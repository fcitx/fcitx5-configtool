
#ifndef FCITXSUBCONFIGPARSER_H
#define FCITXSUBCONFIGPARSER_H

#include <QObject>
#include <QStringList>
#include <QFile>
#include <QMap>
#include <QSet>
#include <QDir>
#include "FcitxSubConfig.h"

namespace Fcitx
{
    class FcitxSubConfigPattern;
    class FcitxSubConfig;

    class FcitxSubConfigParser : public QObject
    {
        Q_OBJECT
    public:
        FcitxSubConfigParser ( const QString& subConfigString, QObject* parent = NULL );
        FcitxSubConfig* getSubConfig( const QString& key );
        QStringList getSubConfigKeys();
        const QString& domain() const;
    protected:
        SubConfigType parseType ( const QString& str );
        QSet<QString> getFiles( const QString& key);
        QStringList getFilesByPattern(QDir& dir, FcitxSubConfigPattern* pattern, int index = 0);
        QMap<QString, FcitxSubConfigPattern*> m_subConfigMap;
        QString m_domain;
    };

}

#endif
