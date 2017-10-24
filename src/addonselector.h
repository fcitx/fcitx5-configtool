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

#ifndef __FCITX_ADDON_SELECTOR_H__
#define __FCITX_ADDON_SELECTOR_H__

// Qt
#include <QWidget>

struct _FcitxAddon;

namespace fcitx {

class Module;

class AddonSelector : public QWidget {
    Q_OBJECT

public:
    AddonSelector(Module *parent);
    virtual ~AddonSelector();
    void load();
    void save();
    void addAddon(struct _FcitxAddon *fcitxAddon);

Q_SIGNALS:
    void changed(bool hasChanged);
    void configCommitted(const QByteArray &componentName);

private:
    class Private;
    Private *d;
    Module *parent;
};
}

#endif
