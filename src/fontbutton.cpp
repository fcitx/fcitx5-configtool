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
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>

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

QFont FontButton::parseFont(const QString &string) {
    QStringList list = string.split(" ", QString::SkipEmptyParts);
    bool bold = false;
    bool italic = false;
    while (!list.empty()) {
        if (list.last() == "Bold") {
            bold = true;
            list.pop_back();
        } else if (list.last() == "Italic") {
            italic = true;
            list.pop_back();
        } else
            break;
    }
    QString family = list.join(" ");
    QFont font;
    font.setFamily(family);
    font.setBold(bold);
    font.setItalic(italic);
    return font;
}

void FontButton::setFont(const QFont &font) {
    font_ = font;
    QString style;
    if (!font.styleName().isEmpty()) {
        style = font.styleName();
    } else {
        QStringList styles;
        if (font.bold())
            styles << "Bold";
        if (font.italic())
            styles << "Italic";
        style = styles.join(" ");
    }
    fontPreviewLabel->setText(QString("%1 %2").arg(font_.family(), style));
    fontPreviewLabel->setFont(font_);
    if (font.family() != font_.family()) {
        emit fontChanged(font_);
    }
}

void FontButton::selectFont() {
    QDialog dialog(NULL);
    KFontChooser *chooser = new KFontChooser(&dialog);
    chooser->enableColumn(KFontChooser::SizeList, false);
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
