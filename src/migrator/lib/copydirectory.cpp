/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "copydirectory.h"
#include <QFutureWatcher>
#include <QtConcurrent>
#include <fcitx-utils/fs.h>
#include <fcitx-utils/i18n.h>
#if __GNUC__ <= 7
#include <boost/filesystem.hpp>
#else
#include <filesystem>
#endif
#include <unistd.h>

#if defined(Q_OS_LINUX)
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>

// in case linux/fs.h is too old and doesn't define it:
#ifndef FICLONE
#define FICLONE _IOW(0x94, 9, int)
#endif
#endif

namespace fcitx {

namespace {

bool cloneFile(int srcfd, int dstfd) {
#if defined(Q_OS_LINUX)
    if (ioctl(dstfd, FICLONE, srcfd) == 0) {
        return true;
    }

    size_t sendFileLimit = 0x7ffff000;
    ssize_t n = sendfile(dstfd, srcfd, NULL, sendFileLimit);
    if (n != -1) {
        while (n) {
            n = sendfile(dstfd, srcfd, NULL, sendFileLimit);
            if (n == -1) {
                return false;
            }
        }
        return true;
    }

    // Check errno as suggested in manpage.
    if (errno != EINVAL && errno != ENOSYS) {
        return false;
    }
#endif
    // Fallback to write.

    char buffer[4096];
    while (true) {
        auto in = fs::safeRead(srcfd, buffer, sizeof(buffer));
        if (in < 0) {
            return false;
        }
        if (in == 0) {
            break;
        }
        if (in != fs::safeWrite(dstfd, buffer, in)) {
            return false;
        }
    }
    return true;
}

bool checkFileName(const QString &name,
                   const QList<QRegularExpression> &excludes) {
    for (const auto &regexp : excludes) {
        if (regexp.match(name).hasMatch()) {
            return false;
        }
    }
    return true;
}

bool copyDirRecursive(CopyDirectory *that, const QString &from,
                      const QString &to,
                      const QList<QRegularExpression> &excludes) {
    QDir fromDir(from);
    if (!fromDir.exists()) {
        that->sendMessage("dialog-error",
                          QString(_("Directory %1 does not exist.")).arg(from));
        return false;
    }

    QDir toDir(to);
    if (!toDir.exists() && !toDir.mkpath(".")) {
        that->sendMessage("dialog-error",
                          QString(_("Failed to create directory %1.")).arg(to));
        return false;
    }
    for (const auto &name :
         fromDir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
        auto fromFile = fromDir.filePath(name);
        auto toFile = toDir.filePath(name);
        if (!checkFileName(name, excludes)) {
            continue;
        }

        QSaveFile to(toFile);
        QFile from(fromFile);
        if (!from.open(QIODevice::ReadOnly)) {
            return false;
        }
        if (!to.open(QIODevice::WriteOnly)) {
            return false;
        }

        that->sendMessage(
            "", QString(_("Copying file %1 to %2")).arg(fromFile, toFile));
        if (cloneFile(from.handle(), to.handle())) {
            to.commit();
        } else {
            to.cancelWriting();
            that->sendMessage("dialog-error",
                              QString(_("Failed to copy file %1 to %2"))
                                  .arg(fromFile, toFile));
            return false;
        }
    }
    for (const auto &name :
         fromDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs)) {
        if (checkFileName(name, excludes)) {
            continue;
        }
        if (!copyDirRecursive(that, fromDir.filePath(name),
                              toDir.filePath(name), excludes)) {
            return false;
        }
    }

    return true;
}

} // namespace

CopyDirectory::CopyDirectory(const QString &from, const QString &to,
                             QObject *parent)
    : CallbackRunner(
          [from, to](CallbackRunner *runner) {
              auto that = static_cast<CopyDirectory *>(runner);
              QList<QRegularExpression> matchers;
              for (const auto &exclude : that->excludes_) {
                  matchers.push_back(QRegularExpression(exclude));
              }
              return copyDirRecursive(that, from, to, matchers);
              ;
          },
          parent) {}

} // namespace fcitx
