/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "dbuscaller.h"
#include <fcitx-utils/i18n.h>

namespace fcitx {

DBusCaller::DBusCaller(std::function<QDBusPendingCallWatcher *()> callback,
                       const QString &startMessage,
                       const QString &finishMessage, QObject *parent)
    : PipelineJob(parent), callback_(std::move(callback)),
      startMessage_(startMessage), finishMessage_(finishMessage) {}

void DBusCaller::start() {
    watcher_ = callback_();
    if (!watcher_) {
        message("dialog-error", QString(_("Failed to start DBus Call.")));
        finished(false);
        return;
    }
    message("", startMessage_);
    connect(watcher_, &QDBusPendingCallWatcher::finished, [this]() {
        watcher_->deleteLater();
        if (watcher_->isError()) {
            message("dialog-error", QString(_("Got error: %1 %2"))
                                        .arg(watcher_->error().name(),
                                             watcher_->error().message()));
        } else {
            message("dialog-information", finishMessage_);
        }
        finished(!watcher_->isError());
        watcher_ = nullptr;
    });
}

void DBusCaller::abort() { cleanUp(); }

void DBusCaller::cleanUp() {
    watcher_->deleteLater();
    watcher_ = nullptr;
}

} // namespace fcitx
