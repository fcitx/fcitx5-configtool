#ifndef FCITXSUBCONFIGPATTERN_H
#define FCITXSUBCONFIGPATTERN_H

#include <QObject>
#include "FcitxSubConfig.h"
#include <QStringList>

namespace Fcitx
{
    class FcitxSubConfigPattern : public QObject
    {
        Q_OBJECT
    public:
        static FcitxSubConfigPattern* parsePattern ( SubConfigType type, const QString& pattern, QObject* parent = NULL);

        int size();
        const QString& getPattern( int index);
        const QString& configdesc();
        SubConfigType type();
        const QString&  nativepath();
    private:
        FcitxSubConfigPattern( Fcitx::SubConfigType type, const QStringList& filePatternlist, QObject* parent = 0);
        QStringList m_filePatternlist;
        QString m_configdesc;
        QString m_nativepath;
        SubConfigType m_type;
    };

}

#endif