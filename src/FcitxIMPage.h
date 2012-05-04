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

#ifndef FCITX_IM_PAGE_H
#define FCITX_IM_PAGE_H

// Qt
#include <QString>
#include <QWidget>
#include <QDBusConnection>

// self
#include "org.fcitx.Fcitx.InputMethod.h"

namespace Ui
{
class FcitxIMPage;
}

namespace Fcitx
{
class FcitxIMPage : public QWidget
{
    Q_OBJECT
public:
    FcitxIMPage(QWidget* parent = 0);
    virtual ~FcitxIMPage();
Q_SIGNALS:
    void changed();
public Q_SLOTS:
    void save();
    void load();
    void invalidate();
private:
    Ui::FcitxIMPage* m_ui;

    class Private;
    Private* d;
};
}

#endif

