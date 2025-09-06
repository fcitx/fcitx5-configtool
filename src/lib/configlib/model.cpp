/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "model.h"
#include <QAbstractItemModel>
#include <QCollator>
#include <QHash>
#include <QLocale>
#include <QObject>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringLiteral>
#include <Qt>
#include <QtTypes>
#include <algorithm>
#include <fcitx-utils/i18n.h>
#include <fcitxqtdbustypes.h>
#include <utility>

namespace fcitx::kcm {

const IMConfigModelInterface::LanguageNames &
IMConfigModelInterface::languageNames(const QString &langCode) const {
    auto iter = languageNames_.find(langCode);
    if (iter == languageNames_.end()) {
        LanguageNames names;
        if (langCode.isEmpty()) {
            names.nativeName = names.localName = _("Unknown");
        } else if (langCode == "*" || langCode == "mul") {
            names.nativeName = names.localName = _("Multilingual");
        } else {
            QString lang = langCode;
            // The lang before _.
            QString territory;
            if (auto items = langCode.split('_'); !items.isEmpty()) {
                if (!items.first().isEmpty()) {
                    lang = items.first();
                }
                if (items.size() > 1) {
                    QLocale locale(langCode);
                    if (locale.territory() != QLocale::AnyTerritory) {
                        territory =
                            QLocale::territoryToString(locale.territory());
                    }
                    if (!territory.isEmpty()) {
                        territory =
                            D_("iso_3166", territory.toUtf8().constData());
                    }
                }
            }
            names.nativeName = iso639_.queryNative(lang);
            names.localName = iso639_.query(lang);
            if (!territory.isEmpty()) {
                names.localName =
                    QString(C_("%1 is language name, %2 is country name",
                               "%1 (%2)"))
                        .arg(names.localName, territory);
            }
        }
        iter = languageNames_.insert(langCode, std::move(names));
    }
    return iter.value();
}

CategorizedItemModel::CategorizedItemModel(QObject *parent)
    : QAbstractItemModel(parent) {}

int CategorizedItemModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return listSize();
    }

    if (parent.internalId() > 0) {
        return 0;
    }

    if (parent.column() > 0 || parent.row() >= listSize()) {
        return 0;
    }

    return subListSize(parent.row());
}

int CategorizedItemModel::columnCount(const QModelIndex & /*parent*/) const {
    return 1;
}

QModelIndex CategorizedItemModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    int row = child.internalId();
    if (row && row - 1 >= listSize()) {
        return QModelIndex();
    }

    return createIndex(row - 1, 0, -1);
}

QModelIndex CategorizedItemModel::index(int row, int column,
                                        const QModelIndex &parent) const {
    // return language index
    if (!parent.isValid()) {
        if (column > 0 || row >= listSize()) {
            return QModelIndex();
        }
        return createIndex(row, column, static_cast<quintptr>(0));
    }

    // return im index
    if (parent.column() > 0 || parent.row() >= listSize() ||
        row >= subListSize(parent.row())) {
        return QModelIndex();
    }

    return createIndex(row, column, parent.row() + 1);
}

QVariant CategorizedItemModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (!index.parent().isValid()) {
        if (index.column() > 0 || index.row() >= listSize()) {
            return QVariant();
        }

        return dataForCategory(index, role);
    }

    if (index.column() > 0 || index.parent().column() > 0 ||
        index.parent().row() >= listSize()) {
        return QVariant();
    }

    if (index.row() >= subListSize(index.parent().row())) {
        return QVariant();
    }
    return dataForItem(index, role);
}

AvailIMModel::AvailIMModel(QObject *parent) : CategorizedItemModel(parent) {}

QVariant AvailIMModel::dataForCategory(const QModelIndex &index,
                                       int role) const {
    switch (role) {

    case Qt::DisplayRole:
        return languageNames(filteredIMEntryList[index.row()].first).localName;

    case FcitxLanguageRole:
        return filteredIMEntryList[index.row()].first;

    case FcitxIMUniqueNameRole:
        return QString();

    case FcitxRowTypeRole:
        return LanguageType;

    default:
        return QVariant();
    }
}

QVariant AvailIMModel::dataForItem(const QModelIndex &index, int role) const {
    const FcitxQtInputMethodEntryList &imEntryList =
        filteredIMEntryList[index.parent().row()].second;

    const FcitxQtInputMethodEntry &imEntry = imEntryList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
        return imEntry.name();

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.languageCode();

    default:
        break;
    }
    return {};
}

void AvailIMModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {
    beginResetModel();

    QMap<QString, int> languageMap;
    filteredIMEntryList.clear();

    QSet<QString> enabledIMs;
    for (const auto &item : enabledIMList) {
        enabledIMs.insert(item.key());
    }

    for (const FcitxQtInputMethodEntry &im : imEntryList) {
        if (enabledIMs.contains(im.uniqueName())) {
            continue;
        }
        int idx;
        if (!languageMap.contains(im.languageCode())) {
            idx = filteredIMEntryList.count();
            languageMap[im.languageCode()] = idx;
            filteredIMEntryList.append(
                std::pair<QString, FcitxQtInputMethodEntryList>(
                    im.languageCode(), FcitxQtInputMethodEntryList()));
        } else {
            idx = languageMap[im.languageCode()];
        }
        filteredIMEntryList[idx].second.append(im);
    }
    endResetModel();
}

IMProxyModel::IMProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
    sort(0);
}

void IMProxyModel::setFilterText(const QString &text) {
    if (filterText_ != text) {
        filterText_ = text;
        invalidate();
    }
}

void IMProxyModel::setShowOnlyCurrentLanguage(bool show) {
    if (showOnlyCurrentLanguage_ != show) {
        showOnlyCurrentLanguage_ = show;
        invalidate();
    }
}

void IMProxyModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {
    languageSet_.clear();

    QSet<QString> enabledIMs;
    for (const auto &item : enabledIMList) {
        enabledIMs.insert(item.key());
    }
    for (const FcitxQtInputMethodEntry &im : imEntryList) {
        if (enabledIMs.contains(im.uniqueName())) {
            languageSet_.insert(im.languageCode().left(2));
        }
    }
    invalidate();
}

bool IMProxyModel::filterAcceptsRow(int source_row,
                                    const QModelIndex &source_parent) const {
    const QModelIndex index =
        sourceModel()->index(source_row, 0, source_parent);

    if (index.data(FcitxRowTypeRole) == LanguageType) {
        return filterLanguage(index);
    }

    return filterIM(index);
}

bool IMProxyModel::filterLanguage(const QModelIndex &index) const {
    if (!index.isValid()) {
        return false;
    }

    int childCount = index.model()->rowCount(index);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterIM(index.model()->index(i, 0, index))) {
            return true;
        }
    }

    return false;
}

bool IMProxyModel::filterIM(const QModelIndex &index) const {
    QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    QString name = index.data(Qt::DisplayRole).toString();
    QString langCode = index.data(FcitxLanguageRole).toString();

    // Always show keyboard us if we are not searching.
    if (uniqueName == "keyboard-us" && filterText_.isEmpty()) {
        return true;
    }

    QString lang = langCode;
    // The lang before _.
    if (auto items = langCode.split('_');
        !items.isEmpty() && !items[0].isEmpty()) {
        lang = items.first();
    }

    if (filterText_.isEmpty()) {
        if (showOnlyCurrentLanguage_) {
            return !lang.isEmpty() && (QLocale().name().startsWith(lang) ||
                                       languageSet_.contains(lang));
        }
        return true;
    }

    if (name.contains(filterText_, Qt::CaseInsensitive) ||
        uniqueName.contains(filterText_, Qt::CaseInsensitive) ||
        langCode.contains(filterText_, Qt::CaseInsensitive)) {
        return true;
    }

    const auto &langNames = languageNames(langCode);
    if (langNames.localName.contains(filterText_, Qt::CaseInsensitive) ||
        langNames.nativeName.contains(filterText_, Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool IMProxyModel::lessThan(const QModelIndex &left,
                            const QModelIndex &right) const {
    int result = compareCategories(left, right);
    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    }

    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    return QCollator().compare(l, r) < 0;
}

int IMProxyModel::compareCategories(const QModelIndex &left,
                                    const QModelIndex &right) const {
    QString l = left.data(FcitxLanguageRole).toString();
    QString r = right.data(FcitxLanguageRole).toString();

    if (l == r)
        return 0;

    if (QLocale().name() == l)
        return -1;

    if (QLocale().name() == r)
        return 1;

    bool fl = QLocale().name().startsWith(l.left(2));
    bool fr = QLocale().name().startsWith(r.left(2));

    if (fl == fr) {
        return l.size() == r.size() ? l.compare(r) : l.size() - r.size();
    }
    return fl ? -1 : 1;
}

FilteredIMModel::FilteredIMModel(Mode mode, QObject *parent)
    : QAbstractListModel(parent), mode_(mode) {}

QVariant FilteredIMModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= filteredIMEntryList_.size()) {
        return QVariant();
    }

    const FcitxQtInputMethodEntry &imEntry =
        filteredIMEntryList_.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        return imEntry.name();

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.languageCode();

    case FcitxIMConfigurableRole:
        return imEntry.configurable();

    case FcitxLanguageNameRole:
        return languageNames(imEntry.languageCode()).localName;

    case FcitxIMLayoutRole: {
        auto iter = std::find_if(enabledIMList_.begin(), enabledIMList_.end(),
                                 [&imEntry](const FcitxQtStringKeyValue &item) {
                                     return item.key() == imEntry.uniqueName();
                                 });
        if (iter != enabledIMList_.end()) {
            return iter->value();
        }
        return QString();
    }
    case FcitxIMActiveRole:
        return index.row() > 0 ? QStringLiteral("active")
                               : QStringLiteral("inactive");

    default:
        return {};
    }
}

int FilteredIMModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return filteredIMEntryList_.count();
}

QHash<int, QByteArray> FilteredIMModel::roleNames() const {
    return {{Qt::DisplayRole, "name"},
            {FcitxIMUniqueNameRole, "uniqueName"},
            {FcitxLanguageRole, "languageCode"},
            {FcitxLanguageNameRole, "language"},
            {FcitxIMConfigurableRole, "configurable"},
            {FcitxIMLayoutRole, "layout"},
            {FcitxIMActiveRole, "active"}};
}

void FilteredIMModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList) {
    beginResetModel();

    filteredIMEntryList_.clear();
    enabledIMList_ = enabledIMList;

    // We implement this twice for following reasons:
    // 1. "enabledIMs" is usually very small.
    // 2. CurrentIM mode need to keep order by enabledIMs.
    if (mode_ == CurrentIM) {
        QMap<QString, const FcitxQtInputMethodEntry *> nameMap;
        for (auto &imEntry : imEntryList) {
            nameMap.insert(imEntry.uniqueName(), &imEntry);
        }

        for (const auto &im : enabledIMList) {
            if (auto value = nameMap.value(im.key(), nullptr)) {
                filteredIMEntryList_.append(*value);
            }
        }
    } else if (mode_ == AvailIM) {
        QSet<QString> enabledIMs;
        for (const auto &item : enabledIMList) {
            enabledIMs.insert(item.key());
        }

        for (const FcitxQtInputMethodEntry &im : imEntryList) {
            if (enabledIMs.contains(im.uniqueName())) {
                continue;
            }
            filteredIMEntryList_.append(im);
        }
    }
    endResetModel();
}

void FilteredIMModel::move(int from, int to) {
    if (from < 0 || from >= filteredIMEntryList_.size() || to < 0 ||
        to >= filteredIMEntryList_.size()) {
        return;
    }
    beginMoveRows(QModelIndex(), from, from, QModelIndex(),
                  to > from ? to + 1 : to);
    filteredIMEntryList_.move(from, to);
    endMoveRows();
    Q_EMIT imListChanged(filteredIMEntryList_);
}

void FilteredIMModel::remove(int idx) {
    if (idx < 0 || idx >= filteredIMEntryList_.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), idx, idx);
    filteredIMEntryList_.removeAt(idx);
    endRemoveRows();
    Q_EMIT imListChanged(filteredIMEntryList_);
}

} // namespace fcitx::kcm
