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
#ifndef _KCM_FCITX5_VARIANTHELPER_H_
#define _KCM_FCITX5_VARIANTHELPER_H_

#include <QString>
#include <QVariantMap>

namespace fcitx {
namespace kcm {

QVariant valueFromVariant(const QVariant &value,
                          const QString &path = QString());

QString valueFromVariantMapByPath(const QVariantMap &map,
                                  const QStringList &path, int depth);

QString valueFromVariantMap(const QVariantMap &map, const QString &path);

void valueToVariantMapByPath(QVariantMap &map, const QStringList &path,
                             const QVariant &value, int depth);

void valueToVariantMap(QVariantMap &map, const QString &path,
                       const QVariant &value);

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_VARIANTHELPER_H_
