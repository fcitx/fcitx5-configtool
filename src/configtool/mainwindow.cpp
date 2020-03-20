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
#include "mainwindow.h"
#include "logging.h"
#include <fcitx-utils/i18n.h>
#include <verticalscrollarea.h>

namespace fcitx {
namespace kcm {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dbus_(new DBusProvider(this)),
      errorOverlay_(new ErrorOverlay(dbus_, this)),
      impage_(new IMPage(dbus_, this)),
      addonPage_(new AddonSelector(this, dbus_)),
      configPage_(new ConfigWidget("fcitx://config/global", dbus_, this)) {
    setupUi(this);

    pageWidget->addTab(impage_, _("Input Method"));
    connect(impage_, &IMPage::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "IMPage changed";
        emit changed(true);
    });
    pageWidget->addTab(addonPage_, _("Addon"));
    connect(addonPage_, &AddonSelector::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "AddonSelector changed";
        emit changed(true);
    });
    auto configPageWrapper = new VerticalScrollArea;
    configPageWrapper->setWidget(configPage_);
    pageWidget->addTab(configPageWrapper, _("Global Config"));
    connect(configPage_, &ConfigWidget::changed, this,
            [this]() { emit changed(true); });

    connect(this, &MainWindow::changed, this, &MainWindow::handleChanged);

    connect(buttonBox, &QDialogButtonBox::clicked, this, &MainWindow::clicked);
    load();
}

void MainWindow::handleChanged(bool changed) {
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(changed);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(changed);
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(changed);
}

void MainWindow::load() {
    impage_->load();
    addonPage_->load();
    configPage_->load();
    emit changed(false);
}

void MainWindow::save() {
    impage_->save();
    addonPage_->save();
    configPage_->save();
    emit changed(false);
}

void MainWindow::defaults() {
    configPage_->buttonClicked(QDialogButtonBox::RestoreDefaults);
    emit changed(true);
}

void MainWindow::clicked(QAbstractButton *button) {
    QDialogButtonBox::StandardButton standardButton =
        buttonBox->standardButton(button);
    if (standardButton == QDialogButtonBox::Apply ||
        standardButton == QDialogButtonBox::Ok) {
        save();
    } else if (standardButton == QDialogButtonBox::Close) {
        qApp->quit();
    } else if (standardButton == QDialogButtonBox::Reset) {
        load();
    } else if (standardButton == QDialogButtonBox::RestoreDefaults) {
        defaults();
    }
}

} // namespace kcm
} // namespace fcitx
