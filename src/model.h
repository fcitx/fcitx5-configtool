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
#ifndef _KCM_FCITX_MODEL_H_
#define _KCM_FCITX_MODEL_H_

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

enum {
    FcitxRowTypeRole = 0x324da8fc,
    FcitxLanguageRole,
    FcitxIMUniqueNameRole,
    FcitxIMConfigurableRole,
};

enum { LanguageType, IMType };

class AvailIMModel : public QAbstractItemModel {
    Q_OBJECT
public:
    AvailIMModel(QObject *parent = 0);
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
signals:
    void select(QModelIndex index);
    void updateIMListFinished();
public slots:
    void filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                           const FcitxQtStringKeyValueList &enabledIMs,
                           const QString &selection = QString());

private:
    QList<QPair<QString, FcitxQtInputMethodEntryList>> filteredIMEntryList;
};

class IMProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    IMProxyModel(QObject *parent = nullptr);
public slots:
    void setFilterText(const QString &text);
    void setShowOnlyCurrentLanguage(bool checked);
    void filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                           const FcitxQtStringKeyValueList &enabledIMs);

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

    bool m_showOnlyCurrentLanguage = true;
    QString m_filterText;
    QSet<QString> m_languageSet;
};

class CurrentIMModel : public QAbstractListModel {
    Q_OBJECT
public:
    CurrentIMModel(QObject *parent = nullptr);
    virtual QModelIndex index(int row, int column = 0,
                              const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index,
                          int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
signals:
    void select(QModelIndex index);
public slots:
    void filterIMEntryList(const FcitxQtInputMethodEntryList &imEntryList,
                           const FcitxQtStringKeyValueList &enabledIMs,
                           const QString &selection = QString());

private:
    FcitxQtInputMethodEntryList filteredIMEntryList;
};

} // namespace kcm

} // namespace fcitx

#endif // _KCM_FCITX_MODEL_H_
