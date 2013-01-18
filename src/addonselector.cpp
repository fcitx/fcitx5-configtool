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

// Qt
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QApplication>
#include <QPainter>

// KDE
#include <KPushButton>
#include <KLineEdit>
#include <KCategorizedView>
#include <KCategoryDrawer>
#include <KLocalizedString>
#include <KDebug>

// system
#include <libintl.h>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>

// self
#include "config.h"
#include "addonselector.h"
#include "addonselector_p.h"
#include "configwidget.h"
#include "module.h"
#include "global.h"
#include "subconfigparser.h"

#define MARGIN 5

namespace Fcitx
{

AddonSelector::Private::Private(AddonSelector* parent) :
    QObject(parent),
    listView(0),
    categoryDrawer(0),
    parent(parent)
{
}

AddonSelector::Private::~Private()
{
}

int AddonSelector::Private::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (listView->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

AddonSelector::Private::AddonModel::AddonModel(AddonSelector::Private *addonSelector_d, QObject* parent)
    : QAbstractListModel(parent),
      addonSelector_d(addonSelector_d)
{
}

AddonSelector::Private::AddonModel::~AddonModel()
{
}

QModelIndex AddonSelector::Private::AddonModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column, (row < addonEntryList.count()) ? (void*) addonEntryList.at(row) : 0);
}

QVariant AddonSelector::Private::AddonModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return QVariant();
    }

    FcitxAddon *addonEntry = static_cast<FcitxAddon*>(index.internalPointer());

    switch (role) {

    case Qt::DisplayRole:
        return QString::fromUtf8(addonEntry->generalname);

    case CommentRole:
        return QString::fromUtf8(addonEntry->comment);

    case ConfigurableRole: {
        FcitxConfigFileDesc* cfdesc = Global::instance()->GetConfigDesc(QString::fromUtf8(addonEntry->name).append(".desc"));
        return (bool)(cfdesc != NULL || strlen(addonEntry->subconfig) != 0);
    }

    case Qt::DecorationRole:
        return QVariant();

    case Qt::CheckStateRole:
        return addonEntry->bEnabled;

    case KCategorizedSortFilterProxyModel::CategoryDisplayRole: {
        const FcitxConfigOptionDesc *codesc = FcitxConfigDescGetOptionDesc(addonEntry->config.configFile->fileDesc, "Addon", "Category");
        const FcitxConfigEnum *e = &codesc->configEnum;
        return QString::fromUtf8(dgettext("fcitx", e->enumDesc[addonEntry->category]));
    }
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        return (int) addonEntry->category;

    default:
        return QVariant();
    }
}

bool AddonSelector::Private::AddonModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        FcitxAddon* addon = static_cast<FcitxAddon*>(index.internalPointer());
        addon->bEnabled = value.toBool();
        QString buf = QString("%1.conf").arg(addon->name);
        FILE* fp = FcitxXDGGetFileUserWithPrefix("addon", buf.toLocal8Bit().constData(), "w", NULL);
        if (fp) {
            fprintf(fp, "[Addon]\nEnabled=%s\n", addon->bEnabled ? "True" : "False");
            fclose(fp);
        }

        ret = true;
    }

    if (ret) {
        emit dataChanged(index, index);
    }

    return ret;
}

void AddonSelector::Private::AddonModel::addAddon(FcitxAddon* addon)
{
    beginInsertRows(QModelIndex(), addonEntryList.count(), addonEntryList.count());
    addonEntryList << addon;
    endInsertRows();
}

int AddonSelector::Private::AddonModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return addonEntryList.count();
}

AddonSelector::Private::ProxyModel::ProxyModel(AddonSelector::Private *addonSelector_d, QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
    , addonSelector_d(addonSelector_d)
{
    sort(0);
}

AddonSelector::Private::ProxyModel::~ProxyModel()
{
}

bool AddonSelector::Private::ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)
    const QModelIndex index = sourceModel()->index(sourceRow, 0);
    const FcitxAddon* addonInfo = static_cast<FcitxAddon*>(index.internalPointer());
    if ((!addonInfo->bEnabled || addonInfo->advance) && !addonSelector_d->advanceCheckbox->isChecked()) {
        return false;
    }
    if (addonInfo->category == AC_FRONTEND && !addonSelector_d->advanceCheckbox->isChecked()) {
        return false;
    }

    if (!addonSelector_d->lineEdit->text().isEmpty()) {
        return QString(addonInfo->name).contains(addonSelector_d->lineEdit->text(), Qt::CaseInsensitive)
               || QString::fromUtf8(addonInfo->generalname).contains(addonSelector_d->lineEdit->text(), Qt::CaseInsensitive)
               || QString::fromUtf8(addonInfo->comment).contains(addonSelector_d->lineEdit->text(), Qt::CaseInsensitive);
    }

    return true;
}

bool AddonSelector::Private::ProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    FcitxAddon* l = static_cast<FcitxAddon*>(left.internalPointer());
    FcitxAddon* r = static_cast<FcitxAddon*>(right.internalPointer());
    return QString::fromUtf8(l->name).compare(QString::fromUtf8(r->name), Qt::CaseInsensitive) < 0;
}

AddonSelector::Private::AddonDelegate::AddonDelegate(AddonSelector::Private *addonSelector_d, QObject *parent)
    : KWidgetItemDelegate(addonSelector_d->listView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new KPushButton)
    , addonSelector_d(addonSelector_d)
{
    pushButton->setIcon(KIcon("configure"));       // only for getting size matters
}

AddonSelector::Private::AddonDelegate::~AddonDelegate()
{
    delete checkBox;
    delete pushButton;
}

void AddonSelector::Private::AddonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    int xOffset = 0;
    if (addonSelector_d->advanceCheckbox->isChecked())
        xOffset = checkBox->sizeHint().width();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    QRect contentsRect(addonSelector_d->dependantLayoutValue(MARGIN * 2 + option.rect.left() + xOffset, option.rect.width() - MARGIN * 2 - xOffset, option.rect.width()), MARGIN + option.rect.top(), option.rect.width() - MARGIN * 2 - xOffset, option.rect.height() - MARGIN * 2);

    int lessHorizontalSpace = MARGIN * 2 + pushButton->sizeHint().width();

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.highlightedText().color());

    if (addonSelector_d->listView->layoutDirection() == Qt::RightToLeft)
        contentsRect.translate(lessHorizontalSpace, 0);

    painter->save();

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignTop, fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();

    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom, option.fontMetrics.elidedText(index.model()->data(index, CommentRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();
}

QSize AddonSelector::Private::AddonDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i = 4;
    int j = 1;

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    return QSize(fmTitle.width(index.model()->data(index, Qt::DisplayRole).toString()) +
                 0 + MARGIN * i + pushButton->sizeHint().width() * j,
                 fmTitle.height() + option.fontMetrics.height() + MARGIN * 2);
}

QList<QWidget*> AddonSelector::Private::AddonDelegate::createItemWidgets() const
{
    QList<QWidget*> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(slotStateChanged(bool)));
    connect(enabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(emitChanged()));

    KPushButton *configurePushButton = new KPushButton;
    configurePushButton->setIcon(KIcon("configure"));
    connect(configurePushButton, SIGNAL(clicked(bool)), this, SLOT(slotConfigureClicked()));

    setBlockedEventTypes(enabledCheckBox, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                         << QEvent::KeyPress << QEvent::KeyRelease);

    setBlockedEventTypes(configurePushButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick
                         << QEvent::KeyPress << QEvent::KeyRelease);

    widgetList << enabledCheckBox << configurePushButton;

    return widgetList;
}

void AddonSelector::Private::AddonDelegate::updateItemWidgets(const QList<QWidget*> widgets,
        const QStyleOptionViewItem &option,
        const QPersistentModelIndex &index) const
{
    QCheckBox *checkBox = static_cast<QCheckBox*>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(addonSelector_d->dependantLayoutValue(MARGIN, checkBox->sizeHint().width(), option.rect.width()), option.rect.height() / 2 - checkBox->sizeHint().height() / 2);
    checkBox->setVisible(addonSelector_d->advanceCheckbox->isChecked());

    KPushButton *configurePushButton = static_cast<KPushButton*>(widgets[1]);
    QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(addonSelector_d->dependantLayoutValue(option.rect.width() - MARGIN - configurePushButtonSizeHint.width(), configurePushButtonSizeHint.width(), option.rect.width()), option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);

    if (!index.isValid() || !index.internalPointer()) {
        checkBox->setVisible(false);
        configurePushButton->setVisible(false);
    } else {
        checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setEnabled(index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setVisible(index.model()->data(index, ConfigurableRole).toBool());
    }
}

void AddonSelector::Private::AddonDelegate::slotStateChanged(bool state)
{
    if (!focusedIndex().isValid())
        return;
    const QModelIndex index = focusedIndex();

    const_cast<QAbstractItemModel*>(index.model())->setData(index, state, Qt::CheckStateRole);
}

void AddonSelector::Private::AddonDelegate::emitChanged()
{
    emit changed(true);
}

void AddonSelector::Private::AddonDelegate::slotConfigureClicked()
{
    const QModelIndex index = focusedIndex();

    FcitxAddon* addonEntry = static_cast<FcitxAddon*>(index.internalPointer());
    QPointer<KDialog> configDialog(ConfigWidget::configDialog(addonSelector_d->parent->parent, addonEntry));
    if (configDialog.isNull())
        return;
    configDialog->exec();
    delete configDialog;
}

void AddonSelector::load()
{

}

void AddonSelector::save()
{

}

AddonSelector::AddonSelector(Module* parent) :
    QWidget(parent),
    d(new Private(this)),
    parent(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);

    d->lineEdit = new KLineEdit(this);
    d->lineEdit->setClearButtonShown(true);
    d->lineEdit->setClickMessage(i18n("Search Addons"));
    d->listView = new KCategorizedView(this);
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);
    d->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    d->categoryDrawer = new KCategoryDrawerV3(d->listView);
    d->listView->setCategoryDrawer(d->categoryDrawer);
    d->advanceCheckbox = new QCheckBox(this);
    d->advanceCheckbox->setText(i18n("Show &Advance option"));
    d->advanceCheckbox->setChecked(false);

    d->proxyModel = new Private::ProxyModel(d, this);
    d->addonModel = new Private::AddonModel(d, this);
    d->proxyModel->setCategorizedModel(true);
    d->proxyModel->setSourceModel(d->addonModel);
    d->listView->setModel(d->proxyModel);
    d->listView->setAlternatingBlockColors(true);

    Private::AddonDelegate *addonDelegate = new Private::AddonDelegate(d, this);
    d->listView->setItemDelegate(addonDelegate);

    d->listView->setMouseTracking(true);
    d->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(d->lineEdit, SIGNAL(textChanged(QString)), d->proxyModel, SLOT(invalidate()));
    connect(d->advanceCheckbox, SIGNAL(clicked(bool)), d->proxyModel, SLOT(invalidate()));
    connect(addonDelegate, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    connect(addonDelegate, SIGNAL(configCommitted(QByteArray)), this, SIGNAL(configCommitted(QByteArray)));

    layout->addWidget(d->lineEdit);
    layout->addWidget(d->listView);
    layout->addWidget(d->advanceCheckbox);
    this->setLayout(layout);
}

void AddonSelector::addAddon(FcitxAddon* fcitxAddon)
{
    d->addonModel->addAddon(fcitxAddon);
    d->proxyModel->sort(0);
}


QFont AddonSelector::Private::AddonDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

AddonSelector::~AddonSelector()
{
    delete d->listView->itemDelegate();
    delete d->listView; // depends on some other things in d, make sure this dies first.
    delete d;
}

}
