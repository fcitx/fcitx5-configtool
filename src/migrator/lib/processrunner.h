/*
 * SPDX-FileCopyrightText: 2013-2018 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _PINYINDICTMANAGER_PROCESSRUNNER_H_
#define _PINYINDICTMANAGER_PROCESSRUNNER_H_

#include "fcitx5migrator_export.h"
#include "pipelinejob.h"
#include <QObject>
#include <QProcess>

namespace fcitx {

class FCITX5MIGRATOR_EXPORT ProcessRunner : public PipelineJob {
    Q_OBJECT
public:
    explicit ProcessRunner(const QString &bin, const QStringList &args,
                           const QString &file, QObject *parent = nullptr);
    void start() override;
    void abort() override;
    void cleanUp() override;
    void setIgnoreFailure(bool ignoreFailure) {
        ignoreFailure_ = ignoreFailure;
    }
    void setPrintOutputToMessage(bool printOutputToMessage) {
        printOutputToMessage_ = printOutputToMessage;
    }

    void setStartMessage(const QString &message) { startMessage_ = message; }
    void setFinishMessage(const QString &message) { finishMessage_ = message; }

private slots:
    void processFinished(int exitCode, QProcess::ExitStatus status);

private:
    QString startMessage_;
    QString finishMessage_;
    QProcess process_;
    QString bin_;
    QStringList args_;
    QString file_;
    bool ignoreFailure_ = false;
    bool printOutputToMessage_ = false;

    QByteArray messages_;
};

} // namespace fcitx

#endif // _PINYINDICTMANAGER_PROCESSRUNNER_H_
