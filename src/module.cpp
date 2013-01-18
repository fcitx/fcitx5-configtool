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


// Qt
#include <QFile>
#include <QPaintEngine>
#include <QDir>
#include <QDebug>

// KDE
#include <KAboutData>
#include <KPluginFactory>
#include <KStandardDirs>
#include <KDebug>

// system
#include <libintl.h>

// Fcitx
#include <fcitx-utils/utils.h>
#include <fcitx/addon.h>
#include <fcitx-config/xdg.h>
#include <fcitx/module/dbus/dbusstuff.h>
#include <fcitx/module/ipc/ipc.h>
#include <fcitx-qt/fcitxqtkeyboardlayout.h>
#include <fcitx-qt/fcitxqtconnection.h>

// self
#include "config.h"
#include "ui_module.h"
#include "module.h"
#include "addonselector.h"
#include "configwidget.h"
#include "configdescmanager.h"
#include "subconfigparser.h"
#include "skinpage.h"
#include "impage.h"
#include "imconfigdialog.h"
#include "uipage.h"
#include "configpage.h"

K_PLUGIN_FACTORY_DECLARATION(KcmFcitxFactory);

namespace Fcitx
{

const UT_icd addonicd = {sizeof(FcitxAddon), 0, 0, FcitxAddonFree};

Module::Module(QWidget *parent, const QVariantList &args) :
    KCModule(KcmFcitxFactory::componentData(), parent, args),
    ui(new Ui::Module),
    addonSelector(0),
    m_configPage(0),
    m_skinPage(0),
    m_imPage(0)
{
    bindtextdomain("fcitx", LOCALEDIR);
    bind_textdomain_codeset("fcitx", "UTF-8");

    FcitxQtInputMethodItem::registerMetaType();
    FcitxQtKeyboardLayout::registerMetaType();

    KAboutData *about = new KAboutData("kcm_fcitx", 0,
                                       ki18n("Fcitx Configuration Module"),
                                       VERSION_STRING_FULL,
                                       ki18n("Configure Fcitx"),
                                       KAboutData::License_GPL_V2,
                                       ki18n("Copyright 2012 Xuetian Weng"),
                                       KLocalizedString(), QByteArray(),
                                       "wengxt@gmail.com");

    about->addAuthor(ki18n("Xuetian Weng"), ki18n("Xuetian Weng"), "wengxt@gmail.com");
    setAboutData(about);

    if (FcitxAddonGetConfigDesc() != NULL) {
        utarray_new(m_addons, &addonicd);
        FcitxAddonsLoad(m_addons);
    }

    ConfigDescManager::instance();

    ui->setupUi(this);
    KPageWidgetItem *page;
    {
        m_imPage = new IMPage(this);
        page = new KPageWidgetItem(m_imPage);
        page->setName(i18n("Input Method"));
        page->setIcon(KIcon("draw-freehand"));
        page->setHeader(i18n("Input Method"));
        ui->pageWidget->addPage(page);
        connect(m_imPage, SIGNAL(changed()), this, SLOT(changed()));
    }

    {
        FcitxConfigFileDesc* configDesc = ConfigDescManager::instance()->GetConfigDesc("config.desc");

        if (configDesc) {
            m_configPage = new ConfigPage;
            page = new KPageWidgetItem(m_configPage);
            page->setName(i18n("Global Config"));
            page->setIcon(KIcon("fcitx"));
            page->setHeader(i18n("Global Config for Fcitx"));
            ui->pageWidget->addPage(page);
            connect(m_configPage, SIGNAL(changed()), this, SLOT(changed()));
        }
    }

    {
        if (ConfigDescManager::instance()->inputMethodProxy()) {
            m_uiPage = new UIPage(this);
            page = new KPageWidgetItem(m_uiPage);
            page->setName(i18n("Appearance"));
            page->setIcon(KIcon("preferences-desktop-theme"));
            page->setHeader(i18n("Appearance"));
            ui->pageWidget->addPage(page);
            connect(m_uiPage, SIGNAL(changed()), this, SLOT(changed()));
        }
    }

    {
        if (FcitxAddonGetConfigDesc() != NULL) {
            addonSelector = new AddonSelector(this);
            page = new KPageWidgetItem(addonSelector);
            page->setName(i18n("Addon Config"));
            page->setIcon(KIcon("preferences-plugin"));
            page->setHeader(i18n("Configure Fcitx addon"));
            ui->pageWidget->addPage(page);
        }
    }

    {
        m_skinPage = new SkinPage(this);
        page = new KPageWidgetItem(m_skinPage);
        page->setName(i18n("Manage Skin"));
        page->setIcon(KIcon("get-hot-new-stuff"));
        page->setHeader(i18n("Manage Fcitx Skin"));
        ui->pageWidget->addPage(page);
        connect(m_skinPage, SIGNAL(changed()), this, SLOT(changed()));
    }

    if (m_addons) {
        for (FcitxAddon* addon = (FcitxAddon *) utarray_front(m_addons);
                addon != NULL;
                addon = (FcitxAddon *) utarray_next(m_addons, addon)) {
            this->addonSelector->addAddon(addon);
        }
    }

    if (args.size() != 0) {
        m_arg = args[0].toString();
    }
}

Module::~Module()
{
    delete ui;
    if (addonSelector)
        delete addonSelector;
    if (m_addons)
        utarray_free(m_addons);
    ConfigDescManager::deInit();
}

FcitxAddon* Module::findAddonByName(const QString& name)
{
    FcitxAddon* addon = NULL;
    for (addon = (FcitxAddon *) utarray_front(m_addons);
         addon != NULL;
         addon = (FcitxAddon *) utarray_next(m_addons, addon)) {
        if (QString::fromUtf8(addon->name) == name)
            break;
    }
    return addon;
}


void Module::load()
{;
    KDialog* configDialog = NULL;
    if (!m_arg.isEmpty()) {
        do {
            if (!ConfigDescManager::instance()->inputMethodProxy())
                break;
            QDBusPendingReply< QString > result = ConfigDescManager::instance()->inputMethodProxy()->GetIMAddon(m_arg);
            result.waitForFinished();
            if (!result.isValid() || result.value().isEmpty())
                break;
            FcitxAddon* addonEntry = findAddonByName(result.value());
            if (!addonEntry)
                break;
            configDialog = new IMConfigDialog(m_arg, addonEntry, 0);
        } while(0);
        if (!configDialog) {
            FcitxAddon* addonEntry = findAddonByName(m_arg);
            if (addonEntry)
                configDialog = ConfigWidget::configDialog(0, addonEntry);
        }
        if (configDialog) {
            configDialog->setAttribute(Qt::WA_DeleteOnClose);
            configDialog->open();
        }
        m_arg = QString::null;
    }

    if (m_imPage)
        m_imPage->load();
    if (m_skinPage)
        m_skinPage->load();
    if (m_configPage)
        m_configPage->load();
}

void Module::save()
{

    if (m_imPage)
        m_imPage->save();
    if (m_skinPage)
        m_skinPage->save();
    if (m_configPage)
        m_configPage->save();
    if (m_uiPage)
        m_uiPage->save();
}

void Module::defaults()
{
    if (m_configPage)
        m_configPage->defaults();
    changed();
}

}
