/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "tablemigrator.h"
#include "pipeline.h"
#include "processrunner.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpaths.h>

namespace fcitx {

namespace {

QString toolPath() {
    auto path = QStandardPaths::findExecutable("libime_migrate_fcitx4_table");
    if (path.isEmpty()) {
        path = QStandardPaths::findExecutable(
            "libime_migrate_fcitx4_table",
            {QString::fromStdString(
                StandardPaths::fcitxPath("bindir").string())});
    }
    return path;
}

QString fcitx4Path() {
    auto path =
        StandardPaths::global().userDirectory(StandardPathsType::Config) /
        "fcitx/table";
    return QString::fromStdString(path.string());
}

} // namespace

QString TableMigrator::name() const { return _("Table"); }

QString TableMigrator::description() const {
    return _("Import table data for installed table from Fcitx 4");
}

bool TableMigrator::check() const {
    QFileInfo file(fcitx4Path());
    return file.isDir() && !toolPath().isEmpty();
}

TableMigrator *TableMigratorPlugin::create() { return new TableMigrator(); }

void TableMigrator::addOfflineJob(Pipeline *pipeline) {
    auto tool = toolPath();
    if (tool.isEmpty()) {
        return;
    }
    QDir tableDir(fcitx4Path());
    for (const auto &name :
         tableDir.entryList({"*.mb"}, QDir::Files | QDir::Readable)) {
        auto *processRunner = new ProcessRunner(tool, {name}, {}, this);
        processRunner->setIgnoreFailure(true);
        processRunner->setPrintOutputToMessage(true);
        processRunner->setStartMessage(
            QString(_("Try to Migrating table file: %1 ...")).arg(name));
        pipeline->addJob(processRunner);
    }
}

} // namespace fcitx
