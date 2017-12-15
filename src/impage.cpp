//
// Copyright (C) 2012~2017 by CSSlayer
// wengxt@gmail.com
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; see the file COPYING. If not,
// see <http://www.gnu.org/licenses/>.
//

#include "impage.h"
#include "categoryhelper.h"
#include "configwidget.h"
#include "layoutselector.h"
#include "model.h"
#include "module.h"
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QStyledItemDelegate>
#include <fcitxqtcontrollerproxy.h>

namespace fcitx {
namespace kcm {

class IMDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit IMDelegate(QObject *parent = 0);
    virtual ~IMDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

IMDelegate::IMDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

IMDelegate::~IMDelegate() {}

void IMDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const {
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    paintCategoryHeader(painter, option, index);
}

QSize IMDelegate::sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
    if (index.data(FcitxRowTypeRole).toInt() == IMType) {
        return QStyledItemDelegate::sizeHint(option, index);
    } else {
        return categoryHeaderSizeHint();
    }
}

IMPage::IMPage(Module *module)
    : QWidget(module), module_(module), availIMModel_(new AvailIMModel(this)),
      availIMProxyModel_(new IMProxyModel(availIMModel_)),
      currentIMModel_(new CurrentIMModel(this)) {
    setupUi(this);

    connect(module, &Module::availabilityChanged, this,
            &IMPage::availabilityChanged);
    connect(inputMethodGroupComboBox, &QComboBox::currentTextChanged, this,
            &IMPage::selectedGroupChanged);
    availabilityChanged();

    connect(this, &IMPage::updateIMList, availIMModel_,
            &AvailIMModel::filterIMEntryList);
    availIMProxyModel_->setSourceModel(availIMModel_);
    connect(this, &IMPage::updateIMList, availIMProxyModel_,
            &IMProxyModel::filterIMEntryList);
    connect(this, &IMPage::updateIMList, currentIMModel_,
            &CurrentIMModel::filterIMEntryList);
    availIMView->setItemDelegate(new IMDelegate);
    availIMView->setModel(availIMProxyModel_);
    connect(availIMProxyModel_, &QAbstractItemModel::layoutChanged, availIMView,
            &QTreeView::expandAll);
    currentIMView->setModel(currentIMModel_);

    connect(filterTextEdit, &QLineEdit::textChanged, availIMProxyModel_,
            &IMProxyModel::setFilterText);
    connect(onlyCurrentLanguageCheckBox, &QCheckBox::toggled,
            availIMProxyModel_, &IMProxyModel::setShowOnlyCurrentLanguage);

    connect(availIMView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &IMPage::availIMSelectionChanged);
    connect(currentIMView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &IMPage::currentIMCurrentChanged);
    connect(addIMButton, &QPushButton::clicked, this, &IMPage::clickAddIM);
    connect(removeIMButton, &QPushButton::clicked, this,
            &IMPage::clickRemoveIM);
    connect(moveUpButton, &QPushButton::clicked, this, &IMPage::moveUpIM);
    connect(moveDownButton, &QPushButton::clicked, this, &IMPage::moveDownIM);
    connect(configureButton, &QPushButton::clicked, this, &IMPage::configureIM);
    connect(addGroupButton, &QPushButton::clicked, this, &IMPage::addGroup);
    connect(deleteGroupButton, &QPushButton::clicked, this,
            &IMPage::deleteGroup);
    // connect(d, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(availIMModel_, &AvailIMModel::select, this, &IMPage::selectAvailIM);
    connect(currentIMModel_, &CurrentIMModel::select, this,
            &IMPage::selectCurrentIM);
    connect(defaultLayoutButton, &QPushButton::clicked, this,
            &IMPage::selectDefaultLayout);
    connect(availIMView, &QTreeView::doubleClicked, this,
            &IMPage::doubleClickAvailIM);
    connect(currentIMView, &QListView::doubleClicked, this,
            &IMPage::doubleClickCurrentIM);
}

void IMPage::save() {
    if (!module_->controller()) {
        return;
    }
    if (changed_) {
        module_->controller()->SetInputMethodGroupInfo(
            inputMethodGroupComboBox->currentText(), defaultLayout_,
            imEntries_);
        changed_ = false;
    }
}

void IMPage::load() { availabilityChanged(); }

void IMPage::defaults() {}

void IMPage::reloadGroup(const QString &focusName) {
    if (!module_->controller()) {
        return;
    }
    auto call = module_->controller()->InputMethodGroups();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, focusName](QDBusPendingCallWatcher *watcher) {
                fetchGroupsFinished(watcher, focusName);
            });
}

void IMPage::availabilityChanged() {
    lastGroup_.clear();
    if (!module_->controller()) {
        return;
    }
    reloadGroup();
    auto imcall = module_->controller()->AvailableInputMethods();
    auto imcallwatcher = new QDBusPendingCallWatcher(imcall, this);
    connect(imcallwatcher, &QDBusPendingCallWatcher::finished, this,
            &IMPage::fetchInputMethodsFinished);
}

void IMPage::fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<FcitxQtInputMethodEntryList> ims = *watcher;
    watcher->deleteLater();
    if (!ims.isError()) {
        allIMs_ = ims.value();
        emit updateIMList(allIMs_, imEntries_, "");
    }
    return;
}

void IMPage::fetchGroupsFinished(QDBusPendingCallWatcher *watcher,
                                 const QString &focusName) {
    QDBusPendingReply<QStringList> groups = *watcher;
    watcher->deleteLater();

    inputMethodGroupComboBox->clear();
    if (!groups.isError()) {
        inputMethodGroupComboBox->addItems(groups.value());
    }
    if (!focusName.isEmpty()) {
        inputMethodGroupComboBox->setCurrentText(focusName);
    }
}

void IMPage::selectedGroupChanged() {
    if (lastGroup_ == inputMethodGroupComboBox->currentText()) {
        return;
    }
    if (changed_ && !lastGroup_.isEmpty()) {
        if (QMessageBox::No ==
            QMessageBox::question(this, i18n("Current group changed"),
                                  "Do you want to change group? Changes to "
                                  "current group will be lost!")) {
            inputMethodGroupComboBox->setCurrentText(lastGroup_);
            return;
        }
    }

    if (module_->available() &&
        !inputMethodGroupComboBox->currentText().isEmpty()) {
        auto call = module_->controller()->InputMethodGroupInfo(
            inputMethodGroupComboBox->currentText());
        lastGroup_ = inputMethodGroupComboBox->currentText();
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &IMPage::fetchGroupInfoFinished);
    }
}

void IMPage::fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    changed_ = false;
    QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply = *watcher;
    if (!reply.isError()) {
        defaultLayout_ = reply.argumentAt<0>();
        imEntries_ = reply.argumentAt<1>();
    } else {
        defaultLayout_.clear();
        imEntries_.clear();
    }

    emit updateIMList(allIMs_, imEntries_, "");
}

void IMPage::availIMSelectionChanged() {
    if (!availIMView->currentIndex().isValid())
        addIMButton->setEnabled(false);
    else
        addIMButton->setEnabled(true);
}

void IMPage::currentIMCurrentChanged() {
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
        if (currentIMView->currentIndex().row() ==
            currentIMModel_->rowCount() - 1)
            moveDownButton->setEnabled(false);
        else
            moveDownButton->setEnabled(true);
        removeIMButton->setEnabled(true);
        configureButton->setEnabled(
            currentIMModel_
                ->data(currentIMView->currentIndex(), FcitxIMConfigurableRole)
                .toBool());
    }
}

void IMPage::selectCurrentIM(const QModelIndex &index) {
    currentIMView->selectionModel()->setCurrentIndex(
        index, QItemSelectionModel::ClearAndSelect);
}

void IMPage::doubleClickCurrentIM(const QModelIndex &index) { removeIM(index); }

void IMPage::doubleClickAvailIM(const QModelIndex &index) { addIM(index); }

void IMPage::selectDefaultLayout() {
    auto dashPos = defaultLayout_.indexOf("-");
    QString layout, variant;
    if (dashPos >= 0) {
        variant = defaultLayout_.mid(dashPos + 1);
        layout = defaultLayout_.left(dashPos);
    } else {
        layout = defaultLayout_;
    }
    auto result = LayoutSelector::selectLayout(
        this, module_, i18n("Select default layout"), layout, variant);
    if (result.second.isEmpty()) {
        defaultLayout_ = result.first;
    } else {
        defaultLayout_ = QString("%0-%1").arg(result.first, result.second);
    }
    emitChanged();
}

void IMPage::emitChanged() {
    changed_ = true;
    emit changed();
}

void IMPage::selectAvailIM(const QModelIndex &index) {
    availIMView->selectionModel()->setCurrentIndex(
        availIMProxyModel_->mapFromSource(index),
        QItemSelectionModel::ClearAndSelect);
}

void IMPage::clickAddIM() { addIM(availIMView->currentIndex()); }

void IMPage::clickRemoveIM() { removeIM(currentIMView->currentIndex()); }

void IMPage::addIM(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    const QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    FcitxQtStringKeyValue imEntry;
    imEntry.setKey(uniqueName);
    imEntries_.push_back(imEntry);
    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::removeIM(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    const QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    imEntries_.removeAt(index.row());
    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::moveDownIM() {
    QModelIndex curIndex = currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        return;
    }
    QModelIndex nextIndex = currentIMModel_->index(curIndex.row() + 1, 0);
    if (!nextIndex.isValid()) {
        return;
    }

    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    imEntries_.swap(curIndex.row(), curIndex.row() + 1);
    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::configureIM() {
    QModelIndex curIndex = currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        return;
    }
    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    QPointer<QDialog> dialog = ConfigWidget::configDialog(
        this, module_, QString("fcitx://config/inputmethod/%1").arg(uniqueName),
        curIndex.data(Qt::DisplayRole).toString());
    dialog->exec();
    delete dialog;
}

void IMPage::moveUpIM() {
    QModelIndex curIndex = currentIMView->currentIndex();
    if (!curIndex.isValid() || curIndex.row() == 0) {
        return;
    }
    QModelIndex nextIndex = currentIMModel_->index(curIndex.row() - 1, 0);
    if (!nextIndex.isValid()) {
        return;
    }

    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    imEntries_.swap(curIndex.row(), curIndex.row() - 1);
    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::addGroup() {
    bool ok;
    QString name =
        QInputDialog::getText(this, i18n("New Group"), i18n("Group Name:"),
                              QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty() && module_->controller()) {
        auto call = module_->controller()->AddInputMethodGroup(name);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this, name](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();
                    if (!watcher->isError()) {
                        reloadGroup(name);
                    }
                });
    }
}

void IMPage::deleteGroup() {
    if (module_->controller()) {
        auto call = module_->controller()->RemoveInputMethodGroup(
            inputMethodGroupComboBox->currentText());
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();
                    if (!watcher->isError()) {
                        reloadGroup();
                    }
                });
    }
}

} // kcm
}

#include "impage.moc"
