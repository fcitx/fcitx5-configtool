/***************************************************************************
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef __FCITX_IM_PAGE_P_H__
#define __FCITX_IM_PAGE_P_H__

// Qt
#include <QObject>
#include <QSet>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

// Fcitx
#include <fcitxqtinputmethoditem.h>

// self
#include "impage.h"

enum {
    FcitxRowTypeRole = 0x324da8fc,
    FcitxLanguageRole,
    FcitxIMUniqueNameRole
};

enum {
    LanguageType,
    IMType
};

class QTreeView;
class QCheckBox;
class QListView;
class QPushButton;
class QLineEdit;
namespace Fcitx
{

class IMPage::Private
    : public QObject
{
    Q_OBJECT
public:
    Private(QObject* parent);
    virtual ~Private();
    void fetchIMList();
    const FcitxQtInputMethodItemList& getIMList();
    int dependantLayoutValue(int value, int width, int totalWidth) const;

    class IMModel;
    class AvailIMModel;
    class IMProxyModel;

    class IMDelegate;

    QPushButton* addIMButton;
    QPushButton* removeIMButton;
    QPushButton* moveUpButton;
    QPushButton* moveDownButton;
    QPushButton* configureButton;
    QListView* currentIMView;
    QTreeView* availIMView;
    QLineEdit* filterTextEdit;

    AvailIMModel* availIMModel;
    IMProxyModel* availIMProxyModel;

    IMModel* currentIMModel;
    QCheckBox* onlyCurrentLanguageCheckBox;
    Module* module;
    QPushButton* defaultLayoutButton;

Q_SIGNALS:
    void updateIMList(const FcitxQtInputMethodItemList& list, const QString& selection);
    void changed();

public Q_SLOTS:
    void availIMSelectionChanged();
    void currentIMCurrentChanged();
    void clickAddIM();
    void clickRemoveIM();
    void addIM(const QModelIndex& index);
    void removeIM(const QModelIndex& index);
    void moveUpIM();
    void moveDownIM();
    void configureIM();
    void save();
    void doubleClickCurrentIM(const QModelIndex& index);
    void doubleClickAvailIM(const QModelIndex& index);
    void selectCurrentIM(const QModelIndex& index);
    void selectAvailIM(const QModelIndex& index);
    void selectDefaultLayout();
    void onConnectStatusChanged(bool connected);
private:
    FcitxQtInputMethodItemList m_list;
};

class IMDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit IMDelegate(QObject* parent = 0);
    virtual ~IMDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;
};


class IMPage::Private::IMProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    IMProxyModel(QAbstractItemModel* sourceModel);
    virtual ~IMProxyModel();
    void setFilterText(const QString& text);
    void setShowOnlyCurrentLanguage(bool checked);
public Q_SLOTS:
    void filterIMEntryList(const FcitxQtInputMethodItemList& imEntryList, const QString& selection = QString());

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const Q_DECL_OVERRIDE;
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const Q_DECL_OVERRIDE;
    int compareCategories(const QModelIndex& left, const QModelIndex& right) const;

private:
    bool filterLanguage(const QModelIndex& index) const;
    bool filterIM(const QModelIndex& index) const;

    bool m_showOnlyCurrentLanguage;
    QString m_filterText;
    QSet< QString > m_languageSet;
};

class IMPage::Private::AvailIMModel : public QAbstractItemModel
{
    Q_OBJECT
    void m_languageSet();

public:
    AvailIMModel(QObject* parent = 0);
    virtual ~AvailIMModel();
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex& child) const Q_DECL_OVERRIDE;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
Q_SIGNALS:
    void select(QModelIndex index);
    void updateIMListFinished();
public Q_SLOTS:
    void filterIMEntryList(const FcitxQtInputMethodItemList& imEntryList, const QString& selection = QString());
private:
    QList<QPair<QString, FcitxQtInputMethodItemList> > filteredIMEntryList;
};

class IMPage::Private::IMModel : public QAbstractListModel
{
    Q_OBJECT
public:

    IMModel(QObject* parent = 0);
    virtual ~IMModel();

    virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
Q_SIGNALS:
    void select(QModelIndex index);
public Q_SLOTS:
    void filterIMEntryList(const FcitxQtInputMethodItemList& imEntryList, const QString& selection = QString());
private:
    FcitxQtInputMethodItemList filteredIMEntryList;
};

}

#endif
