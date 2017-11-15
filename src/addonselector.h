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

// Qt
#include <QWidget>

#include "ui_addonselector.h"

class QDBusPendingCallWatcher;

namespace fcitx {
namespace kcm {

class Module;
class AddonModel;
class ProxyModel;
class AddonDelegate;

class AddonSelector : public QWidget, private Ui::AddonSelector {
    Q_OBJECT

public:
    AddonSelector(Module *parent);
    virtual ~AddonSelector();
    void load();
    void save();

    QString searchText() const { return lineEdit->text(); }

    bool showAdvanced() const { return advancedCheckbox->isChecked(); }

signals:
    void changed();
    void configCommitted(const QByteArray &componentName);

private slots:
    void availabilityChanged();
    void fetchAddonFinished(QDBusPendingCallWatcher *);

private:
    Module *module_;
    KCategoryDrawer *categoryDrawer_;
    ProxyModel *proxyModel_;
    AddonModel *addonModel_;
    AddonDelegate *delegate_;
};
} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_ADDONSELECTOR_H_
