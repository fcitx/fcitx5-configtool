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

#ifndef __FCITX_ADDON_SELECTOR_P_H__
#define __FCITX_ADDON_SELECTOR_P_H__

// Qt
#include <QObject>
#include <QAbstractListModel>
#include <QLineEdit>

// KDE
#include <KCategorizedSortFilterProxyModel>
#include <KWidgetItemDelegate>

// Fcitx
#include <fcitx/addon.h>

// self
#include "addonselector.h"

class KCategoryDrawer;

class KCategorizedView;

class KLineEdit;

class QPushButton;

class QCheckBox;

namespace Fcitx
{

class AddonSelector::Private
    : public QObject
{
    Q_OBJECT

public:
    enum ExtraRoles {
        CommentRole       = 0x19880209,
        ConfigurableRole  = 0x20080331
    };
    Private(AddonSelector* parent);
    virtual ~Private();
    int dependantLayoutValue(int value, int width, int totalWidth) const;

public:

    class AddonModel;

    class ProxyModel;

    class AddonDelegate;
    QLineEdit* lineEdit;
    KCategorizedView* listView;
    KCategoryDrawer *categoryDrawer;
    AddonModel *addonModel;
    ProxyModel *proxyModel;
    AddonDelegate *addonDelegate;
    QCheckBox* advanceCheckbox;

private:
    AddonSelector* parent;
};

class AddonSelector::Private::AddonModel : public QAbstractListModel
{
    Q_OBJECT

public:
    AddonModel(AddonSelector::Private *addonSelector_d, QObject* parent = 0);
    virtual ~AddonModel();

    virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    void addAddon(FcitxAddon* addon);

private:
    QList<FcitxAddon*> addonEntryList;
    AddonSelector::Private *addonSelector_d;
};

class AddonSelector::Private::ProxyModel
    : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:
    ProxyModel(AddonSelector::Private *addonSelector_d, QObject* parent = 0);
    virtual ~ProxyModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
    AddonSelector::Private* addonSelector_d;
};

class AddonSelector::Private::AddonDelegate
    : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    AddonDelegate(AddonSelector::Private *addonSelector_d, QObject* parent = 0);
    virtual ~AddonDelegate();

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

Q_SIGNALS:
    void changed(bool hasChanged);
    void configCommitted(const QByteArray& addonName);

protected:
    virtual QList< QWidget* > createItemWidgets(const QModelIndex &index) const;
    virtual void updateItemWidgets(const QList< QWidget* > widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const;

private Q_SLOTS:
    void slotStateChanged(bool state);
    void emitChanged();
    void slotConfigureClicked();

private:
    QFont titleFont(const QFont &baseFont) const;

    QCheckBox *checkBox;
    QPushButton *pushButton;
    QList<FcitxAddon*> moduleProxyList;
    AddonSelector::Private* addonSelector_d;
};

}

#endif
