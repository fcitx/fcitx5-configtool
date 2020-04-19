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
#include "addonmodel.h"
#include <QCollator>
#include <fcitx-utils/i18n.h>
#include <fcitx/addoninfo.h>

namespace fcitx {
namespace kcm {
namespace {

QString categoryName(int category) {
    if (category >= 5 || category < 0) {
        return QString();
    }

    const char *str[] = {N_("Input Method"), N_("Frontend"), N_("Loader"),
                         N_("Module"), N_("UI")};

    return _(str[category]);
}

} // namespace

AddonModel::AddonModel(QObject *parent) : CategorizedItemModel(parent) {}

QVariant AddonModel::dataForCategory(const QModelIndex &index, int role) const {
    switch (role) {

    case Qt::DisplayRole:
        return categoryName(addonEntryList_[index.row()].first);

    case CategoryRole:
        return addonEntryList_[index.row()].first;

    case RowTypeRole:
        return CategoryType;

    default:
        return QVariant();
    }
}

QVariant AddonModel::dataForItem(const QModelIndex &index, int role) const {
    const auto &addonList = addonEntryList_[index.parent().row()].second;
    const auto &addon = addonList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
        return addon.name();

    case CommentRole:
        return addon.comment();

    case ConfigurableRole:
        return addon.configurable();

    case AddonNameRole:
        return addon.uniqueName();

    case CategoryRole:
        return addon.category();

    case Qt::CheckStateRole:
        if (disabledList_.contains(addon.uniqueName())) {
            return false;
        } else if (enabledList_.contains(addon.uniqueName())) {
            return true;
        }
        return addon.enabled();

    case RowTypeRole:
        return AddonType;
    }
    return QVariant();
}

bool AddonModel::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
    if (!index.isValid() || !index.parent().isValid() ||
        index.parent().row() >= addonEntryList_.size() ||
        index.parent().column() > 0 || index.column() > 0) {
        return false;
    }

    const auto &addonList = addonEntryList_[index.parent().row()].second;

    if (index.row() >= addonList.size()) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        auto oldData = data(index, role).toBool();
        auto &item = addonList[index.row()];
        auto enabled = value.toBool();
        if (item.enabled() == enabled) {
            enabledList_.remove(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else if (enabled) {
            enabledList_.insert(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else {
            enabledList_.remove(item.uniqueName());
            disabledList_.insert(item.uniqueName());
        }
        auto newData = data(index, role).toBool();
        ret = oldData != newData;
    }

    if (ret) {
        emit dataChanged(index, index);
        emit changed();
    }

    return ret;
}

FlatAddonModel::FlatAddonModel(QObject *parent) : QAbstractListModel(parent) {}

bool FlatAddonModel::setData(const QModelIndex &index, const QVariant &value,
                             int role) {
    if (!index.isValid() || index.row() >= addonEntryList_.size() ||
        index.column() > 0) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        auto oldData = data(index, role).toBool();
        auto &item = addonEntryList_[index.row()];
        auto enabled = value.toBool();
        if (item.enabled() == enabled) {
            enabledList_.remove(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else if (enabled) {
            enabledList_.insert(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else {
            enabledList_.remove(item.uniqueName());
            disabledList_.insert(item.uniqueName());
        }
        auto newData = data(index, role).toBool();
        ret = oldData != newData;
    }

    if (ret) {
        emit dataChanged(index, index);
        emit changed();
    }

    return ret;
}

QVariant FlatAddonModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= addonEntryList_.size()) {
        return QVariant();
    }

    const FcitxQtAddonInfo &addon = addonEntryList_.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        return addon.name();

    case CommentRole:
        return addon.comment();

    case ConfigurableRole:
        return addon.configurable();

    case AddonNameRole:
        return addon.uniqueName();

    case CategoryRole:
        return addon.category();

    case CategoryNameRole:
        return categoryName(addon.category());

    case Qt::CheckStateRole:
        if (disabledList_.contains(addon.uniqueName())) {
            return false;
        } else if (enabledList_.contains(addon.uniqueName())) {
            return true;
        }
        return addon.enabled();

    case RowTypeRole:
        return AddonType;
    }
    return QVariant();
}

int FlatAddonModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return addonEntryList_.count();
}

QHash<int, QByteArray> FlatAddonModel::roleNames() const {
    return {
        {Qt::DisplayRole, "name"},          {CommentRole, "comment"},
        {ConfigurableRole, "configurable"}, {AddonNameRole, "uniqueName"},
        {CategoryRole, "category"},         {CategoryNameRole, "categoryName"},
        {Qt::CheckStateRole, "enabled"}};
}

void FlatAddonModel::setAddons(const fcitx::FcitxQtAddonInfoList &list) {
    beginResetModel();
    addonEntryList_ = list;
    enabledList_.clear();
    disabledList_.clear();
    endResetModel();
}

bool AddonProxyModel::filterAcceptsRow(int sourceRow,
                                       const QModelIndex &sourceParent) const {
    Q_UNUSED(sourceParent)
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(RowTypeRole) == CategoryType) {
        return filterCategory(index);
    }

    return filterAddon(index);
}

bool AddonProxyModel::filterCategory(const QModelIndex &index) const {
    int childCount = index.model()->rowCount(index);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterAddon(index.model()->index(i, 0, index))) {
            return true;
        }
    }
    return false;
}

bool AddonProxyModel::filterAddon(const QModelIndex &index) const {
    auto name = index.data(Qt::DisplayRole).toString();
    auto uniqueName = index.data(AddonNameRole).toString();
    auto comment = index.data(CommentRole).toString();

    if (!filterText_.isEmpty()) {
        return name.contains(filterText_, Qt::CaseInsensitive) ||
               uniqueName.contains(filterText_, Qt::CaseInsensitive) ||
               comment.contains(filterText_, Qt::CaseInsensitive);
    }

    return true;
}

bool AddonProxyModel::lessThan(const QModelIndex &left,
                               const QModelIndex &right) const {

    int lhs = left.data(CategoryRole).toInt();
    int rhs = right.data(CategoryRole).toInt();
    // Reorder the addon category.
    // UI and module are more common, because input method config is accessible
    // in the main page.
    static const QMap<int, int> category = {
        {static_cast<int>(AddonCategory::UI), 0},
        {static_cast<int>(AddonCategory::Module), 1},
        {static_cast<int>(AddonCategory::InputMethod), 2},
        {static_cast<int>(AddonCategory::Frontend), 3},
        {static_cast<int>(AddonCategory::Loader), 4},
    };

    int lvalue = category.value(lhs, category.size());
    int rvalue = category.value(rhs, category.size());
    int result = lvalue - rvalue;

    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    }

    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    return QCollator().compare(l, r) < 0;
}

void AddonProxyModel::setFilterText(const QString &text) {
    if (filterText_ != text) {
        filterText_ = text;
        invalidate();
    }
}

} // namespace kcm
} // namespace fcitx
