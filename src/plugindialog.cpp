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
    setWindowTitle(widget->title());
    setWindowIcon(KIcon(widget->icon()));
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Reset);
    setMainWidget(widget);
    connect(m_widget, SIGNAL(changed(bool)), this, SLOT(changed(bool)));
    if (m_widget->asyncSave())
        connect(m_widget, SIGNAL(saveFinished()), this, SLOT(saveFinished()));
}

void PluginDialog::saveFinished()
{
    if (m_widget->asyncSave())
        m_widget->setEnabled(true);
    KDialog::slotButtonClicked(KDialog::Ok);
}

void PluginDialog::slotButtonClicked(int button)
{
    switch (button) {
        case KDialog::Reset:
            m_widget->load();
            break;
        case KDialog::Ok:
            if (m_widget->asyncSave())
                m_widget->setEnabled(false);
            m_widget->save();
            if (!m_widget->asyncSave())
                KDialog::slotButtonClicked(button);
            break;
        default:
            KDialog::slotButtonClicked(button);
    }
}

void PluginDialog::changed(bool changed)
{
    enableButton(KDialog::Ok, changed);
    enableButton(KDialog::Reset, changed);
}

}
