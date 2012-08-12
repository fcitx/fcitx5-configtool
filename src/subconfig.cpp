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
#include "subconfig.h"

namespace Fcitx
{

SubConfig* SubConfig::GetConfigFileSubConfig(const QString& name, const QString& configdesc, const QSet< QString >& fileList)
{
    SubConfig* subconfig = new SubConfig;
    subconfig->m_name = name;
    subconfig->m_type = SC_ConfigFile;
    subconfig->m_filelist = fileList;
    subconfig->m_configdesc = configdesc;
    return subconfig;
}

SubConfig* SubConfig::GetNativeFileSubConfig(const QString& name, const QString& nativepath, const QSet< QString >& fileList)
{
    SubConfig* subconfig = new SubConfig;
    subconfig->m_name = name;
    subconfig->m_type = SC_NativeFile;
    subconfig->m_filelist = fileList;
    subconfig->m_nativepath = nativepath;
    return subconfig;
}

SubConfig::SubConfig(QObject* parent) : QObject(parent)
{

}

SubConfigType SubConfig::type()
{
    return m_type;
}

const QString& SubConfig::name() const
{
    return m_name;
}

const QString& SubConfig::configdesc() const
{
    return m_configdesc;
}

const QString& SubConfig::nativepath() const
{
    return m_nativepath;
}

QSet< QString >& SubConfig::filelist()
{
    return m_filelist;
}

}
