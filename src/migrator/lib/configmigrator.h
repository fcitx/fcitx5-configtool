/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_LIB_CONFIGMIGRATOR_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_LIB_CONFIGMIGRATOR_H_

#include "fcitx5migrator_export.h"
#include "pipelinejob.h"
#include <fcitx-config/rawconfig.h>
#include <fcitxqtcontrollerproxy.h>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT ConfigMigrator : public PipelineJob {
public:
    ConfigMigrator(const QString &config,
                   std::function<bool(RawConfig &)> transformer,
                   QObject *parent);

    void start() override;
    void abort() override;
    void cleanUp() override;

public slots:
    void requestConfigFinished(QDBusPendingCallWatcher *watcher);

    QString valueByPath(const QString &path) const;
    void setValueByPath(const QString &path);
    void removeValueByPath(const QString &path);

    void setStartMessage(const QString &message) { startMessage_ = message; }
    void setFinishMessage(const QString &message) { finishMessage_ = message; }

private:
    QString startMessage_;
    QString finishMessage_;

    QString configPath_;
    FcitxQtControllerProxy *proxy_ = nullptr;
    RawConfig config_;
    std::function<bool(RawConfig &)> transformer_;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_LIB_CONFIGMIGRATOR_H_
