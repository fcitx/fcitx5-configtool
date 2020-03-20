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

#include "erroroverlay.h"
#include "dbusprovider.h"
#include "ui_erroroverlay.h"
#include <QIcon>

namespace fcitx {
namespace kcm {

ErrorOverlay::ErrorOverlay(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), ui_(std::make_unique<Ui::ErrorOverlay>()),
      baseWidget_(parent) {
    ui_->setupUi(this);
    setVisible(false);

    baseWidget_->installEventFilter(this);
    ui_->pixmapLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(64));

    connect(baseWidget_, &QObject::destroyed, this, &QObject::deleteLater);
    connect(dbus, &DBusProvider::availabilityChanged, this,
            &ErrorOverlay::availabilityChanged);
    availabilityChanged(dbus->available());
}

ErrorOverlay::~ErrorOverlay() {}

void ErrorOverlay::availabilityChanged(bool avail) {
    const bool newEnabled = !avail;
    if (enabled_ != newEnabled) {
        enabled_ = newEnabled;
        setVisible(newEnabled);
        if (newEnabled) {
            reposition();
        }
    }
}

void ErrorOverlay::reposition() {
    if (!baseWidget_) {
        return;
    }

    // follow base widget visibility
    // needed eg. in tab widgets
    if (!baseWidget_->isVisible()) {
        hide();
        return;
    }

    show();

    // follow position changes
    const QPoint topLevelPos = baseWidget_->mapTo(window(), QPoint(0, 0));
    const QPoint parentPos = parentWidget()->mapFrom(window(), topLevelPos);
    move(parentPos);

    // follow size changes
    // TODO: hide/scale icon if we don't have enough space
    resize(baseWidget_->size());
    raise();
}

bool ErrorOverlay::eventFilter(QObject *object, QEvent *event) {
    if (enabled_ && object == baseWidget_ &&
        (event->type() == QEvent::Move || event->type() == QEvent::Resize ||
         event->type() == QEvent::Show || event->type() == QEvent::Hide ||
         event->type() == QEvent::ParentChange)) {
        reposition();
    }
    return QWidget::eventFilter(object, event);
}

} // namespace kcm
} // namespace fcitx
