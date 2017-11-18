/*
* Copyright (C) 2017~2017 by CSSlayer
* wengxt@gmail.com
*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of the
* License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; see the file COPYING. If not,
* see <http://www.gnu.org/licenses/>.
*/

#include "module.h"
#include "addonselector.h"
#include "config.h"
#include "configwidget.h"
#include "erroroverlay.h"
#include "impage.h"
#include "logging.h"
#include <KAboutData>
#include <QScrollArea>
#include <fcitx-utils/standardpath.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtwatcher.h>

namespace fcitx {
namespace kcm {

namespace {
QWidget *forwardHelper(QWidget *parent) {
    KLocalizedString::addDomainLocaleDir(
        "kcm_fcitx5",
        QString::fromLocal8Bit(StandardPath::fcitxPath("localedir")));
    return parent;
}
} // namespace

Module::Module(QWidget *parent, const QVariantList &args)
    : KCModule(forwardHelper(parent), args),
      watcher_(new FcitxQtWatcher(QDBusConnection::sessionBus(), this)),
      errorOverlay_(new ErrorOverlay(this)), impage_(new IMPage(this)),
      addonPage_(new AddonSelector(this)),
      configPage_(new ConfigWidget("fcitx://config/global", this)) {
    registerFcitxQtDBusTypes();

    KAboutData *about = new KAboutData(
        "kcm_fcitx5", i18n("Fcitx 5 Configuration Module"), PROJECT_VERSION,
        i18n("Configure Fcitx 5"), KAboutLicense::LGPL_V2,
        i18n("Copyright 2017 Xuetian Weng"), QString(), QString(),
        "wengxt@gmail.com");

    about->addAuthor(i18n("Xuetian Weng"), i18n("Author"), "wengxt@gmail.com");
    setAboutData(about);
    setupUi(this);

    connect(watcher_, &FcitxQtWatcher::availabilityChanged, this,
            &Module::fcitxAvailabilityChanged);
    watcher_->watch();
    pageWidget->addTab(impage_, i18n("Input Method"));
    connect(impage_, &IMPage::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "IMPage changed";
        changed();
    });
    pageWidget->addTab(addonPage_, i18n("Addon"));
    connect(addonPage_, &AddonSelector::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "AddonSelector changed";
        changed();
    });
    auto configPageWrapper = new QScrollArea;
    configPageWrapper->setFrameStyle(QFrame::NoFrame);
    configPageWrapper->setWidgetResizable(true);
    configPageWrapper->setWidget(configPage_);
    pageWidget->addTab(configPageWrapper, i18n("Global Config"));
    connect(configPage_, &ConfigWidget::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "GlobalConfig changed";
        changed();
    });
}

Module::~Module() { watcher_->unwatch(); }

void Module::load() {
    qCDebug(KCM_FCITX5) << "kcm_fcitx5 load()";
    impage_->load();
    addonPage_->load();
    configPage_->load();
}

void Module::save() {
    qCDebug(KCM_FCITX5) << "kcm_fcitx5 save()";
    impage_->save();
    addonPage_->save();
    configPage_->save();
}

void Module::defaults() {
    qCDebug(KCM_FCITX5) << "kcm_fcitx5 defaults()";
    changed();
}

void Module::fcitxAvailabilityChanged(bool avail) {
    delete controller_;
    controller_ = nullptr;

    if (avail) {
        controller_ =
            new FcitxQtControllerProxy(watcher_->serviceName(), "/controller",
                                       watcher_->connection(), this);
        controller_->setTimeout(3000);
    }

    emit availabilityChanged(controller_);
}

} // namespace kcm
} // namespace fcitx
