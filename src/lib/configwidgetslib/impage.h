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
#ifndef _FCITX_IMPAGE_H_
#define _FCITX_IMPAGE_H_

#include "imconfig.h"
#include <QDBusPendingCallWatcher>
#include <QWidget>
#include <fcitxqtdbustypes.h>
#include <memory>

namespace Ui {
class IMPage;
}

namespace fcitx {
namespace kcm {

class AvailIMModel;
class IMProxyModel;
class CurrentIMModel;
class DBusProvider;

class IMPage : public QWidget {
    Q_OBJECT
public:
    IMPage(DBusProvider *dbus, QWidget *parent);
    ~IMPage();
signals:
    void changed();
public slots:
    void save();
    void load();
    void defaults();

private slots:
    void selectedGroupChanged();

    void availIMSelectionChanged();
    void currentIMCurrentChanged();
    void selectCurrentIM(const QModelIndex &index);
    void doubleClickCurrentIM(const QModelIndex &index);
    void doubleClickAvailIM(const QModelIndex &index);
    void selectDefaultLayout();
    void selectLayout();

    void selectAvailIM(const QModelIndex &index);
    void clickAddIM();
    void clickRemoveIM();
    void moveDownIM();
    void moveUpIM();
    void configureIM();
    void addGroup();
    void deleteGroup();

private:
    void addIM(const QModelIndex &index);
    void removeIM(const QModelIndex &index);
    void checkDefaultLayout();
    void emitChanged();

    std::unique_ptr<Ui::IMPage> ui_;
    DBusProvider *dbus_;
    IMConfig *config_;
};
} // namespace kcm
} // namespace fcitx

#endif // _FCITX_IMPAGE_H_
