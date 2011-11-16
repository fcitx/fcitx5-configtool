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

// self
#include "FcitxSubConfig.h"

namespace Fcitx
{

FcitxSubConfig* FcitxSubConfig::GetConfigFileSubConfig(const QString& name, const QString& configdesc, const QSet< QString >& fileList)
{
    FcitxSubConfig* subconfig = new FcitxSubConfig;
    subconfig->m_name = name;
    subconfig->m_type = SC_ConfigFile;
    subconfig->m_filelist = fileList;
    subconfig->m_configdesc = configdesc;
    return subconfig;
}

FcitxSubConfig* FcitxSubConfig::GetNativeFileSubConfig(const QString& name, const QString& nativepath, const QSet< QString >& fileList)
{
    FcitxSubConfig* subconfig = new FcitxSubConfig;
    subconfig->m_name = name;
    subconfig->m_type = SC_NativeFile;
    subconfig->m_filelist = fileList;
    subconfig->m_nativepath = nativepath;
    return subconfig;
}

FcitxSubConfig::FcitxSubConfig(QObject* parent) : QObject(parent)
{

}

SubConfigType FcitxSubConfig::type()
{
    return m_type;
}

const QString& FcitxSubConfig::name() const
{
    return m_name;
}

const QString& FcitxSubConfig::configdesc() const
{
    return m_configdesc;
}

const QString& FcitxSubConfig::nativepath() const
{
    return m_nativepath;
}

QSet< QString >& FcitxSubConfig::filelist()
{
    return m_filelist;
}

}