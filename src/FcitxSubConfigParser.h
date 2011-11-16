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

#ifndef FCITXSUBCONFIGPARSER_H
#define FCITXSUBCONFIGPARSER_H

// Qt
#include <QObject>
#include <QStringList>
#include <QFile>
#include <QMap>
#include <QSet>
#include <QDir>

// self
#include "FcitxSubConfig.h"

namespace Fcitx
{
class FcitxSubConfigPattern;
class FcitxSubConfig;

class FcitxSubConfigParser : public QObject
{
    Q_OBJECT
public:
    FcitxSubConfigParser(const QString& subConfigString, QObject* parent = NULL);
    FcitxSubConfig* getSubConfig(const QString& key);
    QStringList getSubConfigKeys();
    const QString& domain() const;
protected:
    SubConfigType parseType(const QString& str);
    QSet<QString> getFiles(const QString& key);
    QStringList getFilesByPattern(QDir& dir, FcitxSubConfigPattern* pattern, int index = 0);
    QMap<QString, FcitxSubConfigPattern*> m_subConfigMap;
    QString m_domain;
};

}

#endif
