/*
 * SPDX-FileCopyrightText: 2018-2018 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _PINYINDICTMANAGER_PIPELINEJOB_H_
#define _PINYINDICTMANAGER_PIPELINEJOB_H_

#include "fcitx5migrator_export.h"
#include <QObject>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT PipelineJob : public QObject {
    Q_OBJECT
public:
    PipelineJob(QObject *parent = nullptr);

    virtual void start() = 0;
    virtual void abort() = 0;
    virtual void cleanUp() = 0;

signals:
    void finished(bool success);
    void message(const QString &icon, const QString &message);
};

} // namespace fcitx

#endif // _PINYINDICTMANAGER_PIPELINEJOB_H_
