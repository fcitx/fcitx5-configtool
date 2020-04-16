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
#ifndef _CONFIGLIB_LAYOUTMODEL_H_
#define _CONFIGLIB_LAYOUTMODEL_H_

#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <fcitx-utils/i18n.h>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

enum { LayoutLanguageRole = 0x3423545, LayoutInfoRole };

class LanguageModel : public QStandardItemModel {
    Q_OBJECT
public:
    LanguageModel(QObject *parent = nullptr);
    Q_INVOKABLE QString language(int row) const;
    void append(const QString &name, const QString &language);
};

class LanguageFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage);

public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    // Forward role names.
    QHash<int, QByteArray> roleNames() const override {
        if (sourceModel()) {
            return sourceModel()->roleNames();
        }
        return QSortFilterProxyModel::roleNames();
    }

    const QString &language() const { return language_; }
    void setLanguage(const QString &language);

    Q_INVOKABLE QVariant layoutInfo(int row) const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &) const override;
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override;

private:
    QString language_;
};

class LayoutInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    using QAbstractListModel::QAbstractListModel;
    QHash<int, QByteArray> roleNames() const override;

    auto &layoutInfo() const { return layoutInfo_; }
    void setLayoutInfo(FcitxQtLayoutInfoList info);

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    FcitxQtLayoutInfoList layoutInfo_;
};

class VariantInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    using QAbstractListModel::QAbstractListModel;
    QHash<int, QByteArray> roleNames() const override;

    auto &variantInfo() const { return variantInfo_; }
    void setVariantInfo(const FcitxQtLayoutInfo &info);

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    FcitxQtVariantInfoList variantInfo_;
};

} // namespace kcm
} // namespace fcitx

#endif // _CONFIGLIB_LAYOUTMODEL_H_
