/*
 * Copyright (C) 2017~2017 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#ifndef _KCM_FCITX_ADDONSELECTOR_H_
#define _KCM_FCITX_ADDONSELECTOR_H_

#include <QWidget>
#include <memory>

class QDBusPendingCallWatcher;

namespace Ui {
class AddonSelector;
}

namespace fcitx {
namespace kcm {

class AddonModel;
class AddonProxyModel;
class AddonDelegate;
class DBusProvider;

class AddonSelector : public QWidget {
    Q_OBJECT

public:
    AddonSelector(QWidget *parent, DBusProvider *dbus);
    virtual ~AddonSelector();
    void load();
    void save();

    QString searchText() const;
    auto dbus() const { return dbus_; }

    bool showAdvanced() const;

signals:
    void changed();
    void configCommitted(const QByteArray &componentName);

private slots:
    void fetchAddonFinished(QDBusPendingCallWatcher *);
    void availabilityChanged();

private:
    DBusProvider *dbus_;
    AddonModel *addonModel_;
    AddonProxyModel *proxyModel_;
    AddonDelegate *delegate_;
    std::unique_ptr<Ui::AddonSelector> ui_;
};
} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_ADDONSELECTOR_H_
