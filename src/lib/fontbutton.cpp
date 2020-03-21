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
    int size = 9; // Default size.
    if (!list.empty()) {
        bool ok = false;
        auto fontSize = list.back().toInt(&ok);
        if (ok) {
            if (fontSize > 0) {
                size = fontSize;
            }
            list.pop_back();
        }
    }

    QFont::Style style = QFont::StyleNormal;
    int weight = QFont::Normal;
    const QMap<QString, int> strToWeight = {
        {"Thin", QFont::Thin},
        {"Ultra-Light", QFont::Thin},
        {"Extra-Light", QFont::ExtraLight},
        {"Light", QFont::Light},
        {"Semi-Light", QFont::Light},
        {"Demi-Light", QFont::Light},
        {"Book", QFont::Light},
        {"Regular", QFont::Normal},
        {"Medium", QFont::Medium},
        {"Semi-Bold", QFont::Medium},
        {"Demi-Bold", QFont::DemiBold},
        {"Bold", QFont::Bold},
        {"Ultra-Bold", QFont::Bold},
        {"Extra-Bold", QFont::ExtraBold},
        {"Black", QFont::Black},
        {"Ultra-Black", QFont::Black},
        {"Extra-Black", QFont::Black},
    };
    const QMap<QString, QFont::Style> strToStyle = {
        {"Italic", QFont::StyleItalic}, {"Oblique", QFont::StyleOblique}};
    while (!list.empty()) {
        if (strToWeight.contains(list.back())) {
            weight = strToWeight.value(list.back(), QFont::Normal);
            list.pop_back();
        } else if (strToStyle.contains(list.back())) {
            style = strToStyle.value(list.back(), QFont::StyleNormal);
            list.pop_back();
        } else {
            break;
        }
    }
    QString family = list.join(" ");
    QFont font;
    font.setFamily(family);
    font.setWeight(weight);
    font.setStyle(style);
    font.setPointSize(size);
    return font;
}

void FontButton::setFont(const QFont &font) {
    font_ = font;
    if (font.family() != font_.family()) {
        emit fontChanged(font_);
    }
    QString style;
    QStringList styles;
    switch (font_.style()) {
    case QFont::StyleItalic:
        styles << "Italic";
        break;
    case QFont::StyleOblique:
        styles << "Oblique";
        break;
    default:
        break;
    }
#define CASE_WEIGHT(WEIGHT, WEIGHT_STR)                                        \
    case QFont::WEIGHT:                                                        \
        styles << WEIGHT_STR;                                                  \
        break;
    // Use the string accepted by pango.
    switch (font_.weight()) {
        CASE_WEIGHT(Thin, "Thin")
        CASE_WEIGHT(ExtraLight, "Extra-Light")
        CASE_WEIGHT(Light, "Light")
        CASE_WEIGHT(Medium, "Medium")
        CASE_WEIGHT(DemiBold, "Demi-Bold")
        CASE_WEIGHT(Bold, "Bold")
        CASE_WEIGHT(ExtraBold, "Extra-Bold")
        CASE_WEIGHT(Black, "Black")
    default:
        break;
    }
    style = styles.join(" ");
    fontPreviewLabel->setText(QString("%1%2%3 %4")
                                  .arg(font_.family(),
                                       (!style.isEmpty() ? " " : ""), style,
                                       QString::number(font_.pointSize())));
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
