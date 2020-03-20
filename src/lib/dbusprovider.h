//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; see the file COPYING. If not,
// see <http://www.gnu.org/licenses/>.
//
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
