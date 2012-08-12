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
#include <QDBusArgument>
#include <QDBusMetaType>

// self
#include "im.h"

bool Fcitx::IM::enabled() const
{
    return m_enabled;
}
const QString& Fcitx::IM::langCode() const
{
    return m_langCode;
}
const QString& Fcitx::IM::name() const
{
    return m_name;
}
const QString& Fcitx::IM::uniqueName() const
{
    return m_uniqueName;
}
void Fcitx::IM::setEnabled(bool enable)
{
    m_enabled = enable;
}
void Fcitx::IM::setLangCode(const QString& lang)
{
    m_langCode = lang;
}
void Fcitx::IM::setName(const QString& name)
{
    m_name = name;
}
void Fcitx::IM::setUniqueName(const QString& name)
{
    m_uniqueName = name;
}

void Fcitx::IM::registerMetaType()
{
    qRegisterMetaType<Fcitx::IM>("Fcitx::IM");
    qDBusRegisterMetaType<Fcitx::IM>();
    qRegisterMetaType<Fcitx::IMList>("Fcitx::IMList");
    qDBusRegisterMetaType<Fcitx::IMList>();
}

QDBusArgument& operator<<(QDBusArgument& argument, const Fcitx::IM& im)
{
    argument.beginStructure();
    argument << im.name();
    argument << im.uniqueName();
    argument << im.langCode();
    argument << im.enabled();
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, Fcitx::IM& im)
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
