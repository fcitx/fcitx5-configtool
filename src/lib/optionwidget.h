//
// Copyright (C) 2017~2017 by CSSlayer
// wengxt@gmail.com
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
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
#ifndef _KCM_FCITX_OPTIONWIDGET_H_
#define _KCM_FCITX_OPTIONWIDGET_H_

#include <QWidget>
#include <fcitxqtdbustypes.h>

class QFormLayout;

namespace fcitx {
namespace kcm {

class OptionWidget : public QWidget {
    Q_OBJECT
public:
    OptionWidget(const QString &path, QWidget *parent)
        : QWidget(parent), path_(path) {}

    static OptionWidget *addWidget(QFormLayout *layout,
                                   const fcitx::FcitxQtConfigOption &option,
                                   const QString &path, QWidget *parent);

    static bool execOptionDialog(const fcitx::FcitxQtConfigOption &option,
                                 QVariant &result);

    virtual void readValueFrom(const QVariantMap &map) = 0;
    virtual void writeValueTo(QVariantMap &map) = 0;
    virtual void restoreToDefault() = 0;
    virtual bool isValid() const { return true; }

    static QString prettify(const FcitxQtConfigOption &option,
                            const QVariant &value);

    const QString &path() const { return path_; }
signals:
    void valueChanged();

private:
    QString path_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_OPTIONWIDGET_H_
