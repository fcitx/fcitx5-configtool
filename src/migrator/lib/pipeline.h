/*
 * SPDX-FileCopyrightText: 2018-2018 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _PINYINDICTMANAGER_PIPELINE_H_
#define _PINYINDICTMANAGER_PIPELINE_H_

#include "fcitx5migrator_export.h"
#include "pipelinejob.h"
#include <QObject>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT Pipeline : public QObject {
    Q_OBJECT
public:
    Pipeline(QObject *parent = nullptr);

    void addJob(PipelineJob *job);
    void start();
    void abort();
    void reset();
    const std::vector<PipelineJob *> &jobs() { return jobs_; }

Q_SIGNALS:
    void finished(bool);
    void message(const QString &icon, const QString &message);

private:
    void startNext();
    void emitFinished(bool);

    std::vector<PipelineJob *> jobs_;
    int index_ = -1;
};

} // namespace fcitx

#endif // _PINYINDICTMANAGER_PIPELINE_H_
