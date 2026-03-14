/*
 * SPDX-FileCopyrightText: 2025~2025 The fcitx5-configtool authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "optionaloptionwidget.h"
#include "configwidget.h"
#include "optionwidget.h"
#include "varianthelper.h"
#include <QCheckBox>
#include <QDBusArgument>
#include <QDBusVariant>
#include <QFormLayout>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <QtContainerFwd>
#include <fcitxqtdbustypes.h>
#include <qboxlayout.h>
#include <qformlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <string_view>

namespace fcitx::kcm {

OptionalOptionWidget::OptionalOptionWidget(const FcitxQtConfigOption &option,
                                           const QString &path, QWidget *parent)
    : OptionWidget(path, parent), subWidget_(nullptr), subOption_(option) {

    subOption_.setType(option.type().mid(
        std::string_view("Optional|").size())); // Remove Optional|
    auto props = option.properties();
    if (props.contains("OptionalConstrain")) {
        auto itemConstrain = props.value("OptionalConstrain").toMap();
        props.remove("OptionalConstrain");
        for (auto iter = itemConstrain.begin(), end = itemConstrain.end();
             iter != end; ++iter) {
            props[iter.key()] = iter.value();
        }
    }
    subOption_.setProperties(props);
    subOption_.setDescription(QString());

    auto *layout = new QFormLayout;

    valueCheckBox_ = new QCheckBox;
    connect(valueCheckBox_, &QCheckBox::checkStateChanged, this,
            &OptionalOptionWidget::updateValueWidget);
    subWidget_ = OptionWidget::addWidget(layout, subOption_, "Value", this,
                                         valueCheckBox_);
    subWidget_->setSkipConfig(true);
    setLayout(layout);

    auto variant = option.defaultValue().variant();
    if (variant.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(variant);
        argument >> defaultValue_;
    }

    updateValueWidget();
}

void OptionalOptionWidget::updateValueWidget() {
    subWidget_->setVisible(valueCheckBox_->isChecked());
}

void OptionalOptionWidget::readValueFrom(const QVariantMap &map) {
    QString valuePath = path() + "/Value";
    auto value = readVariant(map, valuePath);
    valueCheckBox_->setChecked(!value.isNull());

    if (!value.isNull() && subWidget_) {
        QVariantMap origin;
        origin["Value"] = value;
        qDebug() << "Read optional value" << path() << value;
        subWidget_->readValueFrom(origin);
    }
}

void OptionalOptionWidget::writeValueTo(QVariantMap &map) {
    if (valueCheckBox_->isChecked() && subWidget_) {
        QVariantMap valueMap;
        subWidget_->writeValueTo(valueMap);
        auto value = valueMap.value("Value");
        writeVariant(map, path() + "/Value", value);
    } else {
        map[path()] = QVariantMap();
    }
}

void OptionalOptionWidget::restoreToDefault() { readValueFrom(defaultValue_); }

} // namespace fcitx::kcm
