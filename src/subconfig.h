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

#ifndef FCITXSUBCONFIG_H
#define FCITXSUBCONFIG_H

// Qt
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QSet>

namespace Fcitx
{

class SubConfigPattern;
enum SubConfigType {
    SC_None,
    SC_ConfigFile,
    SC_NativeFile,
    SC_Program
};

class SubConfig
{
public:
    explicit SubConfig(const QString& name, SubConfigPattern* pattern);
    SubConfigType type();
    QSet< QString >& fileList();
    QSet< QString >& userFileList();
    const QString& name() const;
    const QString& configdesc() const;
    const QString& nativepath() const;
    const QString& mimetype() const;
    const QString& program() const;
    void updateFileList();
private:
    void parseProgramSubConfig(const SubConfigPattern* pattern);
    void parseNativeFileSubConfig(const SubConfigPattern* pattern);
    void parseConfigFileSubConfig(const SubConfigPattern* pattern);
    QString m_name;
    SubConfigType m_type;
    QSet< QString > m_fileList;
    QSet< QString > m_userFileList;
    QString m_configdesc;
    QString m_nativepath;
    QString m_mimetype;
    QString m_progam;
    QStringList m_filePatternList;
};

}

#endif

