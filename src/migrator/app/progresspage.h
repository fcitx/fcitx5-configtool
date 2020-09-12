/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_APP_PROGRESSPAGE_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_APP_PROGRESSPAGE_H_

#include "pipeline.h"
#include "ui_progresspage.h"
#include <QWizardPage>

namespace fcitx {

class MainWindow;

class ProgressPage : public QWizardPage, public Ui::ProgressPage {
    Q_OBJECT
public:
    explicit ProgressPage(MainWindow *parent);

    bool isComplete() const override;
    void initializePage() override;

    void appendMessage(const QString &icon, const QString &message);

private:
    MainWindow *parent_;
    Pipeline *pipeline_ = nullptr;
    bool done_ = false;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_APP_PROGRESSPAGE_H_
