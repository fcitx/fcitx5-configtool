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

#ifndef FCITXSUBCONFIGPATTERN_H
#define FCITXSUBCONFIGPATTERN_H

#include <QObject>
#include "FcitxSubConfig.h"
#include <QStringList>

namespace Fcitx
{
    class FcitxSubConfigPattern : public QObject
    {
        Q_OBJECT
    public:
        static FcitxSubConfigPattern* parsePattern ( SubConfigType type, const QString& pattern, QObject* parent = NULL);

        int size();
        const QString& getPattern( int index);
        const QString& configdesc();
        SubConfigType type();
        const QString&  nativepath();
    private:
        FcitxSubConfigPattern( Fcitx::SubConfigType type, const QStringList& filePatternlist, QObject* parent = 0);
        QStringList m_filePatternlist;
        QString m_configdesc;
        QString m_nativepath;
        SubConfigType m_type;
    };

}

#endif