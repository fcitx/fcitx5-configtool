/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _DBUSPROVIDER_H_
#define _DBUSPROVIDER_H_

#include <QObject>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtwatcher.h>

namespace fcitx {
namespace kcm {

class DBusProvider : public QObject {
    Q_OBJECT

public:
    DBusProvider(QObject *parent);
    ~DBusProvider();

    bool available() const { return controller_; }
    FcitxQtControllerProxy *controller() { return controller_; }

signals:
    void availabilityChanged(bool avail);

private slots:
    void fcitxAvailabilityChanged(bool avail);

private:
    FcitxQtWatcher *watcher_;
    FcitxQtControllerProxy *controller_ = nullptr;
};

} // namespace kcm
} // namespace fcitx

#endif // _DBUSPROVIDER_H_
