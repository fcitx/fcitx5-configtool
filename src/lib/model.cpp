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

#include "model.h"
#include <QCollator>
#include <QLocale>
#include <fcitx-utils/i18n.h>

fcitx::kcm::CategorizedItemModel::CategorizedItemModel(QObject *parent)
    : QAbstractItemModel(parent) {}

int fcitx::kcm::CategorizedItemModel::rowCount(
    const QModelIndex &parent) const {
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

int fcitx::kcm::CategorizedItemModel::columnCount(const QModelIndex &) const {
    return 1;
}

QModelIndex
fcitx::kcm::CategorizedItemModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    int row = child.internalId();
    if (row && row - 1 >= listSize()) {
        return QModelIndex();
    }

    return createIndex(row - 1, 0, -1);
}

QModelIndex
fcitx::kcm::CategorizedItemModel::index(int row, int column,
                                        const QModelIndex &parent) const {
    // return language index
    if (!parent.isValid()) {
        if (column > 0 || row >= listSize()) {
            return QModelIndex();
        } else {
            return createIndex(row, column, static_cast<quintptr>(0));
        }
    }

    // return im index
    if (parent.column() > 0 || parent.row() >= listSize() ||
        row >= subListSize(parent.row())) {
        return QModelIndex();
    }

    return createIndex(row, column, parent.row() + 1);
}

QVariant fcitx::kcm::CategorizedItemModel::data(const QModelIndex &index,
                                                int role) const {
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

static QString languageName(const QString &langCode) {
    if (langCode.isEmpty()) {
        return _("Unknown");
    } else if (langCode == "*")
        return _("Multilingual");
    else {
        QLocale locale(langCode);
        if (locale.language() == QLocale::C) {
            // return lang code seems to be a better solution other than
            // indistinguishable "unknown"
            return langCode;
        }
        const bool hasCountry = langCode.indexOf("_") != -1 &&
                                locale.country() != QLocale::AnyCountry;
        QString languageName;
        if (hasCountry) {
            languageName = locale.nativeLanguageName();
        }
        if (languageName.isEmpty()) {
            languageName =
                D_("iso_639",
                   QLocale::languageToString(locale.language()).toUtf8());
        }
        if (languageName.isEmpty()) {
            languageName = _("Other");
        }
        QString countryName;
        // QLocale will always assign a default country for us, check if our
        // lang code

        if (langCode.indexOf("_") != -1 &&
            locale.country() != QLocale::AnyCountry) {
            countryName = locale.nativeCountryName();
            if (countryName.isEmpty()) {
                countryName = QLocale::countryToString(locale.country());
            }
        }

        if (countryName.isEmpty()) {
            return languageName;
        } else {
            return QString(
                       C_("%1 is language name, %2 is country name", "%1 (%2)"))
                .arg(languageName, countryName);
        }
    }
}

fcitx::kcm::AvailIMModel::AvailIMModel(QObject *parent)
    : fcitx::kcm::CategorizedItemModel(parent) {}

QVariant fcitx::kcm::AvailIMModel::dataForCategory(const QModelIndex &index,
                                                   int role) const {
    switch (role) {

    case Qt::DisplayRole:
        return languageName(filteredIMEntryList[index.row()].first);

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

QVariant fcitx::kcm::AvailIMModel::dataForItem(const QModelIndex &index,
                                               int role) const {
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
    }
    return QVariant();
}

void fcitx::kcm::AvailIMModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMList, const QString &selection) {
    beginResetModel();

    QMap<QString, int> languageMap;
    filteredIMEntryList.clear();
    int langRow = -1;
    int imRow = -1;

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
                QPair<QString, FcitxQtInputMethodEntryList>(
                    im.languageCode(), FcitxQtInputMethodEntryList()));
        } else {
            idx = languageMap[im.languageCode()];
        }
        filteredIMEntryList[idx].second.append(im);
        if (im.uniqueName() == selection) {
            langRow = idx;
            imRow = filteredIMEntryList[idx].second.count() - 1;
        }
    }
    endResetModel();

    if (imRow >= 0) {
        emit select(index(imRow, 0, index(langRow, 0)));
    }
}

fcitx::kcm::IMProxyModel::IMProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent) {}

void fcitx::kcm::IMProxyModel::setFilterText(const QString &text) {
    if (filterText_ != text) {
        filterText_ = text;
        invalidate();
    }
}

void fcitx::kcm::IMProxyModel::setShowOnlyCurrentLanguage(bool show) {
    if (showOnlyCurrentLanguage_ != show) {
        showOnlyCurrentLanguage_ = show;
        invalidate();
    }
}

void fcitx::kcm::IMProxyModel::filterIMEntryList(
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

bool fcitx::kcm::IMProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex &source_parent) const {
    const QModelIndex index =
        sourceModel()->index(source_row, 0, source_parent);

    if (index.data(FcitxRowTypeRole) == LanguageType) {
        return filterLanguage(index);
    }

    return filterIM(index);
}

bool fcitx::kcm::IMProxyModel::filterLanguage(const QModelIndex &index) const {
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

bool fcitx::kcm::IMProxyModel::filterIM(const QModelIndex &index) const {
    QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    QString name = index.data(Qt::DisplayRole).toString();
    QString langCode = index.data(FcitxLanguageRole).toString();

    if (uniqueName == "keyboard-us")
        return true;

    bool flag = true;
    QString lang = langCode.left(2);

    flag =
        flag && (showOnlyCurrentLanguage_
                     ? !lang.isEmpty() && (QLocale().name().startsWith(lang) ||
                                           languageSet_.contains(lang))
                     : true);
    if (!filterText_.isEmpty()) {
        flag = flag && (name.contains(filterText_, Qt::CaseInsensitive) ||
                        uniqueName.contains(filterText_, Qt::CaseInsensitive) ||
                        langCode.contains(filterText_, Qt::CaseInsensitive) ||
                        languageName(langCode).contains(filterText_,
                                                        Qt::CaseInsensitive));
    }
    return flag;
}

bool fcitx::kcm::IMProxyModel::lessThan(const QModelIndex &left,
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

int fcitx::kcm::IMProxyModel::compareCategories(
    const QModelIndex &left, const QModelIndex &right) const {
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

fcitx::kcm::CurrentIMModel::CurrentIMModel(QObject *parent)
    : QAbstractListModel(parent) {}

QModelIndex fcitx::kcm::CurrentIMModel::index(int row, int column,
                                              const QModelIndex &parent) const {
    Q_UNUSED(parent);

    return createIndex(row, column,
                       (row < filteredIMEntryList_.count())
                           ? (void *)&filteredIMEntryList_.at(row)
                           : 0);
}

QVariant fcitx::kcm::CurrentIMModel::data(const QModelIndex &index,
                                          int role) const {
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

    default:
        return QVariant();
    }
}

int fcitx::kcm::CurrentIMModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return filteredIMEntryList_.count();
}

void fcitx::kcm::CurrentIMModel::filterIMEntryList(
    const FcitxQtInputMethodEntryList &imEntryList,
    const FcitxQtStringKeyValueList &enabledIMs, const QString &selection) {
    beginResetModel();

    FcitxQtStringKeyValueList languageSet;
    filteredIMEntryList_.clear();
    int row = 0, selectionRow = -1;
    QMap<QString, const FcitxQtInputMethodEntry *> nameMap;
    for (auto &imEntry : imEntryList) {
        nameMap.insert(imEntry.uniqueName(), &imEntry);
    }

    for (const auto &im : enabledIMs) {
        if (auto value = nameMap.value(im.key(), nullptr)) {
            filteredIMEntryList_.append(*value);
            if (im.key() == selection)
                selectionRow = row;
            row++;
        }
    }
    endResetModel();

    if (selectionRow >= 0) {
        emit select(index(selectionRow, 0));
    } else if (row > 0) {
        emit select(index(row - 1, 0));
    }
}
