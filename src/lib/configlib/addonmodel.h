/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _CONFIGLIB_ADDONMODEL_H_
#define _CONFIGLIB_ADDONMODEL_H_

#include "model.h"

namespace fcitx {
namespace kcm {

enum ExtraRoles {
    CommentRole = 0x19880209,
    ConfigurableRole,
    AddonNameRole,
    RowTypeRole,
    CategoryRole,
    CategoryNameRole
};

enum RowType {
    CategoryType,
    AddonType,
};

class AddonModel : public CategorizedItemModel {
    Q_OBJECT

public:
    explicit AddonModel(QObject *parent);
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    void setAddons(const FcitxQtAddonInfoList &list) {
        beginResetModel();

        addonEntryList_.clear();
        QMap<int, int> addonCategoryMap;
        for (const FcitxQtAddonInfo &addon : list) {
            int idx;
            if (!addonCategoryMap.contains(addon.category())) {
                idx = addonEntryList_.count();
                addonCategoryMap[addon.category()] = idx;
                addonEntryList_.append(QPair<int, FcitxQtAddonInfoList>(
                    addon.category(), FcitxQtAddonInfoList()));
            } else {
                idx = addonCategoryMap[addon.category()];
            }
            addonEntryList_[idx].second.append(addon);
        }
        enabledList_.clear();
        disabledList_.clear();
        endResetModel();
    }

    const auto &enabledList() const { return enabledList_; }
    const auto &disabledList() const { return disabledList_; }

signals:
    void changed();

protected:
    int listSize() const override { return addonEntryList_.size(); }
    int subListSize(int idx) const override {
        return addonEntryList_[idx].second.size();
    }
    QVariant dataForItem(const QModelIndex &index, int role) const override;
    QVariant dataForCategory(const QModelIndex &index, int role) const override;

private:
    QSet<QString> enabledList_;
    QSet<QString> disabledList_;
    QList<QPair<int, FcitxQtAddonInfoList>> addonEntryList_;
};

class FlatAddonModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit FlatAddonModel(QObject *parent);
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    void setAddons(const FcitxQtAddonInfoList &list);

    const auto &enabledList() const { return enabledList_; }
    const auto &disabledList() const { return disabledList_; }

signals:
    void changed();

private:
    QSet<QString> enabledList_;
    QSet<QString> disabledList_;
    FcitxQtAddonInfoList addonEntryList_;
};

class AddonProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText);

public:
    explicit AddonProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {}

    // Forward role names.
    QHash<int, QByteArray> roleNames() const override {
        if (sourceModel()) {
            return sourceModel()->roleNames();
        }
        return QSortFilterProxyModel::roleNames();
    }

    const QString &filterText() const { return filterText_; }
    void setFilterText(const QString &text);

protected:
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override;

private:
    bool filterCategory(const QModelIndex &index) const;
    bool filterAddon(const QModelIndex &index) const;
    QString filterText_;
};

} // namespace kcm
} // namespace fcitx

#endif // _CONFIGLIB_ADDONMODEL_H_
