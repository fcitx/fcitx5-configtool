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
ConfigDescManager* ConfigDescManager::inst = NULL;

ConfigDescManager* ConfigDescManager::instance()
{
    if (!inst)
        inst = new ConfigDescManager;
    return inst;
}

void ConfigDescManager::deInit()
{
    if (inst) {
        delete inst;
        inst = 0;
    }
}

ConfigDescManager::ConfigDescManager() :
    QObject(0), m_hash(new QHash<QString, FcitxConfigFileDesc*>)
{

}

ConfigDescManager::~ConfigDescManager()
{
    QHash<QString, FcitxConfigFileDesc*>::iterator iter;

    for (iter = m_hash->begin();
            iter != m_hash->end();
            iter ++) {
        FcitxConfigFreeConfigFileDesc(iter.value());
    }

    delete m_hash;
}

FcitxConfigFileDesc* ConfigDescManager::GetConfigDesc(const QString& name)
{
    if (m_hash->count(name) <= 0) {
        FILE* fp = FcitxXDGGetFileWithPrefix("configdesc", name.toLatin1().constData(), "r", NULL);
        FcitxConfigFileDesc* cfdesc =  FcitxConfigParseConfigFileDescFp(fp);

        if (cfdesc)
            m_hash->insert(name, cfdesc);

        return cfdesc;
    } else
        return (*m_hash) [name];
}

}

#include "moc_ConfigDescManager.cpp"
