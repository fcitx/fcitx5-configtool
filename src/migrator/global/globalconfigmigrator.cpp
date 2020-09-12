/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "globalconfigmigrator.h"
#include "configmigrator.h"
#include "copydirectory.h"
#include "pipeline.h"
#include <QFileInfo>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/stringutils.h>
#include <fcntl.h>

namespace fcitx {

namespace {

QString fcitx4Path() {
    auto path = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::Config),
        "fcitx/config");
    return QString::fromStdString(path);
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
    auto portRuleJob = new ConfigMigrator(
        "fcitx://config/global",
        [](RawConfig &config) {
            auto configFile = StandardPath::global().openUser(
                StandardPath::Type::Config, "fcitx/config", O_RDONLY);
            if (configFile.isValid()) {
                bool changed = false;
                RawConfig fcitx4Config;
                readFromIni(fcitx4Config, configFile.fd());

                std::vector<std::pair<std::string, std::string>> keyPairs = {
                    {"Hotkey/TriggerKey", "Hotkey/TriggerKeys"},
                    {"Hotkey/ActivateKey", "Hotkey/ActivateKeys"},
                    {"Hotkey/InactivateKey", "Hotkey/DeactivateKeys"}};
                for (const auto &[oldKey, newKey] : keyPairs) {
                    if (auto *value = fcitx4Config.valueByPath(oldKey)) {
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
