/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "rimemigrator.h"
#include "callbackrunner.h"
#include "copydirectory.h"
#include "pipeline.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpaths.h>
#include <fcitx-utils/stringutils.h>

namespace fcitx {

namespace {

QString fcitx4Path() {
    auto path =
        StandardPaths::global().userDirectory(StandardPathsType::Config) /
        "fcitx/rime";
    return QString::fromStdString(path.string());
}

QString fcitx5Path() {
    auto path =
        StandardPaths::global().userDirectory(StandardPathsType::PkgData) /
        "rime";
    return QString::fromStdString(path.string());
}

} // namespace

QString RimeMigrator::name() const { return _("Rime"); }

QString RimeMigrator::description() const {
    return _("Migrate Rime dictionary from Fcitx 4. Notice: this will remove "
             "all the existing data.");
}

bool RimeMigrator::check() const {
    QFileInfo file(fcitx4Path());
    return file.isDir();
}

RimeMigrator *RimeMigratorPlugin::create() { return new RimeMigrator(); }

void RimeMigrator::addOfflineJob(Pipeline *pipeline) {
    pipeline->addJob(new CallbackRunner(
        [](CallbackRunner *) {
            QDir dir(fcitx5Path());
            return dir.removeRecursively();
        },
        this));
    auto *migratorJob = new CopyDirectory(fcitx4Path(), fcitx5Path(), this);
    pipeline->addJob(migratorJob);
}

} // namespace fcitx
