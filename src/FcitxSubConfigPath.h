
#ifndef FCITXSUBCONFIGPATH_H
#define FCITXSUBCONFIGPATH_H

#include <QObject>

namespace Fcitx
{
    class FcitxSubConfigPath
    {
    public:
        FcitxSubConfigPath ( const QString& prefix, const QString& path );
        bool isWritable();
        const QString& path() const;
        const QString& prefix() const;
    private:
        QString m_prefix;
        QString m_path;
    };
}

#endif