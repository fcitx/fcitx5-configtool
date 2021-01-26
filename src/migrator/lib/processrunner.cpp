/*
 * SPDX-FileCopyrightText: 2013-2018 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "processrunner.h"
#include "log.h"
#include <QDebug>
#include <QProcess>
#include <QTemporaryFile>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>

namespace fcitx {

ProcessRunner::ProcessRunner(const QString &bin, const QStringList &args,
                             const QString &file, QObject *parent)
    : PipelineJob(parent), bin_(bin), args_(args), file_(file) {
    connect(&process_,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            &ProcessRunner::processFinished);
    connect(&process_, &QProcess::readyReadStandardOutput, this, [this] {
        QByteArray bytes = process_.readAllStandardOutput();
        messages_.append(bytes);
    });
}

void ProcessRunner::start() {
    messages_.clear();
    if (process_.state() != QProcess::NotRunning) {
        process_.kill();
    }

    if (!startMessage_.isEmpty()) {
        message("dialog-information", startMessage_);
    }

    if (printOutputToMessage_) {
        process_.setProcessChannelMode(QProcess::MergedChannels);
    }
    process_.start(bin_, args_);
    if (printOutputToMessage_) {
        process_.closeReadChannel(QProcess::StandardError);
        process_.setReadChannel(QProcess::StandardOutput);
    } else {
        process_.closeReadChannel(QProcess::StandardError);
        process_.closeReadChannel(QProcess::StandardOutput);
    }
}

void ProcessRunner::abort() { process_.kill(); }

void ProcessRunner::cleanUp() {
    if (!file_.isEmpty()) {
        QFile::remove(file_);
    }
}

void ProcessRunner::processFinished(int exitCode, QProcess::ExitStatus status) {
    if (printOutputToMessage_) {
        for (const auto &line : messages_.split('\n')) {
            if (!line.isEmpty()) {
                Q_EMIT message("dialog-information", line);
            }
        }
    }
    if (status == QProcess::CrashExit) {
        Q_EMIT message("dialog-error", QString(_("%1 crashed.")).arg(file_));
        Q_EMIT finished(ignoreFailure_);
        return;
    }

    if (exitCode != 0) {
        Q_EMIT message("dialog-warning",
                       QString(_("%1 failed to start.")).arg(file_));
        Q_EMIT finished(ignoreFailure_);
        return;
    }

    if (!finishMessage_.isEmpty()) {
        Q_EMIT message("dialog-information", finishMessage_);
    }
    Q_EMIT finished(true);
}

} // namespace fcitx
