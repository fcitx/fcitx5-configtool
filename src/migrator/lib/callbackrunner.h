/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_LIB_CALLBACKRUNNER_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_LIB_CALLBACKRUNNER_H_

#include "pipelinejob.h"
#include <QFutureWatcher>
#include <QObject>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT CallbackRunner : public PipelineJob {
    Q_OBJECT
public:
    explicit CallbackRunner(
        std::function<bool(CallbackRunner *runner)> callback,
        QObject *parent = nullptr);
    void start() override;
    void abort() override;
    void cleanUp() override;

    /// Function to be invoked by callback.
    void sendMessage(const QString &icon, const QString &message);
    void sendMessage(const QString &message) {
        sendMessage("dialog-information", message);
    }

private slots:
    void emitFinished(bool result);

private:
    const std::function<bool(CallbackRunner *runner)> callback_;
    QFutureWatcher<bool> *futureWatcher_ = nullptr;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_LIB_CALLBACKRUNNER_H_
