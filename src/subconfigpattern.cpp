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
#include "subconfigpattern.h"

namespace Fcitx
{
QStringList SubConfigPattern::parseFilePattern(const QString& pattern)
{
    do {
        if (pattern.length() == 0 || pattern[0] == '/')
            break;
        QStringList filePatternlist = pattern.split('/');
        if (filePatternlist.length() == 0)
            break;
        Q_FOREACH(const QString & str, filePatternlist) {
            if (str.length() == 0)
                break;
            if (str == ".")
                break;
            if (str == "..")
                break;
        }
        return filePatternlist;
    } while(0);

    return QStringList();
}

SubConfigPattern* SubConfigPattern::parsePattern(Fcitx::SubConfigType type, const QString& p, QObject* parent)
{
    switch (type) {
        case SC_ConfigFile:
        {
            QString pattern = p.section(':', 0, 0);
            QString configdesc = p.section(':', 1, 1);
            if (configdesc.isEmpty())
                return NULL;
            QStringList filePatternlist = parseFilePattern(pattern);
            if (filePatternlist.length() == 0)
                return NULL;
            SubConfigPattern* result = new SubConfigPattern(type, parent);
            result->m_configdesc = configdesc;
            result->m_filePatternlist = filePatternlist;
            return result;
        }
        break;
    case SC_NativeFile:
        {
            QString pattern = p.section(':', 0, 0);
            if (pattern.indexOf('*') > 0)
                return NULL;
            QString mimetype = p.section(':', 1, 1);

            QStringList filePatternlist = parseFilePattern(pattern);
            if (filePatternlist.length() == 0)
                return NULL;
            SubConfigPattern* result = new SubConfigPattern(type, parent);
            if (!mimetype.isEmpty())
                result->m_mimetype = mimetype;
            result->m_nativepath = pattern;
            result->m_filePatternlist = filePatternlist;
            return result;
        }
        break;
    case SC_Program:
        {
            QString pattern = p.section(':', 0, 0);
            if (pattern.isEmpty())
                return NULL;
            SubConfigPattern* result = new SubConfigPattern(type, parent);
            result->m_progam = pattern;
            return result;
        }
        break;
    case SC_Plugin:
        {
            QString pattern = p.section(':', 0, 0);
            if (pattern.isEmpty())
                return NULL;
            SubConfigPattern* result = new SubConfigPattern(type, parent);
            result->m_nativepath = pattern;
            return result;
        }
        break;
    default:
        return NULL;
    }
    return NULL;
}

SubConfigPattern::SubConfigPattern(Fcitx::SubConfigType type, QObject* parent) : QObject(parent)
{
    m_type = type;
}

const QStringList& SubConfigPattern::filePatternList() const
{
    return m_filePatternlist;
}

const QString& SubConfigPattern::configdesc() const
{
    return m_configdesc;
}

const QString& SubConfigPattern::nativepath() const
{
    return m_nativepath;
}

const QString& SubConfigPattern::program() const
{
    return m_progam;
}

const QString& SubConfigPattern::mimetype() const
{
    return m_mimetype;
}

SubConfigType SubConfigPattern::type() const
{
    return m_type;
}
}
