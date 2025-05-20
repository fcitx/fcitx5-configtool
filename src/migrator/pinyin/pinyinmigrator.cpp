/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "pinyinmigrator.h"
#include "pipeline.h"
#include "processrunner.h"
#include <QFileInfo>
#include <QString>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpaths.h>

namespace fcitx {

QString PinyinMigrator::name() const { return _("Pinyin"); }

QString PinyinMigrator::description() const {
    return _("Import Pinyin dictionary from Fcitx 4");
}

bool PinyinMigrator::check() const {
    auto path =
        StandardPaths::global().userDirectory(StandardPathsType::Config) /
        "fcitx/pinyin";
    QFileInfo file(QString::fromStdString(path.string()));
    return file.isDir();
}

PinyinMigrator *PinyinMigratorPlugin::create() { return new PinyinMigrator(); }

void PinyinMigrator::addOfflineJob(Pipeline *pipeline) {
    auto *migratorJob =
        new ProcessRunner("libime_migrate_fcitx4_pinyin", {}, {});
    migratorJob->setStartMessage(_("Migrating Pinyin data..."));
    migratorJob->setFinishMessage(_("Pinyin data is migrated."));
    pipeline->addJob(migratorJob);
}

} // namespace fcitx
