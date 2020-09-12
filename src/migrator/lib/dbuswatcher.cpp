/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "dbuswatcher.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QTimer>
#include <fcitx-utils/i18n.h>

namespace fcitx {

DBusWatcher::DBusWatcher(const QString &service, const QString &startMessage,
                         const QString &finishMessage, bool expectation,
                         QObject *parent)
    : PipelineJob(parent), service_(service), startMessage_(startMessage),
      finishMessage_(finishMessage), watcher_(new QDBusServiceWatcher(this)),
      timer_(new QTimer(this)), expectation_(expectation) {
    connect(watcher_, &QDBusServiceWatcher::serviceRegistered, this,
            [this]() { available_ = true; });
    connect(watcher_, &QDBusServiceWatcher::serviceUnregistered, this,
            [this]() { available_ = false; });
    watcher_->setConnection(QDBusConnection::sessionBus());
    watcher_->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    watcher_->addWatchedService({service});
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, [this]() {
        if (available_ != expectation_) {
            if (available_) {
                message("dialog-warning",
                        QString(_("Service %1 still present on DBus."))
                            .arg(service_));
            } else {
                message("dialog-warning",
                        QString(_("Service %1 does not present on DBus."))
                            .arg(service_));
            }
            if (firstCheck_) {
                timer_->setInterval(4000);
                firstCheck_ = false;
                timer_->start();
            } else {
                finished(false);
            }
        } else {
            message("dialog-information", finishMessage_);
            finished(true);
        }
    });
}

void DBusWatcher::start() {
    firstCheck_ = true;
    message("dialog-information", startMessage_);
    available_ =
        watcher_->connection().interface()->isServiceRegistered(service_);
    if (available_ != expectation_) {
        timer_->setInterval(1000);
        timer_->start();
    } else {
        message("dialog-information", finishMessage_);
        finished(true);
    }
}

void DBusWatcher::abort() { cleanUp(); }

void DBusWatcher::cleanUp() { timer_->stop(); }

} // namespace fcitx
