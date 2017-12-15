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
#include "varianthelper.h"
#include <QDBusArgument>

namespace fcitx {
namespace kcm {

QVariantMap toMap(const QVariant &variant) {
    QVariantMap map;
    if (variant.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(variant);
        argument >> map;
    }
    if (variant.canConvert<QVariantMap>()) {
        map = variant.toMap();
    }
    return map;
}

QString valueFromVariantMapByPath(const QVariantMap &map,
                                  const QStringList &path, int depth) {
    auto iter = map.find(path[depth]);
    if (iter == map.end()) {
        return QString();
    }
    if (depth + 1 == path.size()) {
        if (iter->canConvert<QString>()) {
            return iter->toString();
        }
    } else {
        QVariantMap map = toMap(*iter);

        if (!map.isEmpty()) {
            return valueFromVariantMapByPath(map, path, depth + 1);
        }
    }
    return QString();
}

QVariant valueFromVariantHelper(const QVariant &value,
                                const QStringList &pathList, int depth) {
    if (depth == pathList.size()) {
        return value;
    }
    auto map = toMap(value);
    // Make it finishes faster.
    if (map.isEmpty() || !map.contains(pathList[depth])) {
        return {};
    }
    return valueFromVariantHelper(map[pathList[depth]], pathList, depth + 1);
}

QVariant valueFromVariant(const QVariant &value, const QString &path) {
    auto pathList = path.split("/");
    return valueFromVariantHelper(toMap(value), pathList, 0);
}

QString valueFromVariantMap(const QVariantMap &map, const QString &path) {
    auto pathList = path.split("/");
    if (pathList.empty()) {
        return QString();
    }
    return valueFromVariantMapByPath(map, pathList, 0);
}

void valueToVariantMapByPath(QVariantMap &map, const QStringList &path,
                             const QVariant &value, int depth) {
    if (depth + 1 == path.size()) {
        map[path[depth]] = value;
    } else {
        auto iter = map.find(path[depth]);
        if (iter == map.end()) {
            iter = map.insert(path[depth], QVariantMap());
        }

        if (iter->type() != QVariant::Map) {
            auto oldValue = *iter;
            *iter = QVariantMap({{"", oldValue}});
        }

        auto &nextMap = *static_cast<QVariantMap *>(iter->data());
        valueToVariantMapByPath(nextMap, path, value, depth + 1);
    }
}

void valueToVariantMap(QVariantMap &map, const QString &path,
                       const QVariant &value) {
    auto pathList = path.split("/");
    if (pathList.empty()) {
        return;
    }
    valueToVariantMapByPath(map, pathList, value, 0);
}

} // namespace kcm
} // namespace fcitx
