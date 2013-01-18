/***************************************************************************
 *   Copyright (C) 2013~2013 by CSSlayer                                   *
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

#include "plugindialog.h"

namespace Fcitx {

PluginDialog::PluginDialog(FcitxQtConfigUIWidget* widget, QWidget* parent, Qt::WindowFlags flags) : KDialog(parent, flags)
    ,m_widget(widget)
{
    setWindowIcon(KIcon(widget->icon()));
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Reset);
    setMainWidget(widget);
    connect(m_widget, SIGNAL(changed(bool)), this, SLOT(changed(bool)));

    changed(false);
}

void PluginDialog::slotButtonClicked(int button)
{
    if (button == KDialog::Reset) {
        m_widget->load();
    } else if (button == KDialog::Ok) {
        m_widget->save();
    }

    KDialog::slotButtonClicked(button);
}

void PluginDialog::changed(bool changed)
{
    enableButton(KDialog::Ok, changed);
    enableButton(KDialog::Reset, changed);
}

}
