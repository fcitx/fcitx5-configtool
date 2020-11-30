/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "mainwindow.h"
#include "logging.h"
#include "verticalscrollarea.h"
#include <QKeyEvent>
#include <QPushButton>
#include <fcitx-utils/i18n.h>

namespace fcitx {
namespace kcm {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dbus_(new DBusProvider(this)),
      errorOverlay_(new ErrorOverlay(dbus_, this)),
      impage_(new IMPage(dbus_, this)),
      addonPage_(new AddonSelector(this, dbus_)),
      configPage_(new ConfigWidget("fcitx://config/global", dbus_, this)) {
    setupUi(this);

    connect(impage_, &IMPage::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "IMPage changed";
        emit changed(true);
    });
    connect(addonPage_, &AddonSelector::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "AddonSelector changed";
        emit changed(true);
    });
    auto configPageWrapper = new VerticalScrollArea;
    configPageWrapper->setWidget(configPage_);
    pageWidget->addTab(impage_, _("Input Method"));
    pageWidget->addTab(configPageWrapper, _("Global Options"));
    pageWidget->addTab(addonPage_, _("Addons"));
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

void MainWindow::keyPressEvent(QKeyEvent *event) {
    QMainWindow::keyPressEvent(event);
    if (!event->isAccepted() && event->matches(QKeySequence::Cancel)) {
        qApp->quit();
    }
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
