/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "dbusprovider.h"

namespace fcitx {
namespace kcm {

DBusProvider::DBusProvider(QObject *parent)
    : QObject(parent),
      watcher_(new FcitxQtWatcher(QDBusConnection::sessionBus(), this)) {
    registerFcitxQtDBusTypes();
    connect(watcher_, &FcitxQtWatcher::availabilityChanged, this,
            &DBusProvider::fcitxAvailabilityChanged);
    watcher_->watch();
}

DBusProvider::~DBusProvider() { watcher_->unwatch(); }

void DBusProvider::fcitxAvailabilityChanged(bool avail) {
    delete controller_;
    controller_ = nullptr;

    if (avail) {
        controller_ =
            new FcitxQtControllerProxy(watcher_->serviceName(), "/controller",
                                       watcher_->connection(), this);
        controller_->setTimeout(3000);

        loadCanRestart();
    }

    Q_EMIT availabilityChanged(controller_);
}

void DBusProvider::setCanRestart(bool canRestart) {
    if (canRestart_ != canRestart) {
        canRestart_ = canRestart;
        Q_EMIT canRestartChanged(canRestart_);
    }
}

void DBusProvider::loadCanRestart() {
    auto call = controller_->CanRestart();
    auto callwatcher = new QDBusPendingCallWatcher(call, this);
    connect(callwatcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<bool> canRestart = *watcher;
                watcher->deleteLater();
                if (canRestart.isValid()) {
                    setCanRestart(canRestart.value());
                }
            });
}

} // namespace kcm
} // namespace fcitx
