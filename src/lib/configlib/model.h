/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KCM_FCITX_MODEL_H_
#define _KCM_FCITX_MODEL_H_

#include <QAbstractListModel>
#include <QSet>
#include <QSortFilterProxyModel>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

enum {
    FcitxRowTypeRole = 0x324da8fc,
    FcitxLanguageRole,
    FcitxLanguageNameRole,
    FcitxIMUniqueNameRole,
    FcitxIMConfigurableRole,
    FcitxIMLayoutRole,
};

enum { LanguageType, IMType };

class IMConfigModelInterface {
public:
    virtual ~IMConfigModelInterface() = default;
    virtual void
    filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                      const FcitxQtStringKeyValueList &enabledIMs) = 0;
};

class CategorizedItemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    CategorizedItemModel(QObject *parent = 0);
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

protected:
    virtual int listSize() const = 0;
    virtual int subListSize(int idx) const = 0;
    virtual QVariant dataForItem(const QModelIndex &index, int role) const = 0;
    virtual QVariant dataForCategory(const QModelIndex &index,
                                     int role) const = 0;
};

class AvailIMModel : public CategorizedItemModel,
                     public IMConfigModelInterface {
    Q_OBJECT
public:
    AvailIMModel(QObject *parent = 0);
    void
    filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                      const FcitxQtStringKeyValueList &enabledIMs) override;

protected:
    int listSize() const override { return filteredIMEntryList.size(); }
    int subListSize(int idx) const override {
        return filteredIMEntryList[idx].second.size();
    }
    QVariant dataForItem(const QModelIndex &index, int role) const override;
    QVariant dataForCategory(const QModelIndex &index, int role) const override;

private:
    QList<QPair<QString, FcitxQtInputMethodEntryList>> filteredIMEntryList;
};

class IMProxyModel : public QSortFilterProxyModel,
                     public IMConfigModelInterface {
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText);
    Q_PROPERTY(bool showOnlyCurrentLanguage READ showOnlyCurrentLanguage WRITE
                   setShowOnlyCurrentLanguage);

public:
    IMProxyModel(QObject *parent = nullptr);

    // Forward role names.
    QHash<int, QByteArray> roleNames() const override {
        if (sourceModel()) {
            return sourceModel()->roleNames();
        }
        return QSortFilterProxyModel::roleNames();
    }

    const QString &filterText() const { return filterText_; }
    void setFilterText(const QString &text);
    bool showOnlyCurrentLanguage() const { return showOnlyCurrentLanguage_; }
    void setShowOnlyCurrentLanguage(bool checked);

    void
    filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                      const FcitxQtStringKeyValueList &enabledIMs) override;

protected:
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override;
    int compareCategories(const QModelIndex &left,
                          const QModelIndex &right) const;

private:
    bool filterLanguage(const QModelIndex &index) const;
    bool filterIM(const QModelIndex &index) const;

    bool showOnlyCurrentLanguage_ = true;
    QString filterText_;
    QSet<QString> languageSet_;
};

class FilteredIMModel : public QAbstractListModel,
                        public IMConfigModelInterface {
    Q_OBJECT
    Q_PROPERTY(int count READ count);

public:
    enum Mode { CurrentIM, AvailIM };

    FilteredIMModel(Mode mode, QObject *parent = nullptr);
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;
    void
    filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                      const FcitxQtStringKeyValueList &enabledIMs) override;

    int count() const { return rowCount(); }
    Q_INVOKABLE QString imAt(int idx) const {
        return index(idx).data(FcitxIMUniqueNameRole).toString();
    }
public slots:
    void move(int from, int to);
    void remove(int index);

signals:
    void imListChanged(FcitxQtInputMethodEntryList list);

private:
    Mode mode_;
    FcitxQtInputMethodEntryList filteredIMEntryList_;
    FcitxQtStringKeyValueList enabledIMList_;
};

} // namespace kcm

} // namespace fcitx

#endif // _KCM_FCITX_MODEL_H_
