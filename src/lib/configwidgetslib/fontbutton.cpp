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

#include <KFontChooser>
#include <QDialog>
#include <QDialogButtonBox>

#include "font.h"
#include "fontbutton.h"

namespace fcitx {
namespace kcm {

FontButton::FontButton(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    connect(fontSelectButton, &QPushButton::clicked, this,
            &FontButton::selectFont);
}

FontButton::~FontButton() {}

const QFont &FontButton::font() { return font_; }

QString FontButton::fontName() { return fontPreviewLabel->text(); }

void FontButton::setFont(const QFont &font) {
    font_ = font;
    if (font.family() != font_.family()) {
        emit fontChanged(font_);
    }
    fontPreviewLabel->setText(fontToString(font_));
    fontPreviewLabel->setFont(font_);
}

void FontButton::selectFont() {
    QDialog dialog(NULL);
    KFontChooser *chooser = new KFontChooser(&dialog);
    chooser->setFont(font_);
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialog.setLayout(dialogLayout);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                             QDialogButtonBox::RestoreDefaults);
    dialogLayout->addWidget(chooser);
    dialogLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        setFont(chooser->font());
    }
}

} // namespace kcm
} // namespace fcitx
