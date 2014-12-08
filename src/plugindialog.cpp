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
#include <QIcon>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

namespace Fcitx {

PluginDialog::PluginDialog(FcitxQtConfigUIWidget* widget, QWidget* parent, Qt::WindowFlags flags) : QDialog(parent, flags)
    ,m_widget(widget)
{
    setWindowTitle(widget->title());
    setWindowIcon(QIcon::fromTheme(widget->icon()));
    QHBoxLayout* dialogLayout = new QHBoxLayout;
    setLayout(dialogLayout);
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    dialogLayout->addWidget(m_widget);
    dialogLayout->addWidget(m_buttonBox);
    connect(m_widget, SIGNAL(changed(bool)), this, SLOT(changed(bool)));
    if (m_widget->asyncSave())
        connect(m_widget, SIGNAL(saveFinished()), this, SLOT(saveFinished()));
    connect(m_buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* button) {
        slotButtonClicked(m_buttonBox->standardButton(button));
    });
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &PluginDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &PluginDialog::reject);
}

void PluginDialog::saveFinished()
{
    if (m_widget->asyncSave())
        m_widget->setEnabled(true);
    m_buttonBox->button(QDialogButtonBox::Ok)->click();
}

void PluginDialog::slotButtonClicked(QDialogButtonBox::StandardButton button)
{
    switch (button) {
        case QDialogButtonBox::Reset:
            m_widget->load();
            break;
        case QDialogButtonBox::Ok:
            if (m_widget->asyncSave())
                m_widget->setEnabled(false);
            m_widget->save();
            if (!m_widget->asyncSave()) {
                m_buttonBox->button(button)->click();
            }
            break;
        default:
            m_buttonBox->button(button)->click();
    }
}

void PluginDialog::changed(bool changed)
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(changed);
    m_buttonBox->button(QDialogButtonBox::Reset)->setEnabled(changed);
}

}
