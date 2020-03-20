/*
 * Copyright (C) 2017~2017 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#ifndef _FCITX_ERROROVERLAY_H_
#define _FCITX_ERROROVERLAY_H_

#include <QPointer>
#include <QWidget>
#include <memory>

namespace Ui {
class ErrorOverlay;
}

namespace fcitx {
namespace kcm {

class DBusProvider;

class ErrorOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ErrorOverlay(DBusProvider *dbus, QWidget *parent);
    ~ErrorOverlay();

    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void availabilityChanged(bool avail);

private:
    void reposition();
    std::unique_ptr<Ui::ErrorOverlay> ui_;
    DBusProvider *dbus;
    QPointer<QWidget> baseWidget_;
    bool enabled_ = false;
};

} // namespace kcm
} // namespace fcitx

#endif // _FCITX_ERROROVERLAY_H_
