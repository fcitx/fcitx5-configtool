/*
 * Copyright (C) 2012~2017 by CSSlayer
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
#ifndef _KCM_FCITX_FONTBUTTON_H_
#define _KCM_FCITX_FONTBUTTON_H_

#include "ui_fontbutton.h"

namespace fcitx {
namespace kcm {

class FontButton : public QWidget, public Ui::FontButton {
    Q_OBJECT
public:
    explicit FontButton(QWidget *parent = 0);
    virtual ~FontButton();
    const QFont &font();
    QString fontName();

public slots:
    void setFont(const QFont &font);
signals:
    void fontChanged(const QFont &font);
private slots:
    void selectFont();

private:
    QFont font_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_FONTBUTTON_H_
