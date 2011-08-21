#include <QDebug>
#include "FcitxSubConfigPattern.h"

namespace Fcitx
{
    FcitxSubConfigPattern* FcitxSubConfigPattern::parsePattern ( Fcitx::SubConfigType type, const QString& p, QObject* parent )
    {
        qDebug() << p;
        QString pattern = p;
        if (type == SC_ConfigFile)
            pattern = p.section(':', 0, 0);

        if (pattern.length() == 0 || pattern[0] == '/')
            return NULL;
        if (type == SC_NativeFile && pattern.indexOf('*') > 0)
            return NULL;
        QStringList filePatternlist = pattern.split('/');
        if (filePatternlist.length() == 0)
            return NULL;
        Q_FOREACH(const QString& str, filePatternlist)
        {
            if (str.length() == 0)
                return NULL;
            if (str == ".")
                return NULL;
            if (str == "..")
                return NULL;
        }
        FcitxSubConfigPattern* result = new FcitxSubConfigPattern(type, filePatternlist, parent);

        if (type == SC_ConfigFile)
            result->m_configdesc = p.section(':', 1, 1);
        else if (type == SC_NativeFile)
            result->m_nativepath = pattern;

        return result;
    }

    FcitxSubConfigPattern::FcitxSubConfigPattern ( Fcitx::SubConfigType type, const QStringList& filePatternlist, QObject* parent ) : QObject ( parent )
    {
        m_filePatternlist = filePatternlist;
        m_type = type;
    }

    int FcitxSubConfigPattern::size()
    {
        return m_filePatternlist.length();
    }

    const QString& FcitxSubConfigPattern::getPattern(int index)
    {
        return m_filePatternlist.at(index);
    }


    const QString& FcitxSubConfigPattern::configdesc()
    {
        return m_configdesc;
    }

    const QString& FcitxSubConfigPattern::nativepath()
    {
        return m_nativepath;
    }

    SubConfigType FcitxSubConfigPattern::type()
    {
        return m_type;
    }
}