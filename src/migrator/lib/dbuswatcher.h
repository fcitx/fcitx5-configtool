/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_LIB_DBUSWATCHER_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_LIB_DBUSWATCHER_H_

#include "fcitx5migrator_export.h"
#include "pipelinejob.h"
#include <QDBusServiceWatcher>
#include <QObject>
#include <QTimer>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT DBusWatcher : public PipelineJob {
    Q_OBJECT
public:
    explicit DBusWatcher(const QString &service, const QString &startMessage,
                         const QString &finishMessage, bool expectation,
                         QObject *parent = nullptr);
    void start() override;
    void abort() override;
    void cleanUp() override;

private slots:

private:
    QString service_;
    QString startMessage_;
    QString finishMessage_;
    QDBusServiceWatcher *watcher_;
    QTimer *timer_;
    bool available_ = false;
    bool firstCheck_ = true;
    const bool expectation_ = false;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_LIB_DBUSWATCHER_H_
