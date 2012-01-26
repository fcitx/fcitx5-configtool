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

// KDE
#include <KWidgetItemDelegate>
#include <KCategorizedSortFilterProxyModel>
#include <KLocale>

// self
#include "FcitxIMPage.h"

class QCheckBox;
class KCategorizedView;
class QListView;
class KPushButton;
class KLineEdit;
class KCategoryDrawer;
namespace Fcitx
{

class FcitxIMPage::Private
    : public QObject
{
    Q_OBJECT
public:
    Private(QObject* parent);
    virtual ~Private();
    void fetchIMList();
    const FcitxIMList& getIMList();
    int dependantLayoutValue(int value, int width, int totalWidth) const;

    class IMModel;

    class IMProxyModel;
    
    class IMDelegate;

    KPushButton* addIMButton;
    KPushButton* removeIMButton;
    KPushButton* moveUpButton;
    KPushButton* moveDownButton;
    QListView* currentIMView;
    KCategorizedView* availIMView;
    KLineEdit* filterTextEdit;

    IMModel* availIMModel;
    IMProxyModel* availIMProxyModel;
    KCategoryDrawer* categoryDrawer;

    IMModel* currentIMModel;
    QCheckBox* onlyCurrentLanguageCheckBox;

Q_SIGNALS:
    void updateIMList(QString selection);
    void changed();

public Q_SLOTS:
    void availIMSelectionChanged();
    void currentIMCurrentChanged();
    void addIM();
    void removeIM();
    void moveUpIM();
    void moveDownIM();
    void save();
    void selectCurrentIM(const QModelIndex& index);
    void selectAvailIM(const QModelIndex& index);

private:
    QDBusConnection m_connection;
    org::fcitx::Fcitx::InputMethod* m_inputmethod;
    FcitxIMList m_list;
};


class FcitxIMPage::Private::IMProxyModel
    : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:
    IMProxyModel(FcitxIMPage::Private *impage_d, QObject* parent = 0);
    virtual ~IMProxyModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
    FcitxIMPage::Private* impage_d;
};

class FcitxIMPage::Private::IMModel : public QAbstractListModel
{
    Q_OBJECT
public:

    IMModel(FcitxIMPage::Private *impage_d, QObject* parent = 0);
    virtual ~IMModel();

    virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    void setShowOnlyEnabled(bool show);
Q_SIGNALS:
    void select(QModelIndex index);
private Q_SLOTS:
    void filterIMEntryList(const QString& selection = QString());
private:
    Private* impage_d;
    bool showOnlyEnabled;
    KLocale locale;
    FcitxIMList filteredIMEntryList;
};


class FcitxIMPage::Private::IMDelegate
    : public KWidgetItemDelegate
{
public:
    explicit IMDelegate(Private* impage_d, QObject* parent = 0);
    virtual ~IMDelegate();
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QList< QWidget* > createItemWidgets() const;
    virtual void updateItemWidgets(const QList< QWidget* > widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
private:
    Private* impage_d;
};

}

#endif
