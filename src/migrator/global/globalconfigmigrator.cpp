/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "globalconfigmigrator.h"
#include "configmigrator.h"
#include "pipeline.h"
#include <QFileInfo>
#include <QString>
#include <cstddef>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/standardpaths.h>
#include <fcitx-utils/stringutils.h>
#include <string>
#include <utility>
#include <vector>

namespace fcitx {

namespace {

QString fcitx4Path() {
    auto path =
        StandardPaths::global().userDirectory(StandardPathsType::Config) /
        "fcitx/config";
    return QString::fromStdString(path.string());
}

bool setKeyListToConfig(RawConfig &config, const std::string &path,
                        const KeyList &keyList) {
    if (keyList.empty()) {
        return false;
    }
    config.remove(path);

    for (size_t i = 0; i < keyList.size(); i++) {
        config.setValueByPath(stringutils::joinPath(path, i),
                              keyList[i].toString());
    }
    return true;
}

} // namespace

QString GlobalConfigMigrator::name() const { return _("Global Config"); }

QString GlobalConfigMigrator::description() const {
    return _("Migrate global key option from Fcitx 4");
}

bool GlobalConfigMigrator::check() const {
    QFileInfo file(fcitx4Path());
    return file.isFile();
}

GlobalConfigMigrator *GlobalConfigMigratorPlugin::create() {
    return new GlobalConfigMigrator();
}

void GlobalConfigMigrator::addOnlineJob(Pipeline *pipeline) {
    auto *portRuleJob = new ConfigMigrator(
        "fcitx://config/global",
        [](RawConfig &config) {
            auto configFile = StandardPaths::global().open(
                StandardPathsType::Config, "fcitx/config",
                StandardPathsMode::User);
            if (configFile.isValid()) {
                bool changed = false;
                RawConfig fcitx4Config;
                readFromIni(fcitx4Config, configFile.fd());

                std::vector<std::pair<std::string, std::string>> keyPairs = {
                    {"Hotkey/TriggerKey", "Hotkey/TriggerKeys"},
                    {"Hotkey/ActivateKey", "Hotkey/ActivateKeys"},
                    {"Hotkey/InactivateKey", "Hotkey/DeactivateKeys"}};
                for (const auto &[oldKey, newKey] : keyPairs) {
                    if (const auto *value = fcitx4Config.valueByPath(oldKey)) {
                        changed = setKeyListToConfig(
                                      config, newKey,
                                      Key::keyListFromString(*value)) ||
                                  changed;
                    }
                }

                return changed;
            }
            return false;
        },
        this);
    portRuleJob->setStartMessage(_("Migrating global hotkey key option..."));
    pipeline->addJob(portRuleJob);
}

} // namespace fcitx
