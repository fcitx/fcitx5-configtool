/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "progresspage.h"
#include "mainwindow.h"
#include <QAbstractButton>
#include <fcitx-utils/i18n.h>

namespace fcitx {

ProgressPage::ProgressPage(MainWindow *parent)
    : QWizardPage(parent), parent_(parent) {
    setupUi(this);
    setFinalPage(true);
}

bool ProgressPage::isComplete() const { return done_; }

void ProgressPage::appendMessage(const QString &icon, const QString &message) {
    QListWidgetItem *item =
        new QListWidgetItem(QIcon::fromTheme(icon), message, listWidget);
    listWidget->addItem(item);
    listWidget->scrollToItem(item);
}

void ProgressPage::initializePage() {
    if (pipeline_) {
        delete pipeline_;
    }
    listWidget->clear();
    pipeline_ = parent_->createPipeline();
    done_ = false;

    connect(pipeline_, &Pipeline::message, this, &ProgressPage::appendMessage);

    connect(pipeline_, &Pipeline::finished, this, [this](bool success) {
        if (success) {
            done_ = true;
            parent_->button(QWizard::CancelButton)->setEnabled(false);
            appendMessage("dialog-positive",
                          _("All migration tasks are completed successfully."));
            Q_EMIT completeChanged();
        } else {
            appendMessage(
                "dialog-error",
                _("One of the task is failed. Terminating the migration."));
        }
    });
    pipeline_->start();
}

} // namespace fcitx
