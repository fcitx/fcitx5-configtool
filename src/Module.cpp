/***************************************************************************
 *   Copyright (C) 2010~2011 by CSSlayer                                   *
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

#include "config.h"
#include "ui_Module.h"
#include "Module.h"

#include <QtCore/QFile>
#include <QPaintEngine>
#include <QDir>
#include <QDebug>

#include <KAboutData>
#include <KPluginFactory>
#include <KStandardDirs>
#include <kdebug.h>
#include <fcitx-utils/utils.h>
#include <fcitx/addon.h>
#include <fcitx-config/xdg.h>
#include "FcitxAddonSelector.h"
#include <libintl.h>
#include "FcitxConfigPage.h"
#include "ConfigDescManager.h"
#include "FcitxSubConfigParser.h"
#include "FcitxSkinPage.h"
#include "FcitxIMPage.h"
#include "FcitxIM.h"

K_PLUGIN_FACTORY_DECLARATION ( KcmFcitxFactory );

namespace Fcitx
{

const UT_icd addonicd= {sizeof ( FcitxAddon ), 0, 0, FreeAddon};

Module::Module ( QWidget *parent, const QVariantList &args ) :
        KCModule ( KcmFcitxFactory::componentData(), parent, args ),
        ui ( new Ui::Module ),
        addonSelector ( 0 ),
        m_configPage ( 0 ),
        m_configDescManager ( new ConfigDescManager ( this ) )
{
    bindtextdomain ( "fcitx", LOCALEDIR );
    bind_textdomain_codeset ( "fcitx", "UTF-8" );
    
    FcitxIM::registerMetaType();

    KAboutData *about = new KAboutData ( "kcm_fcitx", 0,
                                         ki18n ( "Fcitx Configuration Module" ),
                                         VERSION_STRING_FULL,
                                         ki18n ( "Configure Fcitx" ),
                                         KAboutData::License_GPL_V2,
                                         ki18n ( "Copyright 2011 Xuetian Weng" ),
                                         KLocalizedString(), QByteArray(),
                                         "wengxt@gmail.com" );

    about->addAuthor ( ki18n ( "Xuetian Weng" ), ki18n ( "Xuetian Weng" ), "wengxt@gmail.com" );
    setAboutData ( about );

    ui->setupUi ( this );
    KPageWidgetItem *page;
    {
        m_imPage = new FcitxIMPage(this);
        page = new KPageWidgetItem ( m_imPage );
        page->setName ( i18n ( "Input Method" ) );
        page->setIcon ( KIcon ( "draw-freehand" ) );
        page->setHeader ( i18n ( "Input Method" ) );
        ui->pageWidget->addPage ( page );
        connect ( m_imPage, SIGNAL ( changed() ), this, SLOT ( changed() ) );
    }
    
    {
        ConfigFileDesc* configDesc = m_configDescManager->GetConfigDesc ( "config.desc" );

        if ( configDesc )
        {
            m_configPage = new FcitxConfigPage ( this, configDesc, "", "config", "" );
            page = new KPageWidgetItem ( m_configPage );
            page->setName ( i18n ( "Global Config" ) );
            page->setIcon ( KIcon ( "fcitx" ) );
            page->setHeader ( i18n ( "Global Config for Fcitx" ) );
            ui->pageWidget->addPage ( page );
            connect ( m_configPage, SIGNAL ( changed() ), this, SLOT ( changed() ) );
        }
    }

    {
        if ( GetAddonConfigDesc() != NULL )
        {
            addonSelector = new FcitxAddonSelector ( this );
            page = new KPageWidgetItem ( addonSelector );
            page->setName ( i18n ( "Addon Config" ) );
            page->setIcon ( KIcon ( "preferences-plugin" ) );
            page->setHeader ( i18n ( "Configure Fcitx addon" ) );
            ui->pageWidget->addPage ( page );
        }
    }
    
    {
        m_skinPage = new FcitxSkinPage(this);
        page = new KPageWidgetItem ( m_skinPage );
        page->setName ( i18n ( "Manage Skin" ) );
        page->setIcon ( KIcon ( "get-hot-new-stuff" ) );
        page->setHeader ( i18n ( "Manage Fcitx Skin" ) );
        ui->pageWidget->addPage ( page );
        connect ( m_skinPage, SIGNAL ( changed() ), this, SLOT ( changed() ) );
    }

    if ( GetAddonConfigDesc() != NULL )
    {
        utarray_new ( m_addons, &addonicd );
        LoadAddonInfo ( m_addons );

        for ( FcitxAddon* addon = ( FcitxAddon * ) utarray_front ( m_addons );
                addon != NULL;
                addon = ( FcitxAddon * ) utarray_next ( m_addons, addon ) )
        {
            this->addonSelector->addAddon ( addon );
        }
    }
}

Module::~Module()
{
    delete ui;
    delete addonSelector;
    utarray_free(m_addons);
}

void Module::load()
{
    kDebug() << "Load Addon Info";
    
    m_imPage->load();
    m_skinPage->load();
    m_configPage->load();
}

void Module::save()
{
    m_imPage->save();
    m_skinPage->save();
    m_configPage->buttonClicked ( KDialog::Ok );
}

void Module::defaults()
{
    m_configPage->buttonClicked ( KDialog::Default );
    changed();
}

ConfigDescManager* Module::configDescManager()
{
    return m_configDescManager;
}
}
