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
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QCollator>

// KDE
#include <KStringHandler>

#include <fcitxqtinputmethodproxy.h>

// self
#include "impage.h"
#include "impage_p.h"
#include "ui_impage.h"
#include "module.h"
#include "global.h"
#include "configwidget.h"
#include "imconfigdialog.h"
#include "erroroverlay.h"

#define MARGIN 0

namespace Fcitx
{

static QString languageName(const QString& langCode)
{
    if (langCode.isEmpty()) {
        return i18n("Unknown");
    }
    else if (langCode == "*")
        return i18n("Multilingual");
    else {
        QLocale locale(langCode);
        if (locale.language() == QLocale::C) {
            return i18n("Unknown");
        }
        QString result = locale.nativeLanguageName();
        if (result.isEmpty()) {
            result = QLocale::languageToString(locale.language());
        }
        if (result.isEmpty()) {
            return i18n("Other");
        }
        return result;
    }
}

IMPage::Private::AvailIMModel::AvailIMModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

IMPage::Private::AvailIMModel::~AvailIMModel()
{
}

QModelIndex IMPage::Private::AvailIMModel::index(int row, int column, const QModelIndex& parent) const
{
    // return language index
    if (!parent.isValid()) {
        if (column > 0 || row >= filteredIMEntryList.count()) {
            return QModelIndex();
        } else {
            return createIndex(row, column, static_cast<quintptr>(0));
        }
    }

    // return im index
    if (parent.column() > 0 || parent.row() >= filteredIMEntryList.count() ||
        row >= filteredIMEntryList[parent.row()].second.size()) {
        return QModelIndex();
    }

    return createIndex(row, column, parent.row() + 1);
}

QVariant IMPage::Private::AvailIMModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (!index.parent().isValid()) {
        if (index.column() > 0 || index.row() >= filteredIMEntryList.count()) {
            return QVariant();
        }
        switch (role) {

        case Qt::DisplayRole:
            return languageName(filteredIMEntryList[index.row()].first);

        case FcitxLanguageRole:
            return filteredIMEntryList[index.row()].first;

        case FcitxIMUniqueNameRole:
            return QString();

        case FcitxRowTypeRole:
            return LanguageType;

        default:
            return QVariant();
        }
    }

    if (index.column() > 0 || index.parent().column() > 0 || index.parent().row() >= filteredIMEntryList.count()) {
        return QVariant();
    }

    const FcitxQtInputMethodItemList& imEntryList = filteredIMEntryList[index.parent().row()].second;

    if (index.row() >= imEntryList.count()) {
        return QVariant();
    }

    const FcitxQtInputMethodItem& imEntry = imEntryList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
        return imEntry.name();

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.langCode();
    }
    return QVariant();
}

QModelIndex IMPage::Private::AvailIMModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    auto row = child.internalId();
    if (row && row - 1 >= filteredIMEntryList.count()) {
        return QModelIndex();
    }

    return createIndex(row - 1, 0, -1);
}

int IMPage::Private::AvailIMModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return filteredIMEntryList.count();
    }

    if (parent.internalId() > 0) {
        return 0;
    }

    if (parent.column() > 0 || parent.row() >= filteredIMEntryList.count()) {
        return 0;
    }

    return filteredIMEntryList[parent.row()].second.count();
}

int IMPage::Private::AvailIMModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

void IMPage::Private::AvailIMModel::filterIMEntryList(const FcitxQtInputMethodItemList& imEntryList, const QString& selection)
{
    beginResetModel();

    QMap<QString, int> languageMap;
    filteredIMEntryList.clear();
    int langRow = -1;
    int imRow = -1;
    Q_FOREACH(const FcitxQtInputMethodItem & im, imEntryList) {
        if (!im.enabled()) {
            int idx;
            if (!languageMap.contains(im.langCode())) {
                idx = filteredIMEntryList.count();
                languageMap[im.langCode()] = idx;
                filteredIMEntryList.append(QPair<QString, FcitxQtInputMethodItemList>(im.langCode(), FcitxQtInputMethodItemList()));
            } else {
                idx = languageMap[im.langCode()];
            }
            filteredIMEntryList[idx].second.append(im);
            if (im.uniqueName() == selection) {
                langRow = idx;
                imRow = filteredIMEntryList[idx].second.count() - 1;
            }
        }
    }
    endResetModel();

    if (imRow >= 0) {
        emit select(index(imRow, 0, index(langRow, 0)));
    }
}

IMPage::Private::IMModel::IMModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

IMPage::Private::IMModel::~IMModel()
{
}

QModelIndex IMPage::Private::IMModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column, (row < filteredIMEntryList.count()) ? (void*) &filteredIMEntryList.at(row) : 0);
}

QVariant IMPage::Private::IMModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= filteredIMEntryList.size()) {
        return QVariant();
    }

    const FcitxQtInputMethodItem& imEntry = filteredIMEntryList.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        return imEntry.name();

    case FcitxRowTypeRole:
        return IMType;

    case FcitxIMUniqueNameRole:
        return imEntry.uniqueName();

    case FcitxLanguageRole:
        return imEntry.langCode();

    default:
        return QVariant();
    }
}

int IMPage::Private::IMModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return filteredIMEntryList.count();
}

void IMPage::Private::IMModel::filterIMEntryList(const FcitxQtInputMethodItemList& imEntryList, const QString& selection)
{
    beginResetModel();

    QSet<QString> languageSet;
    filteredIMEntryList.clear();
    int row = 0, selectionRow = -1;
    Q_FOREACH(const FcitxQtInputMethodItem & im, imEntryList) {
        if (im.enabled()) {
            filteredIMEntryList.append(im);
            if (im.uniqueName() == selection)
                selectionRow = row;
            row ++;
        }
    }
    endResetModel();

    if (selectionRow >= 0) {
        emit select(index(selectionRow, 0));
    }
    else if (row > 0) {
        emit select(index(row - 1, 0));
    }
}

IMPage::Private::IMProxyModel::IMProxyModel(QAbstractItemModel* sourceModel)
    : QSortFilterProxyModel(sourceModel)
     ,m_showOnlyCurrentLanguage(true)
{
}

IMPage::Private::IMProxyModel::~IMProxyModel()
{
}

void
IMPage::Private::IMProxyModel::setFilterText(const QString& text)
{
    if (m_filterText != text) {
        m_filterText = text;
        invalidate();
    }
}

void
IMPage::Private::IMProxyModel::setShowOnlyCurrentLanguage(bool show)
{
    if (m_showOnlyCurrentLanguage != show) {
        m_showOnlyCurrentLanguage = show;
        invalidate();
    }
}

void IMPage::Private::IMProxyModel::filterIMEntryList(const FcitxQtInputMethodItemList& imEntryList, const QString& selection)
{
    m_languageSet.clear();
    Q_FOREACH(const FcitxQtInputMethodItem & im, imEntryList) {
        if (im.enabled()) {
            m_languageSet.insert(im.langCode().left(2));
        }
    }
    invalidate();
}


bool IMPage::Private::IMProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    if (index.data(FcitxRowTypeRole) == LanguageType) {
        return filterLanguage(index);
    }

    return filterIM(index);
}

bool IMPage::Private::IMProxyModel::filterLanguage(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return false;
    }

    int childCount = index.model()->rowCount(index);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterIM(index.model()->index(i, 0, index))) {
            return true;
        }
    }

    return false;
}

bool IMPage::Private::IMProxyModel::filterIM(const QModelIndex& index) const
{
    QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    QString name = index.data(Qt::DisplayRole).toString();
    QString langCode = index.data(FcitxLanguageRole).toString();

    if (uniqueName == "fcitx-keyboard-us")
        return true;

    bool flag = true;
    QString lang = langCode.left(2);

    flag = flag && (m_showOnlyCurrentLanguage
                    ? !lang.isEmpty() && (QLocale().name().startsWith(lang) || m_languageSet.contains(lang))
                    : true );
    if (!m_filterText.isEmpty()) {
        flag = flag &&
               (name.contains(m_filterText, Qt::CaseInsensitive)
               || uniqueName.contains(m_filterText, Qt::CaseInsensitive)
               || langCode.contains(m_filterText, Qt::CaseInsensitive)
               || languageName(langCode).contains(m_filterText, Qt::CaseInsensitive));
    }
    return flag;
}

bool IMPage::Private::IMProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    int result = compareCategories(left, right);
    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    }

    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    return QCollator().compare(l, r) < 0;
}

int IMPage::Private::IMProxyModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    QString l = left.data(FcitxLanguageRole).toString();
    QString r = right.data(FcitxLanguageRole).toString();

    if (l == r)
        return 0;

    if (QLocale().name() == l)
        return -1;

    if (QLocale().name() == r)
        return 1;

    bool fl = QLocale().name().startsWith(l.left(2));
    bool fr = QLocale().name().startsWith(r.left(2));

    if (fl == fr) {
        return l.size() == r.size() ? l.compare(r) : l.size() - r.size();
    }
    return fl ? -1 : 1;
}

IMPage::IMPage(Module* parent): QWidget(parent)
    ,m_ui(new Ui::IMPage)
    ,d(new Private(this))
{
    m_ui->setupUi(this);

    // m_ui->infoIconLabel->setPixmap(QIcon::fromTheme("dialog-information").pixmap(KIconLoader::SizeSmallMedium));
    m_ui->addIMButton->setIcon(QIcon::fromTheme("go-next"));
    m_ui->removeIMButton->setIcon(QIcon::fromTheme("go-previous"));
    m_ui->moveUpButton->setIcon(QIcon::fromTheme("go-up"));
    m_ui->moveDownButton->setIcon(QIcon::fromTheme("go-down"));
    m_ui->configureButton->setIcon(QIcon::fromTheme("configure"));

    d->module = parent;
    d->addIMButton = m_ui->addIMButton;
    d->removeIMButton = m_ui->removeIMButton;
    d->moveUpButton = m_ui->moveUpButton;
    d->moveDownButton = m_ui->moveDownButton;
    d->configureButton = m_ui->configureButton;
    d->availIMView = m_ui->availIMView;
    d->currentIMView = m_ui->currentIMView;
    d->defaultLayoutButton = m_ui->defaultLayoutButton;

    d->onlyCurrentLanguageCheckBox = m_ui->onlyCurrentLanguageCheckBox;
    d->filterTextEdit = m_ui->filterTextEdit;

    d->filterTextEdit->setClearButtonEnabled(true);
    d->filterTextEdit->setPlaceholderText(i18n("Search Input Method"));

    d->availIMModel = new Private::AvailIMModel(d);
    connect(d, SIGNAL(updateIMList(FcitxQtInputMethodItemList,QString)), d->availIMModel, SLOT(filterIMEntryList(FcitxQtInputMethodItemList,QString)));
    d->availIMProxyModel = new Private::IMProxyModel(d->availIMModel);
    d->availIMProxyModel->setSourceModel(d->availIMModel);
    connect(d, SIGNAL(updateIMList(FcitxQtInputMethodItemList,QString)), d->availIMProxyModel, SLOT(filterIMEntryList(FcitxQtInputMethodItemList,QString)));
    d->availIMView->setItemDelegate(new IMDelegate);
    d->availIMView->setModel(d->availIMProxyModel);

    d->currentIMModel = new Private::IMModel(this);
    connect(d, SIGNAL(updateIMList(FcitxQtInputMethodItemList,QString)), d->currentIMModel, SLOT(filterIMEntryList(FcitxQtInputMethodItemList,QString)));
    d->currentIMView->setModel(d->currentIMModel);
    d->currentIMView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(d->filterTextEdit, SIGNAL(textChanged(QString)), this, SLOT(filterTextChanged(QString)));
    connect(d->onlyCurrentLanguageCheckBox, SIGNAL(toggled(bool)), this, SLOT(onlyLanguageChanged(bool)));
    connect(d->availIMView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), d, SLOT(availIMSelectionChanged()));
    connect(d->currentIMView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), d, SLOT(currentIMCurrentChanged()));
    connect(d->addIMButton, SIGNAL(clicked(bool)), d, SLOT(clickAddIM()));
    connect(d->removeIMButton, SIGNAL(clicked(bool)), d, SLOT(clickRemoveIM()));
    connect(d->moveUpButton, SIGNAL(clicked(bool)), d, SLOT(moveUpIM()));
    connect(d->moveDownButton, SIGNAL(clicked(bool)), d, SLOT(moveDownIM()));
    connect(d->configureButton, SIGNAL(clicked(bool)), d, SLOT(configureIM()));
    connect(d, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(d->availIMModel, SIGNAL(select(QModelIndex)), d, SLOT(selectAvailIM(QModelIndex)));
    connect(d->availIMProxyModel, SIGNAL(layoutChanged()), d->availIMView, SLOT(expandAll()));
    connect(d->currentIMModel, SIGNAL(select(QModelIndex)), d, SLOT(selectCurrentIM(QModelIndex)));
    connect(d->defaultLayoutButton, SIGNAL(clicked(bool)), d, SLOT(selectDefaultLayout()));
    connect(d->availIMView, SIGNAL(doubleClicked(QModelIndex)), d, SLOT(doubleClickAvailIM(QModelIndex)));
    connect(d->currentIMView, SIGNAL(doubleClicked(QModelIndex)), d, SLOT(doubleClickCurrentIM(QModelIndex)));
    connect(Global::instance(), SIGNAL(connectStatusChanged(bool)), d, SLOT(onConnectStatusChanged(bool)));

    new ErrorOverlay(this);

    if (Global::instance()->connection()) {
        d->fetchIMList();
    }
}

void IMPage::filterTextChanged(const QString & text)
{
    d->availIMProxyModel->setFilterText(text);
}

void IMPage::onlyLanguageChanged(bool checked)
{
    d->availIMProxyModel->setShowOnlyCurrentLanguage(checked);
}

void IMPage::save()
{
    d->save();
}

void IMPage::load()
{
    d->fetchIMList();
}

void IMPage::defaults()
{
    if (Global::instance()->inputMethodProxy()) {
        Global::instance()->inputMethodProxy()->ResetIMList();
    }
    d->fetchIMList();
}

IMPage::Private::Private(QObject* parent)
    : QObject(parent)
      ,availIMModel(0)
{
}

IMPage::Private::~Private()
{
}

void IMPage::Private::availIMSelectionChanged()
{
    if (!availIMView->currentIndex().isValid())
        addIMButton->setEnabled(false);
    else
        addIMButton->setEnabled(true);
}

void IMPage::Private::currentIMCurrentChanged()
{
    if (!currentIMView->currentIndex().isValid()) {
        removeIMButton->setEnabled(false);
        moveUpButton->setEnabled(false);
        moveDownButton->setEnabled(false);
        configureButton->setEnabled(false);
    } else {
        if (currentIMView->currentIndex().row() == 0)
            moveUpButton->setEnabled(false);
        else
            moveUpButton->setEnabled(true);
        if (currentIMView->currentIndex().row() == currentIMModel->rowCount() - 1)
            moveDownButton->setEnabled(false);
        else
            moveDownButton->setEnabled(true);
        removeIMButton->setEnabled(true);
        configureButton->setEnabled(true);
    }
}

void IMPage::Private::selectCurrentIM(const QModelIndex& index)
{
    currentIMView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

void IMPage::Private::doubleClickCurrentIM(const QModelIndex& index)
{
    removeIM(index);
}

void IMPage::Private::doubleClickAvailIM(const QModelIndex& index)
{
    addIM(index);
}

void IMPage::Private::selectDefaultLayout()
{
    QPointer<QDialog> configDialog(new IMConfigDialog("default", NULL));

    configDialog->exec();
    delete configDialog;
}

void IMPage::Private::selectAvailIM(const QModelIndex& index)
{
    availIMView->selectionModel()->setCurrentIndex(
        availIMProxyModel->mapFromSource(index),
        QItemSelectionModel::ClearAndSelect
    );
}

void IMPage::Private::clickAddIM()
{
    addIM(availIMView->currentIndex());
}

void IMPage::Private::clickRemoveIM()
{
    removeIM(currentIMView->currentIndex());
}

void IMPage::Private::addIM(const QModelIndex& index)
{
    if (index.isValid()) {
        const QString uniqueName =index.data(FcitxIMUniqueNameRole).toString();
        for (int i = 0; i < m_list.size(); i ++) {
            if (uniqueName == m_list[i].uniqueName()) {
                m_list[i].setEnabled(true);
                qStableSort(m_list.begin(), m_list.end());
                emit updateIMList(m_list, uniqueName);
                emit changed();
                break;
            }
        }
    }
}

void IMPage::Private::removeIM(const QModelIndex& index)
{
    if (index.isValid()) {
        const QString uniqueName =index.data(FcitxIMUniqueNameRole).toString();
        for (int i = 0; i < m_list.size(); i ++) {
            if (uniqueName == m_list[i].uniqueName()) {
                m_list[i].setEnabled(false);
                qStableSort(m_list.begin(), m_list.end());
                emit updateIMList(m_list, uniqueName);
                emit changed();
                break;
            }
        }
    }
}

void IMPage::Private::moveDownIM()
{
    QModelIndex curIndex = currentIMView->currentIndex();
    if (curIndex.isValid()) {
        QModelIndex nextIndex = currentIMModel->index(curIndex.row() + 1, 0);

        int i = 0, curIMIdx = -1, nextIMIdx = -1;
        for (i = 0; i < m_list.size(); i ++) {
            if (curIndex.data(FcitxIMUniqueNameRole) == m_list[i].uniqueName())
                curIMIdx = i;

            if (nextIndex.data(FcitxIMUniqueNameRole) == m_list[i].uniqueName())
                nextIMIdx = i;
        }

        if (curIMIdx >= 0 && nextIMIdx >= 0 && curIMIdx != nextIMIdx) {
            m_list.swap(curIMIdx, nextIMIdx);
            qStableSort(m_list.begin(), m_list.end());
            emit updateIMList(m_list, curIndex.data(FcitxIMUniqueNameRole).toString());
            emit changed();
        }
    }
}

void IMPage::Private::configureIM()
{
    QModelIndex curIndex = currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        return;
    }
    if (!Global::instance()->inputMethodProxy()) {
        return;
    }
    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    QDBusPendingReply< QString > result = Global::instance()->inputMethodProxy()->GetIMAddon(uniqueName);
    result.waitForFinished();
    if (result.isValid()) {
        FcitxAddon* addonEntry = module->findAddonByName(result.value());

        QPointer<QDialog> configDialog(new IMConfigDialog(uniqueName, addonEntry));

        configDialog->exec();
        delete configDialog;
    }
}

void IMPage::Private::moveUpIM()
{
    QModelIndex curIndex = currentIMView->currentIndex();
    if (curIndex.isValid() && curIndex.row() > 0) {
        QModelIndex nextIndex = currentIMModel->index(curIndex.row() - 1, 0);

        int i = 0, curIMIdx = -1, nextIMIdx = -1;
        for (i = 0; i < m_list.size(); i ++) {
            if (curIndex.data(FcitxIMUniqueNameRole) == m_list[i].uniqueName())
                curIMIdx = i;

            if (nextIndex.data(FcitxIMUniqueNameRole) == m_list[i].uniqueName())
                nextIMIdx = i;
        }

        if (curIMIdx >= 0 && nextIMIdx >= 0 && curIMIdx != nextIMIdx) {
            m_list.swap(curIMIdx, nextIMIdx);
            qStableSort(m_list.begin(), m_list.end());
            emit updateIMList(m_list, curIndex.data(FcitxIMUniqueNameRole).toString());
            emit changed();
        }
    }
}

void IMPage::Private::save()
{
    if (Global::instance()->inputMethodProxy())
        Global::instance()->inputMethodProxy()->setIMList(m_list);
}

void IMPage::Private::fetchIMList()
{
    if (Global::instance()->inputMethodProxy()) {
        m_list = Global::instance()->inputMethodProxy()->iMList();
        qStableSort(m_list.begin(), m_list.end());
        emit updateIMList(m_list, QString());
    }
}

const FcitxQtInputMethodItemList& IMPage::Private::getIMList()
{
    return m_list;
}

int IMPage::Private::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (availIMView->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

void IMPage::Private::onConnectStatusChanged(bool connected) {
    fetchIMList();
}

IMPage::~IMPage()
{
    delete m_ui;
}



IMDelegate::IMDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

IMDelegate::~IMDelegate()
{
}

const int SPACING = 4;

void IMDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const QString category = index.model()->data(index, Qt::DisplayRole).toString();
    QRect optRect = option.rect;
    optRect.translate(SPACING, SPACING);
    optRect.setWidth(optRect.width() - SPACING * 2);
    optRect.setHeight(optRect.height() - SPACING * 2);
    QFont font(QApplication::font());
    font.setBold(true);
    const QFontMetrics fontMetrics = QFontMetrics(font);

    QColor outlineColor = option.palette.text().color();
    outlineColor.setAlphaF(0.35);

    //BEGIN: top left corner
    {
        painter->save();
        painter->setPen(outlineColor);
        const QPointF topLeft(optRect.topLeft());
        QRectF arc(topLeft, QSizeF(4, 4));
        arc.translate(0.5, 0.5);
        painter->drawArc(arc, 1440, 1440);
        painter->restore();
    }
    //END: top left corner

    //BEGIN: left vertical line
    {
        QPoint start(optRect.topLeft());
        start.ry() += 3;
        QPoint verticalGradBottom(optRect.topLeft());
        verticalGradBottom.ry() += fontMetrics.height() + 5;
        QLinearGradient gradient(start, verticalGradBottom);
        gradient.setColorAt(0, outlineColor);
        gradient.setColorAt(1, Qt::transparent);
        painter->fillRect(QRect(start, QSize(1, fontMetrics.height() + 5)), gradient);
    }
    //END: left vertical line

    //BEGIN: horizontal line
    {
        QPoint start(optRect.topLeft());
        start.rx() += 3;
        QPoint horizontalGradTop(optRect.topLeft());
        horizontalGradTop.rx() += optRect.width() - 6;
        painter->fillRect(QRect(start, QSize(optRect.width() - 6, 1)), outlineColor);
    }
    //END: horizontal line

    //BEGIN: top right corner
    {
        painter->save();
        painter->setPen(outlineColor);
        QPointF topRight(optRect.topRight());
        topRight.rx() -= 4;
        QRectF arc(topRight, QSizeF(4, 4));
        arc.translate(0.5, 0.5);
        painter->drawArc(arc, 0, 1440);
        painter->restore();
    }
    //END: top right corner

    //BEGIN: right vertical line
    {
        QPoint start(optRect.topRight());
        start.ry() += 3;
        QPoint verticalGradBottom(optRect.topRight());
        verticalGradBottom.ry() += fontMetrics.height() + 5;
        QLinearGradient gradient(start, verticalGradBottom);
        gradient.setColorAt(0, outlineColor);
        gradient.setColorAt(1, Qt::transparent);
        painter->fillRect(QRect(start, QSize(1, fontMetrics.height() + 5)), gradient);
    }
    //END: right vertical line

    //BEGIN: text
    {
        QRect textRect(option.rect);
        textRect.setTop(textRect.top() + 7);
        textRect.setLeft(textRect.left() + 7);
        textRect.setHeight(fontMetrics.height());
        textRect.setRight(textRect.right() - 7);

        painter->save();
        painter->setFont(font);
        QColor penColor(option.palette.text().color());
        penColor.setAlphaF(0.6);
        painter->setPen(penColor);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, category);
        painter->restore();
    }
    //END: text

    painter->restore();
}

QSize IMDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        return QStyledItemDelegate::sizeHint(option, index);
    }
    else {
        QFont font(QApplication::font());
        font.setBold(true);
        QFontMetrics fontMetrics(font);
        const int height = fontMetrics.height() + 1 /* 1 pixel-width gradient */
                                                + 11 /* top and bottom separation */ + SPACING;
        return QSize(0, height);
    }
}


}
