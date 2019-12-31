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
#include "module.h"

namespace fcitx {
namespace kcm {

ErrorOverlay::ErrorOverlay(Module *module)
    : QWidget(module), baseWidget_(module) {
    setupUi(this);
    setVisible(false);

    baseWidget_->installEventFilter(this);
    pixmapLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(64));

    connect(baseWidget_, &QObject::destroyed, this, &QObject::deleteLater);
    connect(module, &Module::availabilityChanged, this,
            &ErrorOverlay::availabilityChanged);
    availabilityChanged(module->available());
}

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
