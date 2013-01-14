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

#include "configpage.h"
#include "configdescmanager.h"
#include "configwidget.h"
#include "ui_configpage.h"

namespace Fcitx {

ConfigPage::ConfigPage(QWidget* parent): QWidget(parent)
    ,m_ui(new Ui::ConfigPage)
{
    m_ui->setupUi(this);
    FcitxConfigFileDesc* configDesc = ConfigDescManager::instance()->GetConfigDesc("config.desc");
    m_configWidget = new ConfigWidget(configDesc, "", "config", "");
    m_ui->layout->insertWidget(0, m_configWidget);
    m_ui->infoIconLabel->setPixmap(KIcon("dialog-information").pixmap(KIconLoader::SizeSmallMedium));

    connect(m_configWidget, SIGNAL(changed()), this, SIGNAL(changed()));
}

ConfigPage::~ConfigPage()
{
    delete m_ui;
}

void ConfigPage::load()
{
    m_configWidget->load();
}

void ConfigPage::save()
{
    m_configWidget->buttonClicked(KDialog::Ok);
}

void ConfigPage::defaults()
{
    m_configWidget->buttonClicked(KDialog::Default);
}



}
