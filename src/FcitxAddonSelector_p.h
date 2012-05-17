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

// KDE
#include <KCategorizedSortFilterProxyModel>
#include <KWidgetItemDelegate>

// Fcitx
#include <fcitx/addon.h>

// self
#include "FcitxAddonSelector.h"

class KCategoryDrawerV3;

class KCategorizedView;

class KLineEdit;

class QPushButton;

class QCheckBox;

namespace Fcitx
{

class FcitxAddonSelector::Private
    : public QObject
{
    Q_OBJECT

public:
    enum ExtraRoles {
        CommentRole       = 0x19880209,
        ConfigurableRole  = 0x20080331
    };
    Private(FcitxAddonSelector* parent);
    virtual ~Private();
    int dependantLayoutValue(int value, int width, int totalWidth) const;

public:

    class AddonModel;

    class ProxyModel;

    class AddonDelegate;
    KLineEdit* lineEdit;
    KCategorizedView* listView;
    KCategoryDrawerV3 *categoryDrawer;
    AddonModel *addonModel;
    ProxyModel *proxyModel;
    AddonDelegate *addonDelegate;
    QCheckBox* advanceCheckbox;

private:
    FcitxAddonSelector* parent;
};

class FcitxAddonSelector::Private::AddonModel : public QAbstractListModel
{
    Q_OBJECT

public:
    AddonModel(FcitxAddonSelector::Private *addonSelector_d, QObject* parent = 0);
    virtual ~AddonModel();

    virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    void addAddon(FcitxAddon* addon);

private:
    QList<FcitxAddon*> addonEntryList;
    FcitxAddonSelector::Private *addonSelector_d;
};

class FcitxAddonSelector::Private::ProxyModel
    : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT

public:
    ProxyModel(FcitxAddonSelector::Private *addonSelector_d, QObject* parent = 0);
    virtual ~ProxyModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
    FcitxAddonSelector::Private* addonSelector_d;
};

class FcitxAddonSelector::Private::AddonDelegate
    : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    AddonDelegate(FcitxAddonSelector::Private *addonSelector_d, QObject* parent = 0);
    virtual ~AddonDelegate();

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

Q_SIGNALS:
    void changed(bool hasChanged);
    void configCommitted(const QByteArray& addonName);

protected:
    virtual QList< QWidget* > createItemWidgets() const;
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
    FcitxAddonSelector::Private* addonSelector_d;
};

}

#endif
