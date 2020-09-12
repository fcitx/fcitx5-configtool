/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "configmigrator.h"
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/i18n.h>

namespace fcitx {

namespace {

void decomposeDBusVariant(RawConfig &config, const QVariant &v) {
    QVariantMap map;
    if (v.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(v);
        argument >> map;
    } else if (v.canConvert<QString>()) {
        config.setValue(v.toString().toStdString());
    } else {
        map = v.toMap();
    }
    for (const auto &item :
         MakeIterRange(map.constKeyValueBegin(), map.constKeyValueEnd())) {
        decomposeDBusVariant(*config.get(item.first.toStdString(), true),
                             item.second);
    }
}

QVariant rawConfigToVariant(const RawConfig &config) {
    if (!config.hasSubItems()) {
        return QString::fromStdString(config.value());
    }

    QVariantMap map;
    if (!config.value().empty()) {
        map[""] = QString::fromStdString(config.value());
    }
    if (config.hasSubItems()) {
        auto options = config.subItems();
        for (auto &option : options) {
            auto subConfig = config.get(option);
            map[QString::fromStdString(option)] =
                rawConfigToVariant(*subConfig);
        }
    }
    return map;
}

} // namespace

ConfigMigrator::ConfigMigrator(const QString &configPath,
                               std::function<bool(RawConfig &)> transformer,
                               QObject *parent)
    : PipelineJob(parent), configPath_(configPath), proxy_(nullptr),
      transformer_(std::move(transformer)) {}

void ConfigMigrator::start() {
    if (proxy_) {
        delete proxy_;
    }
    if (!startMessage_.isEmpty()) {
        emit message("dialog-information", startMessage_);
    }
    proxy_ = new FcitxQtControllerProxy("org.fcitx.Fcitx5", "/controller",
                                        QDBusConnection::sessionBus(), this);
    auto call = proxy_->GetConfig(configPath_);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &ConfigMigrator::requestConfigFinished);
}

void ConfigMigrator::abort() {}

void ConfigMigrator::cleanUp() {
    delete proxy_;
    proxy_ = nullptr;
}

void fcitx::ConfigMigrator::requestConfigFinished(
    QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    QDBusPendingReply<QDBusVariant, FcitxQtConfigTypeList> reply = *watcher;
    if (reply.isError()) {
        emit message(
            "dialog-error",
            QString(_("Failed to fetch config for %1")).arg(configPath_));
        emit finished(false);
        return;
    }

    auto variant = reply.argumentAt<0>().variant();
    config_ = RawConfig();
    decomposeDBusVariant(config_, variant);
    // No need for update.
    if (!transformer_(config_)) {
        emit finished(true);
        return;
    }

    auto newVariant = rawConfigToVariant(config_);
    proxy_->SetConfig(configPath_, QDBusVariant(newVariant));
    if (!finishMessage_.isEmpty()) {
        message("dialog-information", finishMessage_);
    }
    emit finished(true);
}

} // namespace fcitx
