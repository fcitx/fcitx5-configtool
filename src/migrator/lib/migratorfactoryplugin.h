/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_MIGRATOR_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_MIGRATOR_H_

#include "fcitx5migrator_export.h"
#include "migrator.h"

namespace fcitx {

struct FCITX5MIGRATOR_EXPORT FcitxMigratorFactoryInterface {
    virtual Migrator *create() = 0;
    virtual ~FcitxMigratorFactoryInterface() = default;
};

#define FcitxMigratorFactoryInterface_iid                                      \
    "org.fcitx.Fcitx.FcitxMigratorFactoryInterface"
} // namespace fcitx

Q_DECLARE_INTERFACE(fcitx::FcitxMigratorFactoryInterface,
                    FcitxMigratorFactoryInterface_iid)

namespace fcitx {

class FCITX5MIGRATOR_EXPORT FcitxMigratorFactoryPlugin
    : public QObject,
      public FcitxMigratorFactoryInterface {
    Q_OBJECT
    Q_INTERFACES(fcitx::FcitxMigratorFactoryInterface)
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_MIGRATOR_H_
