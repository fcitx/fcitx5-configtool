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
#ifndef _KCM_FCITX5_CONFIGTOOL_MAINWINDOW_H_
#define _KCM_FCITX5_CONFIGTOOL_MAINWINDOW_H_

#include "addonselector.h"
#include "configwidget.h"
#include "dbusprovider.h"
#include "erroroverlay.h"
#include "impage.h"
#include "ui_mainwindow.h"
#include <QAbstractButton>
#include <QMainWindow>

namespace fcitx {
namespace kcm {

class MainWindow : public QMainWindow, public Ui::MainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void load();
    void save();
    void defaults();
signals:
    void changed(bool state);

private slots:
    void clicked(QAbstractButton *button);

private:
    void handleChanged(bool state);
    DBusProvider *dbus_;
    ErrorOverlay *errorOverlay_;
    IMPage *impage_;
    AddonSelector *addonPage_;
    ConfigWidget *configPage_;
};
} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_CONFIGTOOL_MAINWINDOW_H (2)_
