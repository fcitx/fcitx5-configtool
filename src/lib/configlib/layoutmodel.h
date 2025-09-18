/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _CONFIGLIB_LAYOUTMODEL_H_
#define _CONFIGLIB_LAYOUTMODEL_H_

#include <QAbstractItemModel>
#include <QHash>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <Qt>
#include <fcitx-utils/i18n.h>
#include <fcitxqtdbustypes.h>
#include <qdebug.h>
#include <qlogging.h>
#include <qnamespace.h>

namespace fcitx::kcm {

enum { LayoutLanguageRole = 0x3423545, LayoutInfoRole };

class LanguageModel : public QStandardItemModel {
    Q_OBJECT
public:
    LanguageModel(QObject *parent = nullptr);
    void append(const QString &name, const QString &language);
};

class SortedLanguageModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;
    // Forward role names.
    QHash<int, QByteArray> roleNames() const override {
        if (sourceModel()) {
            return sourceModel()->roleNames();
        }
        return QSortFilterProxyModel::roleNames();
    }
    Q_INVOKABLE QString language(int row) const;

protected:
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override {
        const bool leftIsAny = left.data(Qt::UserRole).toString().isEmpty();
        const bool rightIsAny = right.data(Qt::UserRole).toString().isEmpty();
        if (leftIsAny || rightIsAny) {
            return leftIsAny > rightIsAny;
        }
        return QString::localeAwareCompare(
                   left.data(Qt::DisplayRole).toString(),
                   right.data(Qt::DisplayRole).toString()) < 0;
    }
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

} // namespace fcitx::kcm

#endif // _CONFIGLIB_LAYOUTMODEL_H_
