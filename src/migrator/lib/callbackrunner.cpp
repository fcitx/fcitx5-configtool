/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "callbackrunner.h"
#include <QFutureWatcher>
#include <QtConcurrent>

namespace fcitx {

CallbackRunner::CallbackRunner(
    std::function<bool(CallbackRunner *runner)> callback, QObject *parent)
    : PipelineJob(parent), callback_(std::move(callback)) {}

void CallbackRunner::start() {
    cleanUp();
    futureWatcher_ = new QFutureWatcher<bool>(this);
    futureWatcher_->setFuture(QtConcurrent::run(callback_, this));
    connect(futureWatcher_, &QFutureWatcher<bool>::finished, this,
            [this]() { emitFinished(futureWatcher_->result()); });
}

void CallbackRunner::abort() {}

void CallbackRunner::cleanUp() {
    if (futureWatcher_) {
        disconnect(futureWatcher_, nullptr, this, nullptr);
        futureWatcher_->deleteLater();
        futureWatcher_ = nullptr;
    }
}

void CallbackRunner::sendMessage(const QString &icon, const QString &message) {
    QMetaObject::invokeMethod(
        this, [this, icon, message]() { this->message(icon, message); },
        Qt::QueuedConnection);
}

void CallbackRunner::emitFinished(bool result) {
    if (!result) {
        return;
    }
    Q_EMIT finished(result);
}

} // namespace fcitx
