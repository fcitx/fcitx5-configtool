/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
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
