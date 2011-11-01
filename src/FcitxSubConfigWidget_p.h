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

#ifndef FCITXSUBCONFIGWIDGET_P_H
#define FCITXSUBCONFIGWIDGET_P_H
#include <QAbstractListModel>

namespace Fcitx
{
    class FcitxConfigFile
    {
    public:
        FcitxConfigFile(const QString& path);
        QString name();
        const QString& path() const;
    private:
        QString m_path;
    };

    class FcitxConfigFileItemModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        FcitxConfigFileItemModel(QObject* parent = 0);
        virtual ~FcitxConfigFileItemModel();
        virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        void addConfigFile(FcitxConfigFile* configfile);
    private:
        QList<FcitxConfigFile*> m_files;
    };
}

#endif
