/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
