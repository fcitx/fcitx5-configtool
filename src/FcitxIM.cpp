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

#include "FcitxIM.h"
#include <qdbusargument.h>
#include <qdbusmetatype.h>

bool FcitxIM::enabled() const
{
    return m_enabled;
}
const QString& FcitxIM::langCode() const
{
    return m_langCode;
}
const QString& FcitxIM::name() const
{
    return m_name;
}
const QString& FcitxIM::uniqueName() const
{
    return m_uniqueName;
}
void FcitxIM::setEnabled(bool enable)
{
    m_enabled = enable;
}
void FcitxIM::setLangCode(const QString& lang)
{
    m_langCode = lang;
}
void FcitxIM::setName(const QString& name)
{
    m_name = name;
}
void FcitxIM::setUniqueName(const QString& name)
{
    m_uniqueName = name;
}

void FcitxIM::registerMetaType()
{
    qRegisterMetaType<FcitxIM>("FcitxIM"); 
    qDBusRegisterMetaType<FcitxIM>();
    qRegisterMetaType<FcitxIMList>("FcitxIMList"); 
    qDBusRegisterMetaType<FcitxIMList>();
}

QDBusArgument& operator<<(QDBusArgument& argument, const FcitxIM& im)
{
    argument.beginStructure();
    argument << im.name();
    argument << im.uniqueName();
    argument << im.langCode();
    argument << im.enabled();
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, FcitxIM& im)
{
    QString name;
    QString uniqueName;
    QString langCode;
    bool enabled;
    argument.beginStructure();
    argument >> name >> uniqueName >> langCode >> enabled;
    argument.endStructure();
    im.setName(name);
    im.setUniqueName(uniqueName);
    im.setLangCode(langCode);
    im.setEnabled(enabled);
    return argument;
}