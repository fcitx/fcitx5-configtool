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
#ifndef _KCM_FCITX_KEYLISTWIDGET_H_
#define _KCM_FCITX_KEYLISTWIDGET_H_

#include <QWidget>
#include <fcitx-utils/key.h>

class QBoxLayout;

namespace fcitx {
namespace kcm {

class KeyListWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyListWidget(QWidget *parent = 0);

    QList<Key> keys() const;
    void setKeys(const QList<Key> &keys);

signals:
    void keyChanged();

private:
    void addKey(Key key = Key());
    bool removeKeyAt(int idx);
    bool showRemoveButton() const;

    QBoxLayout *keysLayout_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_KEYLISTWIDGET_H_
