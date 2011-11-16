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

#ifndef FCITX_SKIN_PAGE_H
#define FCITX_SKIN_PAGE_H

// Qt
#include <QWidget>

// self
#include "ui_FcitxSkinPage.h"


namespace Fcitx
{
class Module;

class FcitxSkinPage : public QWidget
{
    Q_OBJECT
public:
    FcitxSkinPage(Module* module, QWidget* parent = 0);
    virtual ~FcitxSkinPage();
public Q_SLOTS:
    void load();
    void save();
Q_SIGNALS:
    void changed();
protected Q_SLOTS:
    void installButtonClicked();
private:
    class Private;
    Module* m_module;
    Private* d;
    Ui::FcitxSkinPage* m_ui;
};

}

#endif