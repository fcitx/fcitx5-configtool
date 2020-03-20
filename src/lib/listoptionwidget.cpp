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
#include "listoptionwidget.h"
#include "varianthelper.h"
#include <QAbstractListModel>
#include <QDebug>

namespace fcitx {
namespace kcm {

class ListOptionWidgetModel : public QAbstractListModel {
public:
    ListOptionWidgetModel(ListOptionWidget *parent)
        : QAbstractListModel(parent), parent_(parent) {}

    QModelIndex
    index(int row, int column = 0,
          const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        if (parent.isValid() || row >= values_.size() || column != 0) {
            return QModelIndex();
        }
        return createIndex(row, column);
    }

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= values_.size()) {
            return QVariant();
        }
        const auto &value = values_.at(index.row());

        switch (role) {
        case Qt::DisplayRole:
            return OptionWidget::prettify(parent_->subOption(), value);
        }
        return QVariant();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }

        return values_.size();
    }

    void readValueFrom(const QVariantMap &map, const QString &path) {
        beginResetModel();
        int i = 0;
        values_.clear();
        while (true) {
            auto value =
                valueFromVariantMap(map, QString("%1%2%3")
                                             .arg(path)
                                             .arg(path.isEmpty() ? "" : "/")
                                             .arg(i));
            if (value.isNull()) {
                break;
            }
            values_ << value;
            i++;
        }
        endResetModel();
    }

    void writeValueTo(QVariantMap &map, const QString &path) {
        int i = 0;
        for (auto &value : values_) {
            valueToVariantMap(map, QString("%1/%2").arg(path).arg(i), value);
            i++;
        }
        if (!i) {
            map[path] = QVariantMap();
        }
    }

    void addItem(QVariant value) {
        beginInsertRows(QModelIndex(), values_.size(), values_.size());
        values_.append(value);
        endInsertRows();
    }

    void editItem(const QModelIndex &index, QVariant value) {
        if (!index.isValid() || index.row() >= values_.size()) {
            return;
        }

        values_[index.row()] = value;
        emit dataChanged(index, index);
    }

    void removeItem(const QModelIndex &index) {
        if (!index.isValid() || index.row() >= values_.size()) {
            return;
        }
        beginRemoveRows(index.parent(), index.row(), index.row());
        values_.removeAt(index.row());
        endRemoveRows();
    }

    void moveUpItem(const QModelIndex &index) {
        if (!index.isValid() || index.row() >= values_.size() ||
            index.row() == 0) {
            return;
        }
        emit layoutAboutToBeChanged();
        if (!beginMoveRows(index.parent(), index.row(), index.row(),
                           index.parent(), index.row() - 1)) {
            return;
        }
        values_.swapItemsAt(index.row() - 1, index.row());
        endMoveRows();
    }

    void moveDownItem(const QModelIndex &index) {
        if (!index.isValid() || index.row() >= values_.size() ||
            index.row() + 1 == values_.size()) {
            return;
        }
        if (!beginMoveRows(index.parent(), index.row(), index.row(),
                           index.parent(), index.row() + 2)) {
            return;
        }
        values_.swapItemsAt(index.row(), index.row() + 1);
        endMoveRows();
    }

private:
    QList<QVariant> values_;
    ListOptionWidget *parent_;
};

ListOptionWidget::ListOptionWidget(const FcitxQtConfigOption &option,
                                   const QString &path, QWidget *parent)
    : OptionWidget(path, parent), model_(new ListOptionWidgetModel(this)),
      subOption_(option) {
    setupUi(this);
    listView->setModel(model_);

    subOption_.setType(option.type().mid(5)); // Remove List|
    auto props = option.properties();
    if (props.contains("ListConstrain")) {
        auto itemConstrain = props.value("ListConstrain").toMap();
        props.remove("ListConstrain");
        for (auto iter = itemConstrain.begin(), end = itemConstrain.end();
             iter != end; ++iter) {
            props[iter.key()] = iter.value();
        }
    }
    subOption_.setProperties(props);
    subOption_.setDefaultValue(QDBusVariant());

    connect(listView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, [this]() { updateButton(); });

    connect(model_, &QAbstractListModel::rowsMoved, this,
            [this]() { updateButton(); });
    connect(addButton, &QAbstractButton::clicked, this, [this]() {
        QVariant result;
        auto ok = OptionWidget::execOptionDialog(subOption_, result);
        if (ok) {
            model_->addItem(result);
        }
    });
    connect(editButton, &QAbstractButton::clicked, this, [this]() {
        QVariant result;
        auto ok = OptionWidget::execOptionDialog(subOption_, result);
        if (ok) {
            model_->editItem(listView->currentIndex(), result);
        }
    });
    connect(removeButton, &QAbstractButton::clicked, this,
            [this]() { model_->removeItem(listView->currentIndex()); });
    connect(moveUpButton, &QAbstractButton::clicked, this,
            [this]() { model_->moveUpItem(listView->currentIndex()); });
    connect(moveDownButton, &QAbstractButton::clicked, this,
            [this]() { model_->moveDownItem(listView->currentIndex()); });

    auto variant = option.defaultValue().variant();
    if (variant.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(variant);
        argument >> defaultValue_;
    }
}

void ListOptionWidget::updateButton() {
    editButton->setEnabled(listView->currentIndex().isValid());
    removeButton->setEnabled(listView->currentIndex().isValid());
    moveUpButton->setEnabled(listView->currentIndex().row() != 0);
    moveDownButton->setEnabled(listView->currentIndex().row() !=
                               model_->rowCount() - 1);
}

void ListOptionWidget::readValueFrom(const QVariantMap &map) {
    model_->readValueFrom(map, path());
}

void ListOptionWidget::writeValueTo(QVariantMap &map) {
    model_->writeValueTo(map, path());
}

void ListOptionWidget::restoreToDefault() {
    model_->readValueFrom(defaultValue_, "");
}

} // namespace kcm
} // namespace fcitx
