/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_LIB_MIGRATORFACTORY_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_LIB_MIGRATORFACTORY_H_

#include "fcitx5migrator_export.h"
#include "migrator.h"
#include <QObject>
#include <QSet>
#include <memory>
#include <vector>

namespace fcitx {
class MigratorFactoryPrivate;
/**
 * ui plugin factory.
 **/
class FCITX5MIGRATOR_EXPORT MigratorFactory : public QObject {
    Q_OBJECT
public:
    /**
     * create a plugin factory
     *
     * @param parent object parent
     **/
    explicit MigratorFactory(QObject *parent = 0);
    virtual ~MigratorFactory();

    std::vector<std::unique_ptr<Migrator>>
    list(const QSet<QString> &addons) const;

private:
    MigratorFactoryPrivate *d_ptr;
    Q_DECLARE_PRIVATE(MigratorFactory);
};
} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_LIB_MIGRATORFACTORY_H_
