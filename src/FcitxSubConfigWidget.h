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

#ifndef FCITXCONFIGPAGE_P_H
#define FCITXCONFIGPAGE_P_H

// Qt
#include <QWidget>

class QListView;
namespace Fcitx
{

class FcitxConfigFileItemModel;
class FcitxSubConfig;

class FcitxSubConfigWidget : public QWidget
{
    Q_OBJECT
public:
    FcitxSubConfigWidget(FcitxSubConfig* subconfig, QWidget* parent = 0);
    virtual ~FcitxSubConfigWidget();

private slots:
    void OpenSubConfig();
    void OpenNativeFile();

private:
    FcitxSubConfig* m_subConfig;
    FcitxConfigFileItemModel* m_model;
    QListView* m_listView;
};

}

#endif