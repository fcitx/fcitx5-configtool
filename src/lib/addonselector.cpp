//
// Copyright (C) 2017~2017 by CSSlayer
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

#include "addonselector.h"
#include "categoryhelper.h"
#include "configwidget.h"
#include "dbusprovider.h"
#include "model.h"
#include "ui_addonselector.h"
#include <KWidgetItemDelegate>
#include <QApplication>
#include <QCheckBox>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <fcitx-utils/i18n.h>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

constexpr int MARGIN = 5;

namespace fcitx {
namespace kcm {

enum ExtraRoles {
    CommentRole = 0x19880209,
    ConfigurableRole,
    AddonNameRole,
    RowTypeRole,
    CategoryRole
};

enum RowType {
    CategoryType,
    AddonType,
};

class AddonModel : public CategorizedItemModel {
    Q_OBJECT

public:
    explicit AddonModel(AddonSelector *parent);
    virtual ~AddonModel();
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    void setAddons(const FcitxQtAddonInfoList &list) {
        beginResetModel();

        addonEntryList_.clear();
        QMap<int, int> addonCategoryMap;
        for (const FcitxQtAddonInfo &addon : list) {
            int idx;
            if (!addonCategoryMap.contains(addon.category())) {
                idx = addonEntryList_.count();
                addonCategoryMap[addon.category()] = idx;
                addonEntryList_.append(QPair<int, FcitxQtAddonInfoList>(
                    addon.category(), FcitxQtAddonInfoList()));
            } else {
                idx = addonCategoryMap[addon.category()];
            }
            addonEntryList_[idx].second.append(addon);
        }
        enabledList_.clear();
        disabledList_.clear();
        endResetModel();
    }

    const auto &enabledList() const { return enabledList_; }
    const auto &disabledList() const { return disabledList_; }

protected:
    int listSize() const override { return addonEntryList_.size(); }
    int subListSize(int idx) const override {
        return addonEntryList_[idx].second.size();
    }
    QVariant dataForItem(const QModelIndex &index, int role) const override;
    QVariant dataForCategory(const QModelIndex &index, int role) const override;

private:
    QSet<QString> enabledList_;
    QSet<QString> disabledList_;
    QList<QPair<int, FcitxQtAddonInfoList>> addonEntryList_;
    AddonSelector *parent_;
};

class ProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit ProxyModel(AddonSelector *parent)
        : QSortFilterProxyModel(parent), parent_(parent) {}

    ~ProxyModel() = default;

protected:
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override;

private:
    bool filterCategory(const QModelIndex &index) const;
    bool filterAddon(const QModelIndex &index) const;

    AddonSelector *parent_;
};

class AddonDelegate : public KWidgetItemDelegate {
    Q_OBJECT

public:
    AddonDelegate(QAbstractItemView *listView, AddonSelector *parent);
    virtual ~AddonDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

signals:
    void changed();
    void configCommitted(const QByteArray &addonName);

protected:
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;
    void updateItemWidgets(const QList<QWidget *> widgets,
                           const QStyleOptionViewItem &option,
                           const QPersistentModelIndex &index) const override;

private slots:
    void checkBoxClicked(bool state);
    void configureClicked();

private:
    QFont titleFont(const QFont &baseFont) const;
    int dependantLayoutValue(int value, int width, int totalWidth) const;

    QCheckBox *checkBox_;
    QPushButton *pushButton_;
    AddonSelector *parent_;
};

int AddonDelegate::dependantLayoutValue(int value, int width,
                                        int totalWidth) const {
    if (itemView()->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

QFont AddonDelegate::titleFont(const QFont &baseFont) const {
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

AddonModel::AddonModel(AddonSelector *parent)
    : fcitx::kcm::CategorizedItemModel(parent), parent_(parent) {}

AddonModel::~AddonModel() {}

QString categoryName(int category) {
    if (category >= 5 || category < 0) {
        return QString();
    }

    const char *str[] = {N_("Input Method"), N_("Frontend"), N_("Loader"),
                         N_("Module"), N_("UI")};

    return _(str[category]);
}
QVariant AddonModel::dataForCategory(const QModelIndex &index, int role) const {
    switch (role) {

    case Qt::DisplayRole:
        return categoryName(addonEntryList_[index.row()].first);

    case CategoryRole:
        return addonEntryList_[index.row()].first;

    case RowTypeRole:
        return CategoryType;

    default:
        return QVariant();
    }
}

QVariant AddonModel::dataForItem(const QModelIndex &index, int role) const {
    const auto &addonList = addonEntryList_[index.parent().row()].second;
    const auto &addon = addonList[index.row()];

    switch (role) {

    case Qt::DisplayRole:
        return addon.name();

    case CommentRole:
        return addon.comment();

    case ConfigurableRole:
        return addon.configurable();

    case AddonNameRole:
        return addon.uniqueName();

    case CategoryRole:
        return addon.category();

    case Qt::CheckStateRole:
        if (disabledList_.contains(addon.uniqueName())) {
            return false;
        } else if (enabledList_.contains(addon.uniqueName())) {
            return true;
        }
        return addon.enabled();

    case RowTypeRole:
        return AddonType;
    }
    return QVariant();
}

bool AddonModel::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
    if (!index.isValid() || !index.parent().isValid() ||
        index.parent().row() >= addonEntryList_.size() ||
        index.parent().column() > 0 || index.column() > 0) {
        return false;
    }

    const auto &addonList = addonEntryList_[index.parent().row()].second;

    if (index.row() >= addonList.size()) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        auto oldData = data(index, role).toBool();
        auto &item = addonList[index.row()];
        auto enabled = value.toBool();
        if (item.enabled() == enabled) {
            enabledList_.remove(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else if (enabled) {
            enabledList_.insert(item.uniqueName());
            disabledList_.remove(item.uniqueName());
        } else {
            enabledList_.remove(item.uniqueName());
            disabledList_.insert(item.uniqueName());
        }
        auto newData = data(index, role).toBool();
        ret = oldData != newData;
    }

    if (ret) {
        emit dataChanged(index, index);
        emit parent_->changed();
    }

    return ret;
}

bool ProxyModel::filterAcceptsRow(int sourceRow,
                                  const QModelIndex &sourceParent) const {
    Q_UNUSED(sourceParent)
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.data(RowTypeRole) == CategoryType) {
        return filterCategory(index);
    }

    return filterAddon(index);
}

bool ProxyModel::filterCategory(const QModelIndex &index) const {
    int childCount = index.model()->rowCount(index);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterAddon(index.model()->index(i, 0, index))) {
            return true;
        }
    }
    return false;
}

bool ProxyModel::filterAddon(const QModelIndex &index) const {
    auto name = index.data(Qt::DisplayRole).toString();
    auto comment = index.data(CommentRole).toString();

    QString searchText = parent_->searchText();
    if (!searchText.isEmpty()) {
        return name.contains(searchText, Qt::CaseInsensitive) ||
               comment.contains(searchText, Qt::CaseInsensitive);
    }

    return true;
}

bool ProxyModel::lessThan(const QModelIndex &left,
                          const QModelIndex &right) const {
    int result =
        left.data(CategoryRole).toInt() - right.data(CategoryRole).toInt();
    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    }

    QString l = left.data(Qt::DisplayRole).toString();
    QString r = right.data(Qt::DisplayRole).toString();
    return QCollator().compare(l, r) < 0;
}

AddonDelegate::AddonDelegate(QAbstractItemView *listView, AddonSelector *parent)
    : KWidgetItemDelegate(listView, parent), checkBox_(new QCheckBox),
      pushButton_(new QPushButton), parent_(parent) {
    pushButton_->setIcon(
        QIcon::fromTheme("configure")); // only for getting size matters
}

AddonDelegate::~AddonDelegate() {
    delete checkBox_;
    delete pushButton_;
}

void AddonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const {
    if (!index.isValid()) {
        return;
    }

    if (index.data(RowTypeRole).toInt() == CategoryType) {
        paintCategoryHeader(painter, option, index);
        return;
    }

    int xOffset = 0;
    if (parent_->showAdvanced())
        xOffset = checkBox_->sizeHint().width();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option,
                                         painter, 0);

    QRect contentsRect(
        dependantLayoutValue(MARGIN * 2 + option.rect.left() + xOffset,
                             option.rect.width() - MARGIN * 2 - xOffset,
                             option.rect.width()),
        MARGIN + option.rect.top(), option.rect.width() - MARGIN * 2 - xOffset,
        option.rect.height() - MARGIN * 2);

    int lessHorizontalSpace = MARGIN * 2 + pushButton_->sizeHint().width();

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.highlightedText().color());

    if (itemView()->layoutDirection() == Qt::RightToLeft)
        contentsRect.translate(lessHorizontalSpace, 0);

    painter->save();

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(
        contentsRect, Qt::AlignLeft | Qt::AlignTop,
        fmTitle.elidedText(
            index.model()->data(index, Qt::DisplayRole).toString(),
            Qt::ElideRight, contentsRect.width()));
    painter->restore();

    painter->drawText(contentsRect, Qt::AlignLeft | Qt::AlignBottom,
                      option.fontMetrics.elidedText(
                          index.model()->data(index, CommentRole).toString(),
                          Qt::ElideRight, contentsRect.width()));
    painter->restore();
}

QSize AddonDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const {

    if (index.data(RowTypeRole).toInt() == CategoryType) {
        return categoryHeaderSizeHint();
    }
    int i = 4;
    int j = 1;

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    return QSize(
        fmTitle.boundingRect(
                   index.model()->data(index, Qt::DisplayRole).toString())
                .width() +
            0 + MARGIN * i + pushButton_->sizeHint().width() * j,
        fmTitle.height() + option.fontMetrics.height() + MARGIN * 2);
}

QList<QWidget *>
AddonDelegate::createItemWidgets(const QModelIndex &index) const {
    if (index.data(RowTypeRole).toInt() == CategoryType) {
        return {};
    }
    QList<QWidget *> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, &QCheckBox::clicked, this,
            &AddonDelegate::checkBoxClicked);

    QPushButton *configurePushButton = new QPushButton;
    configurePushButton->setIcon(QIcon::fromTheme("configure"));
    connect(configurePushButton, &QPushButton::clicked, this,
            &AddonDelegate::configureClicked);

    setBlockedEventTypes(enabledCheckBox, QList<QEvent::Type>()
                                              << QEvent::MouseButtonPress
                                              << QEvent::MouseButtonRelease
                                              << QEvent::MouseButtonDblClick
                                              << QEvent::KeyPress
                                              << QEvent::KeyRelease);

    setBlockedEventTypes(configurePushButton, QList<QEvent::Type>()
                                                  << QEvent::MouseButtonPress
                                                  << QEvent::MouseButtonRelease
                                                  << QEvent::MouseButtonDblClick
                                                  << QEvent::KeyPress
                                                  << QEvent::KeyRelease);

    widgetList << enabledCheckBox << configurePushButton;

    return widgetList;
}

void AddonDelegate::updateItemWidgets(
    const QList<QWidget *> widgets, const QStyleOptionViewItem &option,
    const QPersistentModelIndex &index) const {
    if (index.data(RowTypeRole).toInt() == CategoryType) {
        return;
    }
    QCheckBox *checkBox = static_cast<QCheckBox *>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(dependantLayoutValue(MARGIN, checkBox->sizeHint().width(),
                                        option.rect.width()),
                   option.rect.height() / 2 -
                       checkBox->sizeHint().height() / 2);
    checkBox->setVisible(parent_->showAdvanced());

    QPushButton *configurePushButton = static_cast<QPushButton *>(widgets[1]);
    QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(
        dependantLayoutValue(
            option.rect.width() - MARGIN - configurePushButtonSizeHint.width(),
            configurePushButtonSizeHint.width(), option.rect.width()),
        option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);

    if (!index.isValid() || !index.internalPointer()) {
        checkBox->setVisible(false);
        configurePushButton->setVisible(false);
    } else {
        checkBox->setChecked(
            index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setEnabled(
            index.model()->data(index, Qt::CheckStateRole).toBool());
        configurePushButton->setVisible(
            index.model()->data(index, ConfigurableRole).toBool());
    }
}

void AddonDelegate::checkBoxClicked(bool state) {
    if (!focusedIndex().isValid())
        return;
    const QModelIndex index = focusedIndex();

    const_cast<QAbstractItemModel *>(index.model())
        ->setData(index, state, Qt::CheckStateRole);
}

void AddonDelegate::configureClicked() {
    const QModelIndex index = focusedIndex();
    auto name = index.data(AddonNameRole).toString();
    if (name.isEmpty()) {
        return;
    }
    auto addonName = index.data(Qt::DisplayRole).toString();
    QPointer<QDialog> dialog = ConfigWidget::configDialog(
        parent_, parent_->dbus(), QString("fcitx://config/addon/%1").arg(name),
        addonName);
    dialog->exec();
    delete dialog;
}

AddonSelector::AddonSelector(QWidget *parent, DBusProvider *dbus)
    : QWidget(parent), dbus_(dbus), proxyModel_(new ProxyModel(this)),
      addonModel_(new AddonModel(this)),
      ui_(std::make_unique<Ui::AddonSelector>()) {
    ui_->setupUi(this);

    connect(dbus_, &DBusProvider::availabilityChanged, this,
            &AddonSelector::availabilityChanged);

    proxyModel_->setSourceModel(addonModel_);
    ui_->listView->setModel(proxyModel_);
    connect(proxyModel_, &QAbstractItemModel::layoutChanged, ui_->listView,
            &QTreeView::expandAll);

    delegate_ = new AddonDelegate(ui_->listView, this);
    ui_->listView->setItemDelegate(delegate_);
    ui_->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(ui_->lineEdit, &QLineEdit::textChanged, proxyModel_,
            &QSortFilterProxyModel::invalidate);
    connect(ui_->advancedCheckbox, &QCheckBox::clicked, proxyModel_,
            &QSortFilterProxyModel::invalidate);
}

AddonSelector::~AddonSelector() { delete delegate_; }

void AddonSelector::load() { availabilityChanged(); }

void AddonSelector::save() {
    if (!dbus_->controller()) {
        return;
    }
    FcitxQtAddonStateList list;
    for (auto &enabled : addonModel_->enabledList()) {
        FcitxQtAddonState state;
        state.setUniqueName(enabled);
        state.setEnabled(true);
        list.append(state);
    }
    for (auto &disabled : addonModel_->disabledList()) {
        FcitxQtAddonState state;
        state.setUniqueName(disabled);
        state.setEnabled(false);
        list.append(state);
    }
    if (list.size()) {
        dbus_->controller()->SetAddonsState(list);
        load();
    }
}

void AddonSelector::availabilityChanged() {
    if (!dbus_->controller()) {
        return;
    }

    auto call = dbus_->controller()->GetAddons();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &AddonSelector::fetchAddonFinished);
}

void AddonSelector::fetchAddonFinished(QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    if (watcher->isError()) {
        return;
    }
    QDBusPendingReply<FcitxQtAddonInfoList> reply(*watcher);
    addonModel_->setAddons(reply.value());

    ui_->listView->expandAll();
}

QString AddonSelector::searchText() const { return ui_->lineEdit->text(); }

bool AddonSelector::showAdvanced() const {
    return ui_->advancedCheckbox->isChecked();
}

} // namespace kcm
} // namespace fcitx

#include "addonselector.moc"
