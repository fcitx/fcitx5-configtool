/***************************************************************************
 *   Copyright (C) 2012~2012 by CSSlayer                                   *
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

#ifndef DUMMYCONFIG_H
#define DUMMYCONFIG_H

#include <QMap>
#include <QString>
#include <stdio.h>

namespace fcitx {

class DummyConfig {
public:
    DummyConfig(FcitxConfigFileDesc *cfdesc);
    ~DummyConfig();

    FcitxGenericConfig *genericConfig();
    void load(FILE *fp);
    void bind(char *group, char *option, FcitxSyncFilter filter = NULL,
              void *arg = NULL);
    bool isValid();
    void sync();

private:
    QMap<QString, void *> m_dummyValue;
    FcitxConfigFileDesc *m_cfdesc;
    FcitxConfigFile *m_cfile;
    FcitxGenericConfig m_config;
};
}

#endif // DUMMYCONFIG_H
