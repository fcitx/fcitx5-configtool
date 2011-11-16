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

#ifndef FCITX_IM_H
#define FCITX_IM_H

// Qt
#include <QString>
#include <QMetaType>
#include <QDebug>
#include <QDBusArgument>

class FcitxIM
{
public:
    const QString& name() const;
    const QString& uniqueName() const;
    const QString& langCode() const;
    bool enabled() const;

    void setName(const QString& name);
    void setUniqueName(const QString& name);
    void setLangCode(const QString& name);
    void setEnabled(bool name);
    static void registerMetaType();

    bool operator < (const FcitxIM& im) const {
        if (m_enabled == true && im.m_enabled == false)
            return true;
        return false;
    }
private:
    QString m_name;
    QString m_uniqueName;
    QString m_langCode;
    bool m_enabled;
};

typedef QList<FcitxIM> FcitxIMList;

inline QDebug &operator<<(QDebug& debug, const FcitxIM& im)
{
    debug << im.name() << " " << im.uniqueName() << " " << im.langCode() << " " << im.enabled();
    return debug;
}

QDBusArgument& operator<<(QDBusArgument& argument, const FcitxIM& im);
const QDBusArgument& operator>>(const QDBusArgument& argument, FcitxIM& im);

Q_DECLARE_METATYPE(FcitxIM)
Q_DECLARE_METATYPE(FcitxIMList)

#endif