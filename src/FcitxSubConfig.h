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

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QSet>

namespace Fcitx
{
    enum SubConfigType
    {
        SC_None,
        SC_ConfigFile,
        SC_NativeFile
    };

    class FcitxSubConfig: public QObject
    {
        Q_OBJECT
    public:
        static FcitxSubConfig* GetConfigFileSubConfig(const QString& name, const QString& configdesc, const QSet< QString >& fileList);
        static FcitxSubConfig* GetNativeFileSubConfig(const QString& name, const QString& nativepath, const QSet< QString >& fileList);
        SubConfigType type();
        QSet< QString >& filelist();
        const QString& name() const;
        const QString& configdesc() const;
        const QString& nativepath() const;
    private:
        FcitxSubConfig ( QObject* parent = 0 );
        SubConfigType m_type;
        QString m_name;
        QSet< QString > m_filelist;
        QString m_configdesc;
        QString m_nativepath;
    };

}

#endif