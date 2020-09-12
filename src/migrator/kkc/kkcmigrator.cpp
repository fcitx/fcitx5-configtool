/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "kkcmigrator.h"
#include "copydirectory.h"
#include "pipeline.h"
#include <QFileInfo>
#include <configmigrator.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/stringutils.h>
#include <fcntl.h>

namespace fcitx {

namespace {

QString fcitx4Path() {
    auto path = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::Config),
        "fcitx/kkc");
    return QString::fromStdString(path);
}

QString fcitx5Path() {
    auto path = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::PkgData),
        "kkc");
    return QString::fromStdString(path);
}

} // namespace

QString KkcMigrator::name() const { return _("Kkc"); }

QString KkcMigrator::description() const {
    return _("Migrate Kkc dictionary from Fcitx 4");
}

bool KkcMigrator::check() const {
    QFileInfo file(fcitx4Path());
    return file.isDir();
}

KkcMigrator *KkcMigratorPlugin::create() { return new KkcMigrator(); }

void KkcMigrator::addOfflineJob(Pipeline *pipeline) {
    auto migratorJob = new CopyDirectory(fcitx4Path(), fcitx5Path(), this);
    migratorJob->setExcludes({"^rule$"});
    pipeline->addJob(migratorJob);
}

void KkcMigrator::addOnlineJob(Pipeline *pipeline) {
    auto portRuleJob = new ConfigMigrator(
        "fcitx://config/addon/skk",
        [](RawConfig &config) {
            auto ruleFile = StandardPath::global().openUser(
                StandardPath::Type::Config, "fcitx/kkc/rule", O_RDONLY);
            QFile file;
            if (ruleFile.isValid() &&
                file.open(ruleFile.fd(), QIODevice::ReadOnly)) {
                auto rule = file.readAll().trimmed();
                if (!rule.isEmpty()) {
                    config.setValueByPath("Rule", rule.toStdString());
                    return true;
                }
            }
            return false;
        },
        this);
    portRuleJob->setStartMessage(_("Migrating Kkc rule option..."));
    pipeline->addJob(portRuleJob);
}

} // namespace fcitx
