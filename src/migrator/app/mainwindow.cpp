/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "mainwindow.h"
#include "progresspage.h"
#include "taskpage.h"
#include "welcomepage.h"
#include <fcitx-utils/i18n.h>

namespace fcitx {

MainWindow::MainWindow(QWidget *parent)
    : QWizard(parent), dbus_(this), welcomePage_(new WelcomePage(this)),
      taskPage_(new TaskPage(this)), progressPage_(new ProgressPage(this)) {
    addPage(welcomePage_);
    addPage(taskPage_);
    addPage(progressPage_);
    setMinimumSize(QSize(700, 480));
    setWindowIcon(QIcon::fromTheme("fcitx"));
    setWindowTitle(_("Migration Tool"));
}

Pipeline *MainWindow::createPipeline() { return taskPage_->createPipeline(); }

} // namespace fcitx
