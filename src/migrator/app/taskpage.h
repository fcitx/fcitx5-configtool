/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_MIGRATOR_APP_TASKPAGE_H_
#define _FCITX5_CONFIGTOOL_MIGRATOR_APP_TASKPAGE_H_

#include "pipeline.h"
#include "ui_taskpage.h"
#include <fcitxqtcontrollerproxy.h>

namespace fcitx {

class MainWindow;
class TaskModel;

class TaskPage : public QWizardPage, private Ui::TaskPage {
    Q_OBJECT
public:
    explicit TaskPage(MainWindow *parent);

    bool isComplete() const override;

    void initializePage() override;
    bool validatePage() override;

    Pipeline *createPipeline();

private slots:
    void availabilityChanged(bool avail);

private:
    MainWindow *parent_;
    FcitxQtControllerProxy *proxy_;
    TaskModel *model_;
};

} // namespace fcitx

#endif // _FCITX5_CONFIGTOOL_MIGRATOR_APP_TASKPAGE_H_
