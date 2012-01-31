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
#include <QDebug>

// self
#include "FcitxSubConfigPattern.h"

namespace Fcitx
{
FcitxSubConfigPattern* FcitxSubConfigPattern::parsePattern(Fcitx::SubConfigType type, const QString& p, QObject* parent)
{
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
    Q_FOREACH(const QString & str, filePatternlist) {
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

FcitxSubConfigPattern::FcitxSubConfigPattern(Fcitx::SubConfigType type, const QStringList& filePatternlist, QObject* parent) : QObject(parent)
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
