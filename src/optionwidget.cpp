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

#include "optionwidget.h"
#include "keylistwidget.h"
#include "logging.h"
#include <KLocalizedString>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <fcitxqtkeysequencewidget.h>

namespace fcitx {
namespace kcm {

namespace {

QString valueFromVariantMapByPath(const QVariantMap &map,
                                  const QStringList &path, int depth) {
    auto iter = map.find(path[depth]);
    if (iter == map.end()) {
        return QString();
    }
    if (depth + 1 == path.size()) {
        if (iter->canConvert<QString>()) {
            return iter->toString();
        }
    } else {
        QVariantMap map;
        QVariant variant = *iter;
        if (variant.canConvert<QDBusArgument>()) {
            auto argument = qvariant_cast<QDBusArgument>(variant);
            argument >> map;
        }
        if (variant.canConvert<QVariantMap>()) {
            map = iter->toMap();
        }

        if (!map.isEmpty()) {
            return valueFromVariantMapByPath(map, path, depth + 1);
        }
    }
    return QString();
}

QString valueFromVariantMap(const QVariantMap &map, const QString &path) {
    auto pathList = path.split("/");
    if (pathList.empty()) {
        return QString();
    }
    return valueFromVariantMapByPath(map, pathList, 0);
}

void valueToVariantMapByPath(QVariantMap &map, const QStringList &path,
                             const QVariant &value, int depth) {
    if (depth + 1 == path.size()) {
        map[path[depth]] = value;
    } else {
        auto iter = map.find(path[depth]);
        if (iter == map.end()) {
            iter = map.insert(path[depth], QVariantMap());
        }

        if (iter->type() != QVariant::Map) {
            auto oldValue = *iter;
            *iter = QVariantMap({{"", oldValue}});
        }

        auto &nextMap = *static_cast<QVariantMap *>(iter->data());
        valueToVariantMapByPath(nextMap, path, value, depth + 1);
    }
}

void valueToVariantMap(QVariantMap &map, const QString &path,
                       const QVariant &value) {
    auto pathList = path.split("/");
    if (pathList.empty()) {
        return;
    }
    valueToVariantMapByPath(map, pathList, value, 0);
}

class IntegerOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    IntegerOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        spinBox_ = new QSpinBox;
        if (option.properties().contains("IntMax")) {
            auto max = option.properties().value("IntMax");
            if (max.type() == QVariant::String) {
                spinBox_->setMaximum(max.toInt());
            }
        }
        if (option.properties().contains("IntMin")) {
            auto min = option.properties().value("IntMin");
            if (min.type() == QVariant::String) {
                spinBox_->setMinimum(min.toInt());
            }
        }
        defaultValue_ = option.defaultValue().variant().toString().toInt();
        connect(spinBox_, qOverload<int>(&QSpinBox::valueChanged), this,
                &OptionWidget::valueChanged);
        layout->addWidget(spinBox_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = valueFromVariantMap(map, path());
        if (value.isNull()) {
            spinBox_->setValue(defaultValue_);
        }
        spinBox_->setValue(value.toInt());
    }

    void writeValueTo(QVariantMap &map) override {
        valueToVariantMap(map, path(), QString::number(spinBox_->value()));
    }

private:
    QSpinBox *spinBox_;
    int defaultValue_;
};

class StringOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    StringOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                       QWidget *parent)
        : OptionWidget(path, parent) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        lineEdit_ = new QLineEdit;
        connect(lineEdit_, &QLineEdit::textChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(lineEdit_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = valueFromVariantMap(map, path());
        lineEdit_->setText(value);
    }

    void writeValueTo(QVariantMap &map) override {
        valueToVariantMap(map, path(), lineEdit_->text());
    }

private:
    QLineEdit *lineEdit_;
};

class BooleanOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    BooleanOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        checkBox_ = new QCheckBox(this);
        connect(checkBox_, &QCheckBox::clicked, this,
                &OptionWidget::valueChanged);
        checkBox_->setText(option.description());
        layout->addWidget(checkBox_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = valueFromVariantMap(map, path());
        checkBox_->setChecked(value == "True");
    }

    void writeValueTo(QVariantMap &map) override {
        QString value = checkBox_->isChecked() ? "True" : "False";
        valueToVariantMap(map, path(), value);
    }

private:
    QCheckBox *checkBox_;
};

class KeyListOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    KeyListOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                        QWidget *parent)
        : OptionWidget(path, parent) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        keyListWidget_ = new KeyListWidget(this);
        connect(keyListWidget_, &KeyListWidget::keyChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(keyListWidget_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        int i = 0;
        QList<Key> keys;
        while (true) {
            auto value =
                valueFromVariantMap(map, QString("%1/%2").arg(path()).arg(i));
            if (value.isNull()) {
                break;
            }
            keys << Key(value.toUtf8().constData());
            i++;
        }
        keyListWidget_->setKeys(keys);
    }

    void writeValueTo(QVariantMap &map) override {
        auto keys = keyListWidget_->keys();
        int i = 0;
        for (auto &key : keys) {
            auto value = QString::fromUtf8(key.toString().data());
            valueToVariantMap(map, QString("%1/%2").arg(path()).arg(i), value);
            i++;
        }
    }

private:
    KeyListWidget *keyListWidget_;
};

class KeyOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    KeyOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                    QWidget *parent)
        : OptionWidget(path, parent),
          keyWidget_(new FcitxQtKeySequenceWidget(this)) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        connect(keyWidget_, &FcitxQtKeySequenceWidget::keySequenceChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(keyWidget_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        Key key;
        auto value = valueFromVariantMap(map, path());
        key = Key(value.toUtf8().constData());
        keyWidget_->setKeySequence({key});
    }

    void writeValueTo(QVariantMap &map) override {
        auto keys = keyWidget_->keySequence();
        Key key;
        if (keys.size()) {
            key = keys[0];
        }
        auto value = QString::fromUtf8(key.toString().data());
        valueToVariantMap(map, path(), value);
    }

private:
    FcitxQtKeySequenceWidget *keyWidget_;
};

class EnumOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    EnumOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                     QWidget *parent)
        : OptionWidget(path, parent) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        comboBox_ = new QComboBox(this);
        int i = 0;
        while (true) {
            auto value = valueFromVariantMap(option.properties(),
                                             QString("Enum/%1").arg(i));
            if (value.isNull()) {
                break;
            }
            auto text = valueFromVariantMap(option.properties(),
                                            QString("EnumI18n/%1").arg(i));
            if (text.isEmpty()) {
                text = value;
            }
            comboBox_->addItem(text, value);
            i++;
        }
        layout->addWidget(comboBox_);
        setLayout(layout);

        connect(comboBox_, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &OptionWidget::valueChanged);

        defaultValue_ = option.defaultValue().variant().toString().toInt();
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = valueFromVariantMap(map, path());
        auto idx = comboBox_->findData(value);
        if (idx < 0) {
            idx = comboBox_->findData(defaultValue_);
        }
        comboBox_->setCurrentIndex(idx);
    }

    void writeValueTo(QVariantMap &map) override {
        valueToVariantMap(map, path(), comboBox_->currentData().toString());
    }

private:
    QComboBox *comboBox_;
    QString defaultValue_;
};
}

} // namespace kcm
} // namespace fcitx

fcitx::kcm::OptionWidget *
fcitx::kcm::OptionWidget::addWidget(QFormLayout *layout,
                                    const fcitx::FcitxQtConfigOption &option,
                                    const QString &path, QWidget *parent) {
    OptionWidget *widget = nullptr;
    if (option.type() == "Integer") {
        widget = new IntegerOptionWidget(option, path, parent);
        layout->addRow(QString(i18n("%1:")).arg(option.description()), widget);
    } else if (option.type() == "String") {
        widget = new StringOptionWidget(option, path, parent);
        layout->addRow(QString(i18n("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Boolean") {
        widget = new BooleanOptionWidget(option, path, parent);
        layout->addRow(widget);
    } else if (option.type() == "Key") {
        widget = new KeyOptionWidget(option, path, parent);
        layout->addRow(QString(i18n("%1:")).arg(option.description()), widget);
    } else if (option.type() == "List|Key") {
        widget = new KeyListOptionWidget(option, path, parent);
        layout->addRow(QString(i18n("%1:")).arg(option.description()), widget);
    } else if (option.type() == "Enum") {
        widget = new EnumOptionWidget(option, path, parent);
        layout->addRow(QString(i18n("%1:")).arg(option.description()), widget);
    }
    return widget;
}

#include "optionwidget.moc"
