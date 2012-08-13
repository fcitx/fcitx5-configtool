/***************************************************************************
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

// Qt
#include <QDir>
#include <QSet>

// Fcitx
#include <fcitx-config/xdg.h>

// self
#include "config.h"
#include "subconfigpattern.h"
#include "subconfigparser.h"

namespace Fcitx
{
SubConfigParser::SubConfigParser(const QString& subConfigString, QObject* parent) :
    QObject(parent)
{
    /*
     * format like: name:type:XXXXXXX,name:type:XXXXXXX
     * valid value contains:
     * <domain name>:domain
     * <name>:native:path
     * <name>:configfile:path:configdesc
     */
    QStringList subConfigList = subConfigString.split(',');
    Q_FOREACH(const QString & str, subConfigList) {
        int i = str.indexOf(':');
        if (i < 0)
            continue;
        QString namestr = str.section(':', 0, 0);
        if (namestr.length() == 0)
            continue;
        QString typestr = str.section(':', 1, 1);
        if (typestr == "domain") {
            m_domain = namestr;
            continue;
        }
        SubConfigType type = parseType(typestr);
        if (type == SC_None)
            continue;
        if (m_subConfigMap.count(namestr) > 0)
            continue;
        QString patternstr = str.section(':', 2, -1);
        SubConfigPattern* pattern = SubConfigPattern::parsePattern(type, patternstr, this);
        if (pattern == NULL)
            continue;
        m_subConfigMap[namestr] = pattern;
    }
}

SubConfigType SubConfigParser::parseType(const QString& str)
{
    if (str == "native") {
        return SC_NativeFile;
    }
    if (str == "configfile") {
        return SC_ConfigFile;
    }
    if (str == "program") {
        return SC_Program;
    }
    return SC_None;
}

QStringList SubConfigParser::getSubConfigKeys()
{
    return m_subConfigMap.keys();
}

QSet<QString> SubConfigParser::getFiles(const QString& name)
{
    if (m_subConfigMap.count(name) != 1)
        return QSet<QString> ();
    SubConfigPattern* pattern = m_subConfigMap[name];
    size_t size;
    char** xdgpath = FcitxXDGGetPathWithPrefix(&size, "");

    QSet<QString> result;
    for (size_t i = 0; i < size; i ++) {
        QDir dir(xdgpath[i]);
        QStringList list = getFilesByPattern(dir, pattern, 0);
        Q_FOREACH(const QString & str, list) {
            result.insert(
                dir.relativeFilePath(str));
        }
    }

    FcitxXDGFreePath(xdgpath);

    return result;
}

QStringList SubConfigParser::getFilesByPattern(QDir& currentdir, SubConfigPattern* pattern, int index)
{
    QStringList result;
    if (!currentdir.exists())
        return result;

    const QString& filter = pattern->getPattern(index);
    QStringList filters;
    filters << filter;
    QDir::Filters filterflag;

    if (index + 1 == pattern->size()) {
        filterflag = QDir::Files;
    } else {
        filterflag = QDir::Dirs | QDir::NoDotAndDotDot;
    }

    QStringList list = currentdir.entryList(filters, filterflag);
    if (index + 1 == pattern->size()) {
        Q_FOREACH(const QString & item, list) {
            result << currentdir.absoluteFilePath(item);
        }
    } else {
        Q_FOREACH(const QString & item, list) {
            QDir dir(currentdir.absoluteFilePath(item));
            result << getFilesByPattern(dir, pattern, index + 1);
        }
    }
    return result;
}

SubConfig* SubConfigParser::getSubConfig(const QString& key)
{
    if (m_subConfigMap.count(key) != 1)
        return NULL;

    SubConfigPattern* pattern = m_subConfigMap[key];

    SubConfig* subconfig = NULL;

    switch (pattern->type()) {
    case SC_ConfigFile:
        subconfig = SubConfig::GetConfigFileSubConfig(key, pattern->configdesc(), this->getFiles(key));
        break;
    case SC_NativeFile:
        subconfig = SubConfig::GetNativeFileSubConfig(key, pattern->nativepath(), pattern->mimetype(), this->getFiles(key));
        break;
    case SC_Program:
        subconfig = SubConfig::GetProgramSubConfig(key, pattern->program());
        break;
    default:
        break;
    }

    return subconfig;
}

const QString& SubConfigParser::domain() const
{
    return m_domain;
}

}
