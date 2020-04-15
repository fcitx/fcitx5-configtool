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
#include "layoutmodel.h"

QVariant fcitx::kcm::LanguageFilterModel::layoutInfo(int row) const {
    auto idx = index(row, 0);
    if (idx.isValid()) {
        return idx.data(LayoutInfoRole);
    }
    return QVariant();
}

fcitx::kcm::LanguageModel::LanguageModel(QObject *parent)
    : QStandardItemModel(parent) {
    setItemRoleNames({{Qt::DisplayRole, "name"}, {Qt::UserRole, "language"}});
}

QString fcitx::kcm::LanguageModel::language(int row) const {
    auto idx = index(row, 0);
    if (idx.isValid()) {
        return idx.data(Qt::UserRole).toString();
    }
    return QString();
}

void fcitx::kcm::LanguageModel::append(const QString &name,
                                       const QString &language) {
    QStandardItem *item = new QStandardItem(name);
    item->setData(language, Qt::UserRole);
    appendRow(item);
}
