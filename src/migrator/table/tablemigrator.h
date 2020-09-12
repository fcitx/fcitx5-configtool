/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_SKK_SKKMIGRATOR_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_SKK_SKKMIGRATOR_H_

#include "migrator.h"
#include "migratorfactoryplugin.h"

namespace fcitx {

class TableMigrator : public Migrator {
    Q_OBJECT
public:
    QString name() const override;
    QString description() const override;
    bool check() const override;
    bool hasOfflineJob() const override { return true; }
    void addOfflineJob(Pipeline *pipeline) override;
};

class TableMigratorPlugin : public FcitxMigratorFactoryPlugin {
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID FcitxMigratorFactoryInterface_iid FILE
                      "tablemigrator.json")
    TableMigrator *create() override;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_SKK_SKKMIGRATOR_H_
