/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "skkmigrator.h"
#include "configmigrator.h"
#include "copydirectory.h"
#include "pipeline.h"
#include <QFileInfo>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/stringutils.h>
#include <fcntl.h>

namespace fcitx {

namespace {

QString fcitx4Path() {
    auto path = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::Config),
        "fcitx/skk");
    return QString::fromStdString(path);
}

QString fcitx5Path() {
    auto path = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::PkgData),
        "skk");
    return QString::fromStdString(path);
}

} // namespace

QString SkkMigrator::name() const { return _("Skk"); }

QString SkkMigrator::description() const {
    return _("Migrate Skk dictionary from Fcitx 4");
}

bool SkkMigrator::check() const {
    QFileInfo file(fcitx4Path());
    return file.isDir();
}

SkkMigrator *SkkMigratorPlugin::create() { return new SkkMigrator(); }

void SkkMigrator::addOfflineJob(Pipeline *pipeline) {
    auto migratorJob = new CopyDirectory(fcitx4Path(), fcitx5Path(), this);
    migratorJob->setExcludes({"^rule$"});
    pipeline->addJob(migratorJob);
}

void SkkMigrator::addOnlineJob(Pipeline *pipeline) {
    auto portRuleJob = new ConfigMigrator(
        "fcitx://config/addon/skk",
        [](RawConfig &config) {
            auto ruleFile = StandardPath::global().openUser(
                StandardPath::Type::Config, "fcitx/skk/rule", O_RDONLY);
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
    portRuleJob->setStartMessage(_("Migrating Skk rule option..."));
    pipeline->addJob(portRuleJob);
}

} // namespace fcitx
