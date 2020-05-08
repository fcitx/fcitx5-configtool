/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
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

class DBusProvider;

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
    explicit ConfigWidget(const QString &uri, DBusProvider *module,
                          QWidget *parent = 0);

    static QDialog *configDialog(QWidget *parent, DBusProvider *module,
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
    DBusProvider *dbus_;
    QWidget *mainWidget_;

    bool dontEmitChanged_ = false;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_CONFIGWIDGET_H_
