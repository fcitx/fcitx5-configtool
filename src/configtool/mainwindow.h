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
Q_SIGNALS:
    void changed(bool state);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private Q_SLOTS:
    void clicked(QAbstractButton *button);
    void commitData(QSessionManager &manager);

private:
    void handleChanged(bool state);
    bool changed_ = false;
    DBusProvider *dbus_;
    ErrorOverlay *errorOverlay_;
    IMPage *impage_;
    AddonSelector *addonPage_;
    ConfigWidget *configPage_;
};
} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_CONFIGTOOL_MAINWINDOW_H (2)_
