/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_LIB_MIGRATOR_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_LIB_MIGRATOR_H_

#include "fcitx5migrator_export.h"
#include <QObject>

namespace fcitx {

class Pipeline;

class FCITX5MIGRATOR_EXPORT Migrator : public QObject {
public:
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual bool check() const = 0;

    virtual bool hasOfflineJob() const = 0;
    virtual void addOfflineJob(Pipeline *) {}
    virtual void addOnlineJob(Pipeline *) {}
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_LIB_MIGRATOR_H_
