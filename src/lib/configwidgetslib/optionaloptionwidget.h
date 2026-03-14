/*
 * SPDX-FileCopyrightText: 2025~2025 The fcitx5-configtool authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _KCM_FCITX5_OPTIONIONALOPTIONWIDGET_H_
#define _KCM_FCITX5_OPTIONIONALOPTIONWIDGET_H_

#include "optionwidget.h"
#include <QCheckBox>
#include <QString>
#include <QWidget>
#include <QtContainerFwd>
#include <fcitxqtdbustypes.h>

namespace fcitx::kcm {

class OptionalOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    OptionalOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                         QWidget *parent);

    void readValueFrom(const QVariantMap &map) override;
    void writeValueTo(QVariantMap &map) override;
    void restoreToDefault() override;

    const auto &subOption() { return subOption_; }

private:
    void updateValueWidget();
    QCheckBox *valueCheckBox_;
    OptionWidget *subWidget_;
    FcitxQtConfigOption subOption_;
    QVariantMap defaultValue_;
};

} // namespace fcitx::kcm

#endif // _KCM_FCITX5_OPTIONIONALOPTIONWIDGET_H_
