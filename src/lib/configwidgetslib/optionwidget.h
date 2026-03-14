/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _KCM_FCITX_OPTIONWIDGET_H_
#define _KCM_FCITX_OPTIONWIDGET_H_

#include <QString>
#include <QVariantMap>
#include <QWidget>
#include <QtContainerFwd>
#include <fcitxqtdbustypes.h>
#include <utility>

class QFormLayout;

namespace fcitx::kcm {

class OptionWidget : public QWidget {
    Q_OBJECT
public:
    OptionWidget(QString path, QWidget *parent)
        : QWidget(parent), path_(std::move(path)) {}

    static OptionWidget *addWidget(QFormLayout *layout,
                                   const fcitx::FcitxQtConfigOption &option,
                                   const QString &path, QWidget *parent,
                                   QWidget *labelWidget = nullptr);

    static bool execOptionDialog(QWidget *parent,
                                 const fcitx::FcitxQtConfigOption &option,
                                 QVariant &result);

    virtual void readValueFrom(const QVariantMap &map) = 0;
    virtual void writeValueTo(QVariantMap &map) = 0;
    virtual void restoreToDefault() = 0;
    virtual bool isValid() const { return true; }
    bool skipConfig() const { return skipConfig_; }
    void setSkipConfig(bool skip) { skipConfig_ = skip; }

    QString prettify(const FcitxQtConfigOption &option, const QVariant &value);

    const QString &path() const { return path_; }
Q_SIGNALS:
    void valueChanged();

private:
    QString path_;
    bool skipConfig_ = false;
};

} // namespace fcitx::kcm

#endif // _KCM_FCITX_OPTIONWIDGET_H_
