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

#include "iso639.h"
#include "config.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QMap<QString, QString> readAlpha3ToNameMap(const char *name, const char *base) {
    QMap<QString, QString> map;
    QFile file(name);
    file.open(QIODevice::ReadOnly);
    auto data = file.readAll();
    QJsonParseError error;
    auto document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    auto object = document.object();
    auto iso = object.value(base);
    if (!iso.isArray()) {
        return {};
    }
    const auto array = iso.toArray();
    for (const auto &item : array) {
        if (!item.isObject()) {
            continue;
        }
        auto alpha3 = item.toObject().value("alpha_3").toString();
        auto bibliographic = item.toObject().value("bibliographic").toString();
        auto name = item.toObject().value("name").toString();
        if (alpha3.isEmpty() || name.isEmpty()) {
            continue;
        }
        map.insert(alpha3, name);
        if (!bibliographic.isEmpty()) {
            map.insert(bibliographic, name);
        }
    }
    return map;
}

fcitx::kcm::Iso639::Iso639() {
    iso639_2data_ = readAlpha3ToNameMap(ISOCODES_ISO639_2_JSON, "639-2");
    iso639_3data_ = readAlpha3ToNameMap(ISOCODES_ISO639_3_JSON, "639-3");
    iso639_5data_ = readAlpha3ToNameMap(ISOCODES_ISO639_5_JSON, "639-5");
}
