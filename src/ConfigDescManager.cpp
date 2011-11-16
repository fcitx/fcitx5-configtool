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

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>

// self
#include "ConfigDescManager.h"

namespace Fcitx
{

ConfigDescManager::ConfigDescManager(QObject* parent) :
    QObject(parent), m_hash(new QHash<QString, ConfigFileDesc*>)
{

}

ConfigDescManager::~ConfigDescManager()
{
    QHash<QString, ConfigFileDesc*>::iterator iter;

    for (iter = m_hash->begin();
            iter != m_hash->end();
            iter ++) {
        FreeConfigFileDesc(iter.value());
    }

    delete m_hash;
}

ConfigFileDesc* ConfigDescManager::GetConfigDesc(const QString& name)
{
    if (m_hash->count(name) <= 0) {
        FILE* fp = GetXDGFileWithPrefix("configdesc", name.toLatin1().constData(), "r", NULL);
        ConfigFileDesc* cfdesc =  ParseConfigFileDescFp(fp);

        if (cfdesc)
            m_hash->insert(name, cfdesc);

        return cfdesc;
    } else
        return (*m_hash) [name];
}

}

#include "moc_ConfigDescManager.cpp"
