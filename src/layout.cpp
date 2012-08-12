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
#include "layout.h"

const QString& Fcitx::Layout::layout() const
{
    return m_layout;
}
const QString& Fcitx::Layout::langCode() const
{
    return m_langCode;
}
const QString& Fcitx::Layout::name() const
{
    return m_name;
}

const QString& Fcitx::Layout::variant() const
{
    return m_variant;
}

void Fcitx::Layout::setLayout(const QString& layout)
{
    m_layout = layout;
}

void Fcitx::Layout::setLangCode(const QString& lang)
{
    m_langCode = lang;
}

void Fcitx::Layout::setName(const QString& name)
{
    m_name = name;
}

void Fcitx::Layout::setVariant(const QString& variant)
{
    m_variant = variant;
}

void Fcitx::Layout::registerMetaType()
{
    qRegisterMetaType<Fcitx::Layout>("Fcitx::Layout");
    qDBusRegisterMetaType<Fcitx::Layout>();
    qRegisterMetaType<Fcitx::LayoutList>("Fcitx::LayoutList");
    qDBusRegisterMetaType<Fcitx::LayoutList>();
}

QDBusArgument& operator<<(QDBusArgument& argument, const Fcitx::Layout& layout)
{
    argument.beginStructure();
    argument << layout.layout();
    argument << layout.variant();
    argument << layout.name();
    argument << layout.langCode();
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, Fcitx::Layout& layout)
{
    QString l;
    QString variant;
    QString name;
    QString langCode;
    argument.beginStructure();
    argument >> l >> variant >> name >> langCode;
    argument.endStructure();
    layout.setLayout(l);
    layout.setVariant(variant);
    layout.setName(name);
    layout.setLangCode(langCode);
    return argument;
}
