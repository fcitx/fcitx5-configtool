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
#include "configwidget.h"
#include "keylistwidget.h"
#include "logging.h"
#include "module.h"
#include "optionwidget.h"
#include "verticalscrollarea.h"
#include <KTitleWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <fcitxqtcontrollerproxy.h>

namespace fcitx {
namespace kcm {

namespace {
QString joinPath(const QString &path, const QString &option) {
    if (path.isEmpty()) {
        return option;
    }
    return QString("%1/%2").arg(path, option);
}
} // namespace

ConfigWidget::ConfigWidget(const QString &uri, Module *module, QWidget *parent)
    : QWidget(parent), uri_(uri), parent_(module),
      mainWidget_(new QWidget(this)) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainWidget_);
    setLayout(layout);
}

void ConfigWidget::requestConfig(bool sync) {
    if (!parent_->controller()) {
        return;
    }
    auto call = parent_->controller()->GetConfig(uri_);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &ConfigWidget::requestConfigFinished);
    if (sync) {
        watcher->waitForFinished();
    }
}

void ConfigWidget::requestConfigFinished(QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    QDBusPendingReply<QDBusVariant, FcitxQtConfigTypeList> reply = *watcher;
    if (reply.isError()) {
        qCWarning(KCM_FCITX5) << reply.error();
        return;
    }

    // qCDebug(KCM_FCITX5) << reply.argumentAt<0>().variant();

    if (!initialized_) {
        auto desc = reply.argumentAt<1>();
        if (!desc.size()) {
            return;
        }

        for (auto &type : desc) {
            desc_[type.name()] = type.options();
        }
        mainType_ = desc[0].name();
        setupWidget(mainWidget_, mainType_, QString());
        initialized_ = true;
    }

    if (initialized_) {
        dontEmitChanged_ = true;
        auto optionWidgets = findChildren<OptionWidget *>();
        auto variant = reply.argumentAt<0>().variant();
        QVariantMap map;
        if (variant.canConvert<QDBusArgument>()) {
            auto argument = qvariant_cast<QDBusArgument>(variant);
            argument >> map;
        }
        for (auto optionWidget : optionWidgets) {
            optionWidget->readValueFrom(map);
        }
        dontEmitChanged_ = false;
    }

    adjustSize();
}

void ConfigWidget::load() { requestConfig(); }

void ConfigWidget::save() {
    if (!parent_->controller()) {
        return;
    }
    QVariantMap map;
    auto optionWidgets = findChildren<OptionWidget *>();
    for (auto optionWidget : optionWidgets) {
        optionWidget->writeValueTo(map);
    }
    QDBusVariant var(QVariant::fromValue(map));
    parent_->controller()->SetConfig(uri_, var);
}

void ConfigWidget::buttonClicked(QDialogButtonBox::StandardButton button) {
    if (button == QDialogButtonBox::RestoreDefaults) {
        auto optionWidgets = findChildren<OptionWidget *>();
        for (auto optionWidget : optionWidgets) {
            optionWidget->restoreToDefault();
        }
    } else if (button == QDialogButtonBox::Ok) {
        save();
    }
}

void ConfigWidget::setupWidget(QWidget *widget, const QString &type,
                               const QString &path) {
    if (!desc_.contains(type)) {
        qCCritical(KCM_FCITX5) << type << " type does not exists.";
    }

    auto layout = new QFormLayout(widget);
    const auto &options = desc_[type];
    for (auto &option : options) {
        addOptionWidget(layout, option, joinPath(path, option.name()));
    }

    widget->setLayout(layout);
}

void ConfigWidget::addOptionWidget(QFormLayout *layout,
                                   const FcitxQtConfigOption &option,
                                   const QString &path) {
    if (auto optionWidget =
            OptionWidget::addWidget(layout, option, path, this)) {
        connect(optionWidget, &OptionWidget::valueChanged, this,
                &ConfigWidget::doChanged);
    } else if (desc_.contains(option.type())) {
        QGroupBox *box = new QGroupBox;
        box->setTitle(option.description());
        QVBoxLayout *innerLayout = new QVBoxLayout;
        QWidget *widget = new QWidget;
        setupWidget(widget, option.type(), path);
        innerLayout->addWidget(widget);
        box->setLayout(innerLayout);
        layout->addRow(box);
    } else {
        qCDebug(KCM_FCITX5) << "Unknown type: " << option.type();
    }
}

QDialog *ConfigWidget::configDialog(QWidget *parent, Module *module,
                                    const QString &uri, const QString &title) {
    QDialog *dialog = new QDialog(parent);
    auto configPage = new ConfigWidget(uri, module, dialog);
    dialog->setWindowIcon(QIcon::fromTheme("fcitx"));
    QVBoxLayout *dialogLayout = new QVBoxLayout;
    dialog->setLayout(dialogLayout);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                             QDialogButtonBox::RestoreDefaults);

    configPage->requestConfig(true);
    auto configPageWrapper = new VerticalScrollArea;
    configPageWrapper->setWidget(configPage);
    if (!title.isEmpty()) {
        auto titleWidget = new KTitleWidget;
        titleWidget->setText(title);
        dialogLayout->addWidget(titleWidget);
    }
    dialogLayout->addWidget(configPageWrapper);
    dialogLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::clicked, configPage,
            [configPage, buttonBox](QAbstractButton *button) {
                configPage->buttonClicked(buttonBox->standardButton(button));
            });
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    return dialog;
}

void ConfigWidget::doChanged() {
    if (dontEmitChanged_) {
        return;
    }
    emit changed();
}

} // namespace kcm
} // namespace fcitx
