//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; see the file COPYING. If not,
// see <http://www.gnu.org/licenses/>.
//
#include "dbusprovider.h"

namespace fcitx {
namespace kcm {

fcitx::kcm::DBusProvider::DBusProvider(QObject *parent)
    : QObject(parent),
      watcher_(new FcitxQtWatcher(QDBusConnection::sessionBus(), this)) {
    registerFcitxQtDBusTypes();
    connect(watcher_, &FcitxQtWatcher::availabilityChanged, this,
            &DBusProvider::fcitxAvailabilityChanged);
    watcher_->watch();
}

DBusProvider::~DBusProvider() { watcher_->unwatch(); }

void fcitx::kcm::DBusProvider::fcitxAvailabilityChanged(bool avail) {
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
