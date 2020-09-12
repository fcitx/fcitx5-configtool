/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_APP_MAINWINDOW_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_APP_MAINWINDOW_H_

#include "dbusprovider.h"
#include "migratorfactory.h"
#include "pipeline.h"
#include <QWizard>
#include <fcitxqtwatcher.h>

namespace fcitx {

class WelcomePage;
class TaskPage;
class ProgressPage;

class MainWindow : public QWizard {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    auto *dbus() { return &dbus_; }
    auto *factory() const { return &factory_; }
    Pipeline *createPipeline();

private:
    kcm::DBusProvider dbus_;
    MigratorFactory factory_;
    WelcomePage *welcomePage_;
    TaskPage *taskPage_;
    ProgressPage *progressPage_;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_APP_MAINWINDOW_H_
