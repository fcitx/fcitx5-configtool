
#include "FcitxSubConfigParser.h"
#include <QDir>
#include <fcitx-config/xdg.h>
#include "config.h"
#include "FcitxSubConfigPattern.h"
#include "FcitxSubConfigPath.h"

namespace Fcitx
{
    FcitxSubConfigParser::FcitxSubConfigParser ( const QString& subConfigString, QObject* parent ) :
            QObject ( parent )
    {
        QStringList subConfigList = subConfigString.split(',');
        Q_FOREACH(const QString& str, subConfigList)
        {
            int i = str.indexOf(':');
            if (i < 0)
                continue;
            QString namestr = str.section(':', 0, 0);
            if (namestr.length() == 0)
                continue;
            QString typestr = str.section(':', 1, 1);
            if (typestr == "domain")
            {
                m_domain = namestr;
                continue;
            }
            SubConfigType type = parseType(typestr);
            if (type == SC_None)
                continue;
            if (m_subConfigMap.count(namestr) > 0)
                continue;
            QString patternstr = str.section(':', 2, -1);
            FcitxSubConfigPattern* pattern = FcitxSubConfigPattern::parsePattern(type, patternstr, this);
            if (pattern == NULL)
                continue;
            m_subConfigMap[namestr] = pattern;
        }
    }

    SubConfigType FcitxSubConfigParser::parseType(const QString& str)
    {
        if (str == "native")
        {
            return SC_NativeFile;
        }
        if (str == "configfile")
        {
            return SC_ConfigFile;
        }
        return SC_None;
    }

    QStringList FcitxSubConfigParser::getSubConfigKeys()
    {
        return m_subConfigMap.keys();
    }

    QMultiMap<QString, FcitxSubConfigPath > FcitxSubConfigParser::getFiles( const QString& name)
    {
        if (m_subConfigMap.count(name) != 1)
            return QMultiMap<QString, FcitxSubConfigPath>();
        FcitxSubConfigPattern* pattern = m_subConfigMap[name];
        size_t size;
        char** xdgpath = GetXDGPath(&size, "XDG_CONFIG_HOME", ".config" , PACKAGE , DATADIR, PACKAGE);

        QMultiMap<QString, FcitxSubConfigPath> result;
        for (size_t i = 0; i < size; i ++)
        {
            QDir dir(xdgpath[i]);
            QStringList list = getFilesByPattern(dir, pattern, 0);
            Q_FOREACH(const QString& str, list)
            {
                result.insert(
                    dir.relativeFilePath(str),
                    FcitxSubConfigPath(
                        QString(xdgpath[i]),
                        str
                    ));
            }
        }

        FreeXDGPath(xdgpath);

        return result;
    }

    QStringList FcitxSubConfigParser::getFilesByPattern(QDir& currentdir, FcitxSubConfigPattern* pattern, int index)
    {
        QStringList result;
        if (!currentdir.exists())
            return result;

        const QString& filter = pattern->getPattern(index);
        QStringList filters;
        filters << filter;
        QDir::Filters filterflag;

        if (index + 1 == pattern->size())
        {
            filterflag = QDir::Files;
        }
        else
        {
            filterflag = QDir::Dirs | QDir::NoDotAndDotDot;
        }

        QStringList list = currentdir.entryList(filters, filterflag);
        if (index + 1 == pattern->size())
        {
            Q_FOREACH(const QString& item, list)
            {
                result << currentdir.absoluteFilePath(item);
            }
        }
        else
        {
            Q_FOREACH(const QString& item, list)
            {
                QDir dir(currentdir.absoluteFilePath(item));
                result << getFilesByPattern(dir, pattern, index + 1);
            }
        }
        return result;
    }

    FcitxSubConfig* FcitxSubConfigParser::getSubConfig ( const QString& key )
    {
        if (m_subConfigMap.count(key) != 1)
            return NULL;

        FcitxSubConfigPattern* pattern = m_subConfigMap[key];

        FcitxSubConfig* subconfig = NULL;

        switch(pattern->type())
        {
            case SC_ConfigFile:
                subconfig = FcitxSubConfig::GetConfigFileSubConfig(key, pattern->configdesc(), this->getFiles(key));
                break;
            case SC_NativeFile:
                subconfig = FcitxSubConfig::GetNativeFileSubConfig(key, pattern->nativepath(), this->getFiles(key));
                break;
            default:
                break;
        }

        return subconfig;
    }

    const QString& FcitxSubConfigParser::domain() const
    {
        return m_domain;
    }

}

#include "moc_FcitxSubConfigParser.cpp"
