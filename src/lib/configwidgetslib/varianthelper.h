/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _KCM_FCITX5_VARIANTHELPER_H_
#define _KCM_FCITX5_VARIANTHELPER_H_

#include <QString>
#include <QVariantMap>

namespace fcitx {
namespace kcm {

QVariant valueFromVariant(const QVariant &value,
                          const QString &path = QString());

QString stringFromVariantMap(const QVariantMap &map, const QString &path);

void valueToVariantMapByPath(QVariantMap &map, const QStringList &path,
                             const QVariant &value, int depth);

void valueToVariantMap(QVariantMap &map, const QString &path,
                       const QVariant &value);

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_VARIANTHELPER_H_
