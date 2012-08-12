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

#ifndef FCITX_LAYOUT_H
#define FCITX_LAYOUT_H

// Qt
#include <QString>
#include <QMetaType>
#include <QDebug>
#include <QDBusArgument>

namespace Fcitx {

class Layout
{
public:
    const QString& layout() const;
    const QString& variant() const;
    const QString& name() const;
    const QString& langCode() const;
    void setLayout(const QString& layout);
    void setLangCode(const QString& lang);
    void setName(const QString& name);
    void setVariant(const QString& variant);

    static void registerMetaType();
private:
    QString m_layout;
    QString m_variant;
    QString m_name;
    QString m_langCode;
};

typedef QList<Layout> LayoutList;

}

QDBusArgument& operator<<(QDBusArgument& argument, const Fcitx::Layout& im);
const QDBusArgument& operator>>(const QDBusArgument& argument, Fcitx::Layout& l);

Q_DECLARE_METATYPE(Fcitx::Layout)
Q_DECLARE_METATYPE(Fcitx::LayoutList)

#endif
