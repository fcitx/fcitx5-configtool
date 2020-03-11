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
#include "fontbutton.h"
#include "keylistwidget.h"
#include "listoptionwidget.h"
#include "logging.h"
#include "varianthelper.h"
#include <KLocalizedString>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPointer>
#include <QProcess>
#include <QSpinBox>
#include <QVBoxLayout>
#include <fcitx-utils/standardpath.h>
#include <fcitxqtkeysequencewidget.h>

namespace fcitx {
namespace kcm {

namespace {

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
    StringOptionWidget(const FcitxQtConfigOption &, const QString &path,
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

class FontOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    FontOptionWidget(const FcitxQtConfigOption &, const QString &path,
                     QWidget *parent)
        : OptionWidget(path, parent) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        fontButton_ = new FontButton;
        connect(fontButton_, &FontButton::fontChanged, this,
                &OptionWidget::valueChanged);
        layout->addWidget(fontButton_);
        setLayout(layout);
    }

    void readValueFrom(const QVariantMap &map) override {
        auto value = valueFromVariantMap(map, path());
        fontButton_->setFont(FontButton::parseFont(value));
    }

    void writeValueTo(QVariantMap &map) override {
        valueToVariantMap(map, path(), fontButton_->fontName());
    }

private:
    FontButton *fontButton_;
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

        keyListWidget_->setAllowModifierLess(
            valueFromVariantMap(option.properties(),
                                "ListConstrain/AllowModifierLess") == "True");
        keyListWidget_->setAllowModifierOnly(
            valueFromVariantMap(option.properties(),
                                "ListConstrain/AllowModifierOnly") == "True");
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
        if (keys.empty()) {
            valueToVariantMap(map, path(), QVariantMap());
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

        keyWidget_->setModifierlessAllowed(
            valueFromVariantMap(option.properties(), "AllowModifierLess") ==
            "True");
        keyWidget_->setModifierOnlyAllowed(
            valueFromVariantMap(option.properties(), "AllowModifierOnly") ==
            "True");

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

class ExternalOptionWidget : public OptionWidget {
    Q_OBJECT
public:
    ExternalOptionWidget(const FcitxQtConfigOption &option, const QString &path,
                         QWidget *parent)
        : OptionWidget(path, parent) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);

        button_ = new QToolButton(this);
        button_->setIcon(QIcon::fromTheme("configure"));
        layout->addWidget(button_);
        setLayout(layout);

        uri_ = valueFromVariantMap(option.properties(), "External");

        connect(button_, &QPushButton::clicked, this, [this, parent]() {
            if (uri_.startsWith("fcitx://config/addon/")) {
                auto wrapperPath = stringutils::joinPath(
                    StandardPath::global().fcitxPath("libdir"),
                    "fcitx5/libexec/fcitx5-qt5-gui-wrapper");
                QStringList args;
                if (QGuiApplication::platformName() == "xcb") {
                    auto wid = parent->winId();
                    if (wid) {
                        args << "-w";
                        args << QString::number(wid);
                    }
                }
                args << uri_;
                qCDebug(KCM_FCITX5) << "Launch: " << wrapperPath.data() << args;
                QProcess::startDetached(wrapperPath.data(), args);
            }
        });
    }

    void readValueFrom(const QVariantMap &) override {}
    void writeValueTo(QVariantMap &) override {}

private:
    QToolButton *button_;
    QString uri_;
};
} // namespace

} // namespace kcm
} // namespace fcitx

fcitx::kcm::OptionWidget *
fcitx::kcm::OptionWidget::addWidget(QFormLayout *layout,
                                    const fcitx::FcitxQtConfigOption &option,
                                    const QString &path, QWidget *parent) {
    OptionWidget *widget = nullptr;
    if (option.type() == "Integer") {
        widget = new IntegerOptionWidget(option, path, parent);
        layout->addRow(i18n("%1:", option.description()), widget);
    } else if (option.type() == "String") {
        auto font = valueFromVariantMap(option.properties(), "Font");
        if (font == "True") {
            widget = new FontOptionWidget(option, path, parent);
        } else {
            widget = new StringOptionWidget(option, path, parent);
        }
        layout->addRow(i18n("%1:", option.description()), widget);
    } else if (option.type() == "Boolean") {
        widget = new BooleanOptionWidget(option, path, parent);
        layout->addRow(widget);
    } else if (option.type() == "Key") {
        widget = new KeyOptionWidget(option, path, parent);
        layout->addRow(i18n("%1:", option.description()), widget);
    } else if (option.type() == "List|Key") {
        widget = new KeyListOptionWidget(option, path, parent);
        layout->addRow(i18n("%1:", option.description()), widget);
    } else if (option.type() == "Enum") {
        widget = new EnumOptionWidget(option, path, parent);
        layout->addRow(i18n("%1:", option.description()), widget);
    } else if (option.type().startsWith("List|")) {
        widget = new ListOptionWidget(option, path, parent);
        layout->addRow(i18n("%1:", option.description()), widget);
    } else if (option.type() == "External") {
        widget = new ExternalOptionWidget(option, path, parent);
        layout->addRow(i18n("%1:", option.description()), widget);
    }
    return widget;
}

bool fcitx::kcm::OptionWidget::execOptionDialog(
    const fcitx::FcitxQtConfigOption &option, QVariant &result) {
    QPointer<QDialog> dialog = new QDialog;
    dialog->setWindowIcon(QIcon::fromTheme("fcitx"));
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    QFormLayout *subLayout = new QFormLayout;
    dialogLayout->addLayout(subLayout);
    dialog->setLayout(dialogLayout);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    dialogLayout->addWidget(buttonBox);

    auto optionWidget =
        addWidget(subLayout, option, QString("Value"), dialog.data());
    QVariantMap origin;
    origin["Value"] = result;
    optionWidget->readValueFrom(origin);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    auto ret = dialog->exec();
    if (ret && dialog && optionWidget->isValid()) {
        QVariantMap map;
        optionWidget->writeValueTo(map);
        result = map.value("Value");
        return true;
    }
    return false;
}

QString
fcitx::kcm::OptionWidget::prettify(const fcitx::FcitxQtConfigOption &option,
                                   const QVariant &value) {
    if (option.type() == "Integer") {
        return value.toString();
    } else if (option.type() == "String") {
        return value.toString();
    } else if (option.type() == "Boolean") {
        return value.toString() == "True" ? i18n("Yes") : i18n("No");
    } else if (option.type() == "Key") {
        return value.toString() == "True" ? i18n("Yes") : i18n("No");
    } else if (option.type() == "Enum") {
        QMap<QString, QString> enumMap;
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
            enumMap[value] = text;
            i++;
        }
        return enumMap.value(value.toString());
    } else if (option.type().startsWith("List|")) {

        int i = 0;
        QStringList strs;
        strs.clear();
        auto subOption = option;
        subOption.setType(option.type().mid(5)); // Remove List|
        while (true) {
            auto subValue = valueFromVariant(value, QString(i));
            strs << prettify(subOption, subValue);
            i++;
        }
        return i18n("[%1]", strs.join(" "));
    }
    return QString();
}

#include "optionwidget.moc"
