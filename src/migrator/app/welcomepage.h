/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_APP_WELCOMEPAGE_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_APP_WELCOMEPAGE_H_

#include "ui_welcomepage.h"

namespace fcitx {

class MainWindow;

class WelcomePage : public QWizardPage, private Ui::WelcomePage {
    Q_OBJECT
public:
    explicit WelcomePage(MainWindow *parent);

    bool isComplete() const override;

private:
    MainWindow *parent_;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_APP_WELCOMEPAGE_H_
