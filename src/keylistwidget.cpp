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

#include "keylistwidget.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QToolButton>
#include <QVBoxLayout>
#include <fcitxqtkeysequencewidget.h>

fcitx::kcm::KeyListWidget::KeyListWidget(QWidget *parent) : QWidget(parent) {
    auto layout = new QHBoxLayout;
    layout->setMargin(0);
    keysLayout_ = new QVBoxLayout;
    keysLayout_->setMargin(0);
    auto subLayout = new QVBoxLayout;

    addButton_ = new QToolButton;
    addButton_->setAutoRaise(true);
    addButton_->setIcon(QIcon::fromTheme("list-add"));
    addButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(addButton_, &QToolButton::clicked, this, [this]() {
        addKey(Key());
        emit keyChanged();
    });

    layout->addLayout(keysLayout_);
    subLayout->addWidget(addButton_, 0, Qt::AlignTop);
    // subLayout->addStretch(1);
    layout->addLayout(subLayout);

    setLayout(layout);

    // Add an empty one.
    addKey();
}

void fcitx::kcm::KeyListWidget::addKey(fcitx::Key key) {
    auto keyWidget = new FcitxQtKeySequenceWidget;
    keyWidget->setClearButtonShown(false);
    keyWidget->setKeySequence({key});
    auto widget = new QWidget;
    auto layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(keyWidget);
    auto removeButton = new QToolButton;
    removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    removeButton->setIcon(QIcon::fromTheme("dialog-close"));
    removeButton->setVisible(showRemoveButton());
    layout->addWidget(removeButton);
    widget->setLayout(layout);
    connect(removeButton, &QPushButton::clicked, widget, [widget, this]() {
        auto idx = keysLayout_->indexOf(widget);
        if (removeKeyAt(idx)) {
            emit keyChanged();
        }
    });
    connect(keyWidget, &FcitxQtKeySequenceWidget::keySequenceChanged, this,
            &KeyListWidget::keyChanged);
    connect(this, &KeyListWidget::keyChanged, removeButton,
            [this, removeButton]() {
                removeButton->setVisible(showRemoveButton());
            });
    keysLayout_->addWidget(widget);
}

void fcitx::kcm::KeyListWidget::setKeys(const QList<fcitx::Key> &keys) {
    while (keysLayout_->count() > 1) {
        removeKeyAt(0);
    }
    removeKeyAt(0);

    bool first = true;
    for (auto key : keys) {
        if (first) {
            first = false;
            keysLayout_->itemAt(0)
                ->widget()
                ->findChild<FcitxQtKeySequenceWidget *>()
                ->setKeySequence({key});
        } else {
            addKey(key);
        }
    }
    emit keyChanged();
}

QList<fcitx::Key> fcitx::kcm::KeyListWidget::keys() const {
    QList<fcitx::Key> result;
    for (int i = 0; i < keysLayout_->count(); i++) {
        if (auto keyWidget = keysLayout_->itemAt(i)
                                 ->widget()
                                 ->findChild<FcitxQtKeySequenceWidget *>()) {
            if (keyWidget->keySequence().isEmpty()) {
                continue;
            }
            auto &key = keyWidget->keySequence()[0];
            if (key.isValid() && !result.contains(key)) {
                result << keyWidget->keySequence()[0];
            }
        }
    }
    return result;
}

bool fcitx::kcm::KeyListWidget::removeKeyAt(int idx) {
    if (idx < 0 || idx > keysLayout_->count()) {
        return false;
    }
    auto widget = keysLayout_->itemAt(idx)->widget();
    if (keysLayout_->count() == 1) {
        keysLayout_->itemAt(0)
            ->widget()
            ->findChild<FcitxQtKeySequenceWidget *>()
            ->setKeySequence(QList<Key>());
    } else {
        keysLayout_->removeWidget(widget);
        delete widget;
    }
    return true;
}

bool fcitx::kcm::KeyListWidget::showRemoveButton() const {
    return keysLayout_->count() > 1 ||
           (keysLayout_->count() == 1 &&
            keysLayout_->itemAt(0)
                ->widget()
                ->findChild<FcitxQtKeySequenceWidget *>()
                ->keySequence()
                .size());
}

void fcitx::kcm::KeyListWidget::resizeEvent(QResizeEvent *event) {
    if (keysLayout_->count() > 0) {
        addButton_->setMinimumHeight(
            keysLayout_->itemAt(0)
                ->widget()
                ->findChild<FcitxQtKeySequenceWidget *>()
                ->height());
        addButton_->setMaximumHeight(addButton_->minimumHeight());
    }

    QWidget::resizeEvent(event);
}
