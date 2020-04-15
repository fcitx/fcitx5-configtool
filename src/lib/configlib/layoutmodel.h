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
    explicit LanguageFilterModel(QObject *parent)
        : QSortFilterProxyModel(parent) {}

    // Forward role names.
    QHash<int, QByteArray> roleNames() const override {
        if (sourceModel()) {
            return sourceModel()->roleNames();
        }
        return QSortFilterProxyModel::roleNames();
    }

    const QString &language() const { return language_; }

    void setLanguage(const QString &language) {
        if (language_ != language) {
            language_ = language;
            invalidateFilter();
        }
    }

    Q_INVOKABLE QVariant layoutInfo(int row) const;

public:
    bool filterAcceptsRow(int source_row, const QModelIndex &) const override {
        if (language_.isEmpty()) {
            return true;
        }

        auto index = sourceModel()->index(source_row, 0);
        return sourceModel()
            ->data(index, LayoutLanguageRole)
            .toStringList()
            .contains(language_);
    }
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override {
        return data(left, Qt::DisplayRole).toString() <
               data(right, Qt::DisplayRole).toString();
    }

private:
    QString language_;
};

class LayoutInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    LayoutInfoModel(QObject *parent) : QAbstractListModel(parent) {}

    QHash<int, QByteArray> roleNames() const override {
        return {
            {Qt::DisplayRole, "name"},
            {Qt::UserRole, "layout"},
            {LayoutLanguageRole, "language"},
        };
    }

    auto &layoutInfo() const { return layoutInfo_; }
    void setLayoutInfo(FcitxQtLayoutInfoList info) {
        beginResetModel();
        layoutInfo_ = std::move(info);
        endResetModel();
    }

    QModelIndex
    index(int row, int column = 0,
          const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return createIndex(row, column);
    }
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= layoutInfo_.size()) {
            return QVariant();
        }
        const auto &layout = layoutInfo_.at(index.row());

        switch (role) {
        case Qt::DisplayRole:
            return layout.description();
        case Qt::UserRole:
            return layout.layout();
        case LayoutLanguageRole: {
            QStringList languages;
            languages = layout.languages();
            for (const auto &variants : layout.variants()) {
                languages << variants.languages();
            }
            return languages;
        }
        case LayoutInfoRole:
            return QVariant::fromValue(layout);
        }
        return QVariant();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }

        return layoutInfo_.size();
    }

private:
    FcitxQtLayoutInfoList layoutInfo_;
};

class VariantInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    VariantInfoModel(QObject *parent) : QAbstractListModel(parent) {}

    QHash<int, QByteArray> roleNames() const override {
        return {
            {Qt::DisplayRole, "name"},
            {Qt::UserRole, "variant"},
            {LayoutLanguageRole, "language"},
        };
    }

    auto &variantInfo() const { return variantInfo_; }
    void setVariantInfo(const FcitxQtLayoutInfo &info) {
        beginResetModel();
        variantInfo_.clear();
        FcitxQtVariantInfo defaultVariant;
        defaultVariant.setVariant("");
        defaultVariant.setDescription(_("Default"));
        defaultVariant.setLanguages(info.languages());
        variantInfo_ << defaultVariant;
        variantInfo_ << info.variants();
        endResetModel();
    }

    QModelIndex
    index(int row, int column = 0,
          const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return createIndex(row, column);
    }
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= variantInfo_.size()) {
            return QVariant();
        }
        const auto &layout = variantInfo_.at(index.row());

        switch (role) {

        case Qt::DisplayRole:
            return layout.description();

        case Qt::UserRole:
            return layout.variant();

        case LayoutLanguageRole:
            return layout.languages();

        default:
            return QVariant();
        }
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }

        return variantInfo_.size();
    }

private:
    FcitxQtVariantInfoList variantInfo_;
};

} // namespace kcm
} // namespace fcitx

#endif // _CONFIGLIB_LAYOUTMODEL_H_
