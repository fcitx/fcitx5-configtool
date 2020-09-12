/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_LIB_COPYDIRECTORY_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_LIB_COPYDIRECTORY_H_

#include "callbackrunner.h"
#include <QFutureWatcher>
#include <QObject>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT CopyDirectory : public CallbackRunner {
    Q_OBJECT
public:
    explicit CopyDirectory(const QString &from, const QString &to,
                           QObject *parent = nullptr);

    void setExcludes(const QStringList &excludes) { excludes_ = excludes; }

private:
    QString from_, to_;
    QStringList excludes_;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_LIB_COPYDIRECTORY_H_
