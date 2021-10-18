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
#include <QMessageBox>
#include <QPushButton>
#include <QSessionManager>
#include <fcitx-utils/i18n.h>

namespace fcitx {
namespace kcm {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dbus_(new DBusProvider(this)),
      errorOverlay_(new ErrorOverlay(dbus_, this)),
      impage_(new IMPage(dbus_, this)),
      addonPage_(new AddonSelector(this, dbus_)),
      configPage_(new ConfigWidget("fcitx://config/global", dbus_, this)) {
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest, this,
            &MainWindow::commitData);
    connect(qApp, &QGuiApplication::saveStateRequest, this,
            [](QSessionManager &manager) {
                manager.setRestartHint(QSessionManager::RestartNever);
            });

    setupUi(this);

    connect(impage_, &IMPage::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "IMPage changed";
        Q_EMIT changed(true);
    });
    connect(addonPage_, &AddonSelector::changed, this, [this]() {
        qCDebug(KCM_FCITX5) << "AddonSelector changed";
        Q_EMIT changed(true);
    });
    auto configPageWrapper = new VerticalScrollArea;
    configPageWrapper->setWidget(configPage_);
    pageWidget->addTab(impage_, _("Input Method"));
    pageWidget->addTab(configPageWrapper, _("Global Options"));
    pageWidget->addTab(addonPage_, _("Addons"));
    connect(configPage_, &ConfigWidget::changed, this,
            [this]() { Q_EMIT changed(true); });

    connect(this, &MainWindow::changed, this, &MainWindow::handleChanged);

    connect(buttonBox, &QDialogButtonBox::clicked, this, &MainWindow::clicked);
    load();

    buttonBox->button(QDialogButtonBox::Apply)->setText(_("&Apply"));
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&OK"));
    buttonBox->button(QDialogButtonBox::Close)->setText(_("&Close"));
    buttonBox->button(QDialogButtonBox::Reset)->setText(_("&Reset"));
    buttonBox->button(QDialogButtonBox::RestoreDefaults)
        ->setText(_("Restore &Defaults"));
}

void MainWindow::handleChanged(bool changed) {
    changed_ = changed;
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(changed);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(changed);
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(changed);
}

void MainWindow::load() {
    impage_->load();
    addonPage_->load();
    configPage_->load();
    Q_EMIT changed(false);
}

void MainWindow::save() {
    impage_->save();
    addonPage_->save();
    configPage_->save();
    Q_EMIT changed(false);
}

void MainWindow::defaults() {
    configPage_->buttonClicked(QDialogButtonBox::RestoreDefaults);
    Q_EMIT changed(true);
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
    if (standardButton == QDialogButtonBox::Apply) {
        save();
    } else if (standardButton == QDialogButtonBox::Ok) {
        save();
        qApp->quit();
    } else if (standardButton == QDialogButtonBox::Close) {
        qApp->quit();
    } else if (standardButton == QDialogButtonBox::Reset) {
        load();
    } else if (standardButton == QDialogButtonBox::RestoreDefaults) {
        defaults();
    }
}

void MainWindow::commitData(QSessionManager &manager) {
    manager.setRestartHint(QSessionManager::RestartNever);
    if (!manager.allowsInteraction() || !changed_) {
        return;
    }
    int ret = QMessageBox::warning(
        this, _("Apply configuration changes"),
        _("Do you want to save the changes or discard them?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:
        save();
        manager.release();
        break;
    case QMessageBox::Discard:
        break;
    case QMessageBox::Cancel:
    default:
        manager.cancel();
    }
}

} // namespace kcm
} // namespace fcitx
