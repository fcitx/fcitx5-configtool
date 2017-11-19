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
#ifndef _KCM_FCITX_CONFIGWIDGET_H_
#define _KCM_FCITX_CONFIGWIDGET_H_

#include <KColorButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>
#include <fcitxqtdbustypes.h>

class QDBusPendingCallWatcher;
class QFormLayout;

namespace fcitx {
namespace kcm {

class Module;

class ColorButton : public KColorButton {
    Q_OBJECT
public:
    explicit ColorButton(QWidget *parent = 0) : KColorButton(parent) {}
public slots:
    void setColor(const QColor &color) { KColorButton::setColor(color); }
};

class ConfigWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConfigWidget(const QString &uri, Module *module,
                          QWidget *parent = 0);

    static QDialog *configDialog(QWidget *parent, Module *module,
                                 const QString &uri,
                                 const QString &title = QString());
signals:
    void changed();

public slots:
    void load();
    void save();
    void buttonClicked(QDialogButtonBox::StandardButton);

    void requestConfig(bool sync = false);
private slots:
    void requestConfigFinished(QDBusPendingCallWatcher *watcher);
    void doChanged();

private:
    void setupWidget(QWidget *widget, const QString &type, const QString &path);
    void addOptionWidget(QFormLayout *layout, const FcitxQtConfigOption &option,
                         const QString &path);

    bool initialized_ = false;
    QString uri_;
    QMap<QString, FcitxQtConfigOptionList> desc_;
    QString mainType_;
    Module *parent_;
    QWidget *mainWidget_;

    bool dontEmitChanged_ = false;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_CONFIGWIDGET_H_
