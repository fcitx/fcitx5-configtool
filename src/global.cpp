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
#include <fcitx-qt/fcitxqtconnection.h>
#include <fcitx-qt/fcitxqtconfiguifactory.h>
#include <fcitx-qt/fcitxqtinputmethodproxy.h>

// self
#include "global.h"


namespace Fcitx
{
Global* Global::inst = NULL;

Global* Global::instance()
{
    if (!inst)
        inst = new Global;
    return inst;
}

void Global::deInit()
{
    if (inst) {
        inst->deleteLater();
        inst = 0;
    }
}

Global::Global() :
    m_hash(new QHash<QString, FcitxConfigFileDesc*>),
    m_factory(new FcitxQtConfigUIFactory(this)),
    m_connection(new FcitxQtConnection(this)),
    m_inputmethod(0),
    m_keyboard(0)
{
    connect(m_connection, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_connection, SIGNAL(disconnected()), this, SLOT(disconnected()));

    m_connection->startConnection();
}

Global::~Global()
{
    QHash<QString, FcitxConfigFileDesc*>::iterator iter;

    for (iter = m_hash->begin();
            iter != m_hash->end();
            iter ++) {
        FcitxConfigFreeConfigFileDesc(iter.value());
    }

    delete m_hash;
}

void Global::connected()
{
    if (m_inputmethod)
        delete m_inputmethod;

    if (m_keyboard)
        delete m_keyboard;

    m_inputmethod = new FcitxQtInputMethodProxy(
        m_connection->serviceName(),
        "/inputmethod",
        *m_connection->connection(),
        this
    );

    m_keyboard = new FcitxQtKeyboardProxy(
        m_connection->serviceName(),
        "/keyboard",
        *m_connection->connection(),
        this
    );

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
    m_inputmethod->setTimeout(3000);
    m_keyboard->setTimeout(3000);
#endif
}

void Global::disconnected()
{
    if (m_inputmethod)
        delete m_inputmethod;
    m_inputmethod = 0;
    if (m_keyboard)
        delete m_keyboard;
    m_keyboard = 0;
}

FcitxConfigFileDesc* Global::GetConfigDesc(const QString& name)
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
