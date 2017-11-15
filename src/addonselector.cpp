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
#include <QApplication>
#include <QCheckBox>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

// KDE
#include <KCategorizedSortFilterProxyModel>
#include <KCategorizedView>
#include <KCategoryDrawer>
#include <KLocalizedString>
#include <KWidgetItemDelegate>

// Fcitx

// self
#include "addonselector.h"
#include "module.h"
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

#define MARGIN 5

namespace fcitx {
namespace kcm {

enum ExtraRoles { CommentRole = 0x19880209, ConfigurableRole = 0x20080331 };

class AddonModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit AddonModel(AddonSelector *parent);
    virtual ~AddonModel();

    QModelIndex index(int row, int column = 0,
                      const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void setAddons(const FcitxQtAddonInfoList &list) {
        beginResetModel();
        addonEntryList_ = list;
        enabledList_.clear();
        disabledList_.clear();
        endResetModel();
    }

    const auto &enabledList() const { return enabledList_; }
    const auto &disabledList() const { return disabledList_; }

private:
    QSet<QString> enabledList_;
    QSet<QString> disabledList_;
    FcitxQtAddonInfoList addonEntryList_;
    AddonSelector *parent_;
};

class ProxyModel : public KCategorizedSortFilterProxyModel {
    Q_OBJECT

public:
    explicit ProxyModel(AddonSelector *parent)
        : KCategorizedSortFilterProxyModel(parent), parent_(parent) {}

    ~ProxyModel() = default;

protected:
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const override;
    bool subSortLessThan(const QModelIndex &left,
                         const QModelIndex &right) const override;

private:
    AddonSelector *parent_;
};

class AddonDelegate : public KWidgetItemDelegate {
    Q_OBJECT

public:
    AddonDelegate(QAbstractItemView *listView, AddonSelector *parent);
    virtual ~AddonDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

signals:
    void changed();
    void configCommitted(const QByteArray &addonName);

protected:
    virtual QList<QWidget *> createItemWidgets(const QModelIndex &index) const;
    virtual void updateItemWidgets(const QList<QWidget *> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const;

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

AddonModel::AddonModel(AddonSelector *parent)
    : QAbstractListModel(parent), parent_(parent) {}

AddonModel::~AddonModel() {}

QModelIndex AddonModel::index(int row, int column,
                              const QModelIndex &parent) const {
    Q_UNUSED(parent);

    return createIndex(row, column);
}

QString categoryName(int category) {
    if (category >= 5 || category < 0) {
        return QString();
    }

    const char *str[] = {I18N_NOOP("InputMethod"), I18N_NOOP("Frontend"),
                         I18N_NOOP("Loader"), I18N_NOOP("Module"),
                         I18N_NOOP("UI")};

    return i18n(str[category]);
}

QVariant AddonModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= addonEntryList_.size()) {
        return QVariant();
    }
    const auto &addon = addonEntryList_.at(index.row());

    switch (role) {

    case Qt::DisplayRole:
        return addon.name();

    case CommentRole:
        return addon.comment();

    case ConfigurableRole:
        // FIXME
        return false;

    case Qt::CheckStateRole:
        if (disabledList_.contains(addon.uniqueName())) {
            return false;
        } else if (enabledList_.contains(addon.uniqueName())) {
            return true;
        }
        return addon.enabled();

    case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
        return categoryName(addon.category());
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        return addon.category();
    }
    return QVariant();
}

bool AddonModel::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
    if (!index.isValid() || index.row() >= addonEntryList_.size()) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        auto oldData = data(index, role).toBool();
        auto &item = addonEntryList_[index.row()];
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

int AddonModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return addonEntryList_.count();
}

bool ProxyModel::filterAcceptsRow(int sourceRow,
                                  const QModelIndex &sourceParent) const {
    Q_UNUSED(sourceParent)
    const QModelIndex index = sourceModel()->index(sourceRow, 0);
    auto name = sourceModel()->data(index, Qt::DisplayRole).toString();
    auto comment = sourceModel()->data(index, CommentRole).toString();

    QString searchText = parent_->searchText();
    if (!searchText.isEmpty()) {
        return name.contains(searchText, Qt::CaseInsensitive) ||
               comment.contains(searchText, Qt::CaseInsensitive);
    }

    return true;
}

bool ProxyModel::subSortLessThan(const QModelIndex &left,
                                 const QModelIndex &right) const {
    return data(left, Qt::DisplayRole)
               .toString()
               .compare(data(right, Qt::DisplayRole).toString(),
                        Qt::CaseInsensitive) < 0;
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
    int i = 4;
    int j = 1;

    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);

    return QSize(
        fmTitle.width(index.model()->data(index, Qt::DisplayRole).toString()) +
            0 + MARGIN * i + pushButton_->sizeHint().width() * j,
        fmTitle.height() + option.fontMetrics.height() + MARGIN * 2);
}

QList<QWidget *> AddonDelegate::createItemWidgets(const QModelIndex &) const {
    QList<QWidget *> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, &QCheckBox::clicked, this,
            &AddonDelegate::checkBoxClicked);

    QPushButton *configurePushButton = new QPushButton;
    configurePushButton->setIcon(QIcon::fromTheme("configure"));
    connect(configurePushButton, &QPushButton::clicked, this,
            &AddonDelegate::configureClicked);

    setBlockedEventTypes(enabledCheckBox,
                         QList<QEvent::Type>()
                             << QEvent::MouseButtonPress
                             << QEvent::MouseButtonRelease
                             << QEvent::MouseButtonDblClick << QEvent::KeyPress
                             << QEvent::KeyRelease);

    setBlockedEventTypes(configurePushButton,
                         QList<QEvent::Type>()
                             << QEvent::MouseButtonPress
                             << QEvent::MouseButtonRelease
                             << QEvent::MouseButtonDblClick << QEvent::KeyPress
                             << QEvent::KeyRelease);

    widgetList << enabledCheckBox << configurePushButton;

    return widgetList;
}

void AddonDelegate::updateItemWidgets(
    const QList<QWidget *> widgets, const QStyleOptionViewItem &option,
    const QPersistentModelIndex &index) const {
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
    // FIXME
}

AddonSelector::AddonSelector(Module *parent)
    : QWidget(parent), module_(parent), proxyModel_(new ProxyModel(this)),
      addonModel_(new AddonModel(this)) {
    setupUi(this);

    connect(module_, &Module::availabilityChanged, this,
            &AddonSelector::availabilityChanged);

    categoryDrawer_ = new KCategoryDrawer(listView);
    listView->setCategoryDrawer(categoryDrawer_);
    proxyModel_->setCategorizedModel(true);
    proxyModel_->setSourceModel(addonModel_);
    listView->setModel(proxyModel_);

    delegate_ = new AddonDelegate(listView, this);
    listView->setItemDelegate(delegate_);
    listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(lineEdit, &QLineEdit::textChanged, proxyModel_,
            &QSortFilterProxyModel::invalidate);
    connect(advancedCheckbox, &QCheckBox::clicked, proxyModel_,
            &QSortFilterProxyModel::invalidate);
}

AddonSelector::~AddonSelector() { delete delegate_; }

void AddonSelector::load() { availabilityChanged(); }

void AddonSelector::save() {
    if (!module_->controller()) {
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
        module_->controller()->SetAddonsState(list);
    }
}

void AddonSelector::availabilityChanged() {
    if (!module_->controller()) {
        return;
    }

    auto call = module_->controller()->GetAddons();
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
}

QFont AddonDelegate::titleFont(const QFont &baseFont) const {
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}

} // namespace kcm
} // namespace fcitx

#include "addonselector.moc"
