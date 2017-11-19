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
#include <QEvent>
#include <QScrollBar>
#include <QVBoxLayout>

#include "verticalscrollarea.h"

namespace fcitx {
namespace kcm {

VerticalScrollArea::VerticalScrollArea(QWidget *parent) : QScrollArea(parent) {
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void VerticalScrollArea::setWidget(QWidget *widget) {
    QScrollArea::setWidget(widget);
    widget->installEventFilter(this);
}

bool VerticalScrollArea::eventFilter(QObject *o, QEvent *e) {
    if (o == widget() && e->type() == QEvent::Resize)
        setMinimumWidth(widget()->minimumSizeHint().width() +
                        verticalScrollBar()->width());

    return false;
}

} // namespace kcm
} // namespace fcitx
