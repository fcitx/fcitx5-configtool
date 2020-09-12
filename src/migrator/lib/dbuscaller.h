/*
 * SPDX-FileCopyrightText: 2013-2018 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_LIB_DBUSCALLER_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_LIB_DBUSCALLER_H_

#include "fcitx5migrator_export.h"
#include "pipelinejob.h"
#include <QDBusPendingCallWatcher>
#include <QObject>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT DBusCaller : public PipelineJob {
    Q_OBJECT
public:
    explicit DBusCaller(std::function<QDBusPendingCallWatcher *()> callback,
                        const QString &startMessage,
                        const QString &finishMessage,
                        QObject *parent = nullptr);
    void start() override;
    void abort() override;
    void cleanUp() override;

private slots:

private:
    std::function<QDBusPendingCallWatcher *()> callback_;
    QString startMessage_;
    QString finishMessage_;
    QDBusPendingCallWatcher *watcher_;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_LIB_DBUSCALLER_H_
