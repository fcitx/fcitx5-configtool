/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "welcomepage.h"
#include "mainwindow.h"
#include <QDebug>

namespace fcitx {

WelcomePage::WelcomePage(MainWindow *parent)
    : QWizardPage(parent), parent_(parent) {
    setupUi(this);
    fcitxNotRunningMessage->setVisible(!parent_->dbus()->available());

    connect(parent->dbus(), &kcm::DBusProvider::availabilityChanged, this,
            &QWizardPage::completeChanged);
    connect(parent->dbus(), &kcm::DBusProvider::availabilityChanged, this,
            [this](bool avail) { fcitxNotRunningMessage->setVisible(!avail); });
}

bool WelcomePage::isComplete() const { return parent_->dbus()->available(); }
} // namespace fcitx
