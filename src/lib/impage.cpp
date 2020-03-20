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
#include "dbusprovider.h"
#include "layoutselector.h"
#include "model.h"
#include "ui_impage.h"
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

IMPage::IMPage(DBusProvider *dbus, QWidget *parent)
    : QWidget(parent), ui_(std::make_unique<Ui::IMPage>()), dbus_(dbus),
      availIMModel_(new AvailIMModel(this)),
      availIMProxyModel_(new IMProxyModel(availIMModel_)),
      currentIMModel_(new CurrentIMModel(this)) {
    ui_->setupUi(this);

    connect(dbus, &DBusProvider::availabilityChanged, this,
            &IMPage::availabilityChanged);
    connect(ui_->inputMethodGroupComboBox, &QComboBox::currentTextChanged, this,
            &IMPage::selectedGroupChanged);
    availabilityChanged();

    connect(this, &IMPage::updateIMList, availIMModel_,
            &AvailIMModel::filterIMEntryList);
    availIMProxyModel_->setSourceModel(availIMModel_);
    connect(this, &IMPage::updateIMList, availIMProxyModel_,
            &IMProxyModel::filterIMEntryList);
    connect(this, &IMPage::updateIMList, currentIMModel_,
            &CurrentIMModel::filterIMEntryList);
    ui_->availIMView->setItemDelegate(new IMDelegate);
    ui_->availIMView->setModel(availIMProxyModel_);
    connect(availIMProxyModel_, &QAbstractItemModel::layoutChanged,
            ui_->availIMView, &QTreeView::expandAll);
    ui_->currentIMView->setModel(currentIMModel_);

    connect(ui_->filterTextEdit, &QLineEdit::textChanged, availIMProxyModel_,
            &IMProxyModel::setFilterText);
    connect(ui_->onlyCurrentLanguageCheckBox, &QCheckBox::toggled,
            availIMProxyModel_, &IMProxyModel::setShowOnlyCurrentLanguage);

    connect(ui_->availIMView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &IMPage::availIMSelectionChanged);
    connect(ui_->currentIMView->selectionModel(),
            &QItemSelectionModel::currentChanged, this,
            &IMPage::currentIMCurrentChanged);
    connect(ui_->addIMButton, &QPushButton::clicked, this, &IMPage::clickAddIM);
    connect(ui_->removeIMButton, &QPushButton::clicked, this,
            &IMPage::clickRemoveIM);
    connect(ui_->moveUpButton, &QPushButton::clicked, this, &IMPage::moveUpIM);
    connect(ui_->moveDownButton, &QPushButton::clicked, this,
            &IMPage::moveDownIM);
    connect(ui_->configureButton, &QPushButton::clicked, this,
            &IMPage::configureIM);
    connect(ui_->addGroupButton, &QPushButton::clicked, this,
            &IMPage::addGroup);
    connect(ui_->deleteGroupButton, &QPushButton::clicked, this,
            &IMPage::deleteGroup);
    // connect(d, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(availIMModel_, &AvailIMModel::select, this, &IMPage::selectAvailIM);
    connect(currentIMModel_, &CurrentIMModel::select, this,
            &IMPage::selectCurrentIM);
    connect(ui_->defaultLayoutButton, &QPushButton::clicked, this,
            &IMPage::selectDefaultLayout);
    connect(ui_->availIMView, &QTreeView::doubleClicked, this,
            &IMPage::doubleClickAvailIM);
    connect(ui_->currentIMView, &QListView::doubleClicked, this,
            &IMPage::doubleClickCurrentIM);
}

IMPage::~IMPage() {}

void IMPage::save() {
    if (!dbus_->controller()) {
        return;
    }
    if (changed_) {
        dbus_->controller()->SetInputMethodGroupInfo(
            ui_->inputMethodGroupComboBox->currentText(), defaultLayout_,
            imEntries_);
        changed_ = false;
    }
}

void IMPage::load() { availabilityChanged(); }

void IMPage::defaults() {}

void IMPage::reloadGroup(const QString &focusName) {
    if (!dbus_->controller()) {
        return;
    }
    auto call = dbus_->controller()->InputMethodGroups();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, focusName](QDBusPendingCallWatcher *watcher) {
                fetchGroupsFinished(watcher, focusName);
            });
}

void IMPage::availabilityChanged() {
    lastGroup_.clear();
    if (!dbus_->controller()) {
        return;
    }
    reloadGroup();
    auto imcall = dbus_->controller()->AvailableInputMethods();
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

    ui_->inputMethodGroupComboBox->clear();
    if (!groups.isError()) {
        ui_->inputMethodGroupComboBox->addItems(groups.value());
    }
    if (!focusName.isEmpty()) {
        ui_->inputMethodGroupComboBox->setCurrentText(focusName);
    }
}

void IMPage::selectedGroupChanged() {
    if (lastGroup_ == ui_->inputMethodGroupComboBox->currentText()) {
        return;
    }
    if (changed_ && !lastGroup_.isEmpty()) {
        if (QMessageBox::No ==
            QMessageBox::question(this, _("Current group changed"),
                                  _("Do you want to change group? Changes to "
                                    "current group will be lost!"))) {
            ui_->inputMethodGroupComboBox->setCurrentText(lastGroup_);
            return;
        }
    }

    if (dbus_->available() &&
        !ui_->inputMethodGroupComboBox->currentText().isEmpty()) {
        auto call = dbus_->controller()->InputMethodGroupInfo(
            ui_->inputMethodGroupComboBox->currentText());
        lastGroup_ = ui_->inputMethodGroupComboBox->currentText();
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
    if (!ui_->availIMView->currentIndex().isValid())
        ui_->addIMButton->setEnabled(false);
    else
        ui_->addIMButton->setEnabled(true);
}

void IMPage::currentIMCurrentChanged() {
    if (!ui_->currentIMView->currentIndex().isValid()) {
        ui_->removeIMButton->setEnabled(false);
        ui_->moveUpButton->setEnabled(false);
        ui_->moveDownButton->setEnabled(false);
        ui_->configureButton->setEnabled(false);
    } else {
        if (ui_->currentIMView->currentIndex().row() == 0)
            ui_->moveUpButton->setEnabled(false);
        else
            ui_->moveUpButton->setEnabled(true);
        if (ui_->currentIMView->currentIndex().row() ==
            currentIMModel_->rowCount() - 1) {
            ui_->moveDownButton->setEnabled(false);
        } else {
            ui_->moveDownButton->setEnabled(true);
        }
        ui_->removeIMButton->setEnabled(true);
        ui_->configureButton->setEnabled(
            currentIMModel_
                ->data(ui_->currentIMView->currentIndex(),
                       FcitxIMConfigurableRole)
                .toBool());
    }
}

void IMPage::selectCurrentIM(const QModelIndex &index) {
    ui_->currentIMView->selectionModel()->setCurrentIndex(
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
    bool ok = false;
    auto result = LayoutSelector::selectLayout(
        this, dbus_, _("Select default layout"), layout, variant, &ok);
    if (!ok) {
        return;
    }
    if (result.second.isEmpty()) {
        defaultLayout_ = result.first;
    } else {
        defaultLayout_ = QString("%0-%1").arg(result.first, result.second);
    }

    auto imname = QString("keyboard-%0").arg(defaultLayout_);
    if (imEntries_.size() == 0 || imEntries_[0].key() != imname) {
        auto result = QMessageBox::question(
            this, _("Change Input method to match layout selection."),
            _("Your currently configured input method does not match your "
              "selected layout, do you want to add the corresponding input "
              "method for the layout?"),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
            QMessageBox::Yes);
        if (result == QMessageBox::Yes) {
            FcitxQtStringKeyValue imEntry;
            int i = 0;
            for (; i < imEntries_.size(); i++) {
                if (imEntries_[i].key() == imname) {
                    imEntry = imEntries_[i];
                    imEntries_.removeAt(i);
                    break;
                }
            }
            if (i == imEntries_.size()) {
                imEntry.setKey(imname);
            }
            imEntries_.push_front(imEntry);
            emit updateIMList(allIMs_, imEntries_, imname);
        }
    }

    emitChanged();
}

void IMPage::emitChanged() {
    changed_ = true;
    emit changed();
}

void IMPage::selectAvailIM(const QModelIndex &index) {
    ui_->availIMView->selectionModel()->setCurrentIndex(
        availIMProxyModel_->mapFromSource(index),
        QItemSelectionModel::ClearAndSelect);
}

void IMPage::clickAddIM() { addIM(ui_->availIMView->currentIndex()); }

void IMPage::clickRemoveIM() { removeIM(ui_->currentIMView->currentIndex()); }

void IMPage::checkDefaultLayout() {
    if (imEntries_.size() > 0 &&
        imEntries_[0].key() != QString("keyboard-%0").arg(defaultLayout_) &&
        imEntries_[0].key().startsWith("keyboard-")) {
        // Remove "keyboard-".
        auto layoutString = imEntries_[0].key().mid(9);
        auto result = QMessageBox::question(
            this, _("Change System layout to match input method selection."),
            _("Your currently configured input method does not match your "
              "layout, do you want to change the layout setting?"),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No),
            QMessageBox::Yes);
        if (result == QMessageBox::Yes) {
            defaultLayout_ = layoutString;
        }
    }
}

void IMPage::addIM(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    const QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    FcitxQtStringKeyValue imEntry;
    imEntry.setKey(uniqueName);
    imEntries_.push_back(imEntry);
    if (imEntries_.size() == 1) {
        checkDefaultLayout();
    }
    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::removeIM(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    const QString uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    imEntries_.removeAt(index.row());
    if (index.row() == 0) {
        checkDefaultLayout();
    }

    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::moveDownIM() {
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        return;
    }
    QModelIndex nextIndex = currentIMModel_->index(curIndex.row() + 1, 0);
    if (!nextIndex.isValid()) {
        return;
    }

    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    imEntries_.swapItemsAt(curIndex.row(), curIndex.row() + 1);
    if (curIndex.row() == 0) {
        checkDefaultLayout();
    }
    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::configureIM() {
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid()) {
        return;
    }
    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    QPointer<QDialog> dialog = ConfigWidget::configDialog(
        this, dbus_, QString("fcitx://config/inputmethod/%1").arg(uniqueName),
        curIndex.data(Qt::DisplayRole).toString());
    dialog->exec();
    delete dialog;
}

void IMPage::moveUpIM() {
    QModelIndex curIndex = ui_->currentIMView->currentIndex();
    if (!curIndex.isValid() || curIndex.row() == 0) {
        return;
    }
    QModelIndex nextIndex = currentIMModel_->index(curIndex.row() - 1, 0);
    if (!nextIndex.isValid()) {
        return;
    }

    const QString uniqueName = curIndex.data(FcitxIMUniqueNameRole).toString();
    imEntries_.swapItemsAt(curIndex.row(), curIndex.row() - 1);
    if (curIndex.row() == 1) {
        checkDefaultLayout();
    }
    emit updateIMList(allIMs_, imEntries_, uniqueName);
    emitChanged();
}

void IMPage::addGroup() {
    bool ok;
    QString name = QInputDialog::getText(this, _("New Group"), _("Group Name:"),
                                         QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty() && dbus_->controller()) {
        auto call = dbus_->controller()->AddInputMethodGroup(name);
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
    if (dbus_->controller()) {
        auto call = dbus_->controller()->RemoveInputMethodGroup(
            ui_->inputMethodGroupComboBox->currentText());
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

} // namespace kcm
} // namespace fcitx

#include "impage.moc"
