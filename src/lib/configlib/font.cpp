//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "font.h"
#include <QMap>

QFont fcitx::kcm::parseFont(const QString &string) {
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

QString fcitx::kcm::fontToString(const QFont &font) {
    QString style;
    QStringList styles;
    switch (font.style()) {
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
    switch (font.weight()) {
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
    return QString("%1%2%3 %4")
        .arg(font.family(), (!style.isEmpty() ? " " : ""), style,
             QString::number(font.pointSize()));
}
