/*
 * Copyright (C) 2017~2017 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include "layoutselector.h"
#include "../layout/keyboardlayoutwidget.h"
#include "module.h"
#include <QDBusPendingCallWatcher>
#include <QDialog>
#include <QDialogButtonBox>
#include <QStringListModel>
#include <QX11Info>
#include <fcitxqtcontrollerproxy.h>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

enum { LayoutLanguageRole = 0x3423545, LayoutInfoRole };

class LanguageFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit LanguageFilterModel(QObject *parent)
        : QSortFilterProxyModel(parent) {}

    void setLanguage(const QString &language) {
        language_ = language;
        invalidateFilter();
    }
    bool filterAcceptsRow(int source_row, const QModelIndex &) const override {
        if (language_.isEmpty()) {
            return true;
        }

        auto index = sourceModel()->index(source_row, 0);
        return sourceModel()
            ->data(index, LayoutLanguageRole)
            .toStringList()
            .contains(language_);
    }
    bool lessThan(const QModelIndex &left,
                  const QModelIndex &right) const override {
        return data(left, Qt::DisplayRole).toString() <
               data(right, Qt::DisplayRole).toString();
    }

private:
    QString language_;
};

class LayoutInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    LayoutInfoModel(QObject *parent) : QAbstractListModel(parent) {}

    auto &layoutInfo() const { return layoutInfo_; }
    void setLayoutInfo(FcitxQtLayoutInfoList info) {
        beginResetModel();
        layoutInfo_ = std::move(info);
        endResetModel();
    }

    QModelIndex
    index(int row, int column = 0,
          const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return createIndex(row, column);
    }
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= layoutInfo_.size()) {
            return QVariant();
        }
        const auto &layout = layoutInfo_.at(index.row());

        switch (role) {
        case Qt::DisplayRole:
            return layout.description();
        case Qt::UserRole:
            return layout.layout();
        case LayoutLanguageRole: {
            QStringList languages;
            languages = layout.languages();
            for (const auto &variants : layout.variants()) {
                languages << variants.languages();
            }
            return languages;
        }
        case LayoutInfoRole:
            return QVariant::fromValue(layout);
        }
        return QVariant();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }

        return layoutInfo_.size();
    }

private:
    FcitxQtLayoutInfoList layoutInfo_;
};

class VariantInfoModel : public QAbstractListModel {
    Q_OBJECT
public:
    VariantInfoModel(QObject *parent) : QAbstractListModel(parent) {}

    auto &variantInfo() const { return variantInfo_; }
    void setVariantInfo(const FcitxQtLayoutInfo &info) {
        beginResetModel();
        variantInfo_.clear();
        FcitxQtVariantInfo defaultVariant;
        defaultVariant.setVariant("");
        defaultVariant.setDescription(i18n("Default"));
        defaultVariant.setLanguages(info.languages());
        variantInfo_ << defaultVariant;
        variantInfo_ << info.variants();
        endResetModel();
    }

    QModelIndex
    index(int row, int column = 0,
          const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return createIndex(row, column);
    }
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= variantInfo_.size()) {
            return QVariant();
        }
        const auto &layout = variantInfo_.at(index.row());

        switch (role) {

        case Qt::DisplayRole:
            return layout.description();

        case Qt::UserRole:
            return layout.variant();

        case LayoutLanguageRole:
            return layout.languages();

        default:
            return QVariant();
        }
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }

        return variantInfo_.size();
    }

private:
    FcitxQtVariantInfoList variantInfo_;
};

LayoutSelector::LayoutSelector(Module *module, QWidget *parent)
    : QWidget(parent), module_(module), layoutModel_(new LayoutInfoModel(this)),
      variantModel_(new VariantInfoModel(this)),
      layoutFilterModel_(new LanguageFilterModel(this)),
      variantFilterModel_(new LanguageFilterModel(this)) {
    setupUi(this);
    layoutFilterModel_->setSourceModel(layoutModel_);
    variantFilterModel_->setSourceModel(variantModel_);

    layoutComboBox->setModel(layoutFilterModel_);
    variantComboBox->setModel(variantFilterModel_);

    connect(module_, &Module::availabilityChanged, this,
            &LayoutSelector::availabilityChanged);

    connect(languageComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &LayoutSelector::languageComboBoxChanged);
    connect(layoutComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &LayoutSelector::layoutComboBoxChanged);
    connect(variantComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &LayoutSelector::variantComboBoxChanged);
    if (QX11Info::isPlatformX11()) {
        keyboardLayoutWidget_ = new KeyboardLayoutWidget(this);
        keyboardLayoutWidget_->setMinimumSize(QSize(400, 200));
        keyboardLayoutWidget_->setSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Expanding);
        verticalLayout->addWidget(keyboardLayoutWidget_);
    }
    availabilityChanged();
}

QPair<QString, QString>
LayoutSelector::selectLayout(QWidget *parent, Module *module,
                             const QString &title, const QString &layout,
                             const QString &variant, bool *ok) {
    QPointer<QDialog> dialog(new QDialog(parent));
    auto mainLayout = new QVBoxLayout(dialog);
    dialog->setLayout(mainLayout);
    dialog->setWindowTitle(title);
    auto layoutSelector = new LayoutSelector(module, dialog);
    mainLayout->addWidget(layoutSelector);
    layoutSelector->setLayout(layout, variant);

    auto buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    auto ret = dialog->exec();
    if (ok) {
        *ok = !!ret;
    }
    if (ret) {
        return layoutSelector->layout();
    } else {
        return {};
    }
}

void LayoutSelector::setLayout(const QString &layout, const QString &variant) {
    if (loadingCounter_) {
        preSelectLayout_ = layout;
        preSelectVariant_ = variant;
    } else {
        languageComboBox->setCurrentIndex(0);
        auto &info = layoutModel_->layoutInfo();
        auto iter = std::find_if(info.begin(), info.end(),
                                 [&layout](const FcitxQtLayoutInfo &info) {
                                     return info.layout() == layout;
                                 });
        if (iter != info.end()) {
            auto row = std::distance(info.begin(), iter);
            layoutComboBox->setCurrentIndex(
                layoutFilterModel_->mapFromSource(layoutModel_->index(row))
                    .row());
        }
        if (variant.isEmpty()) {
            variantComboBox->setCurrentIndex(0);
        } else {
            auto &vinfo = variantModel_->variantInfo();
            auto iter =
                std::find_if(vinfo.begin(), vinfo.end(),
                             [&variant](const FcitxQtVariantInfo &info) {
                                 return info.variant() == variant;
                             });
            if (iter != vinfo.end()) {
                auto row = std::distance(vinfo.begin(), iter);
                variantComboBox->setCurrentIndex(
                    variantFilterModel_
                        ->mapFromSource(variantModel_->index(row))
                        .row());
            }
        }
        preSelectLayout_.clear();
        preSelectVariant_.clear();
    }
}

QPair<QString, QString> LayoutSelector::layout() const {
    return {layoutComboBox->currentData().toString(),
            variantComboBox->currentData().toString()};
}

void LayoutSelector::availabilityChanged() {
    if (!module_->controller()) {
        return;
    }

    auto call = module_->controller()->AvailableKeyboardLayouts();
    loadingCounter_ += 1;
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            &LayoutSelector::fetchLayoutFinished);
}

void LayoutSelector::languageComboBoxChanged() {
    layoutFilterModel_->setLanguage(languageComboBox->currentData().toString());
}

void LayoutSelector::layoutComboBoxChanged() {
    variantComboBox->clear();
    if (layoutComboBox->currentIndex() < 0) {
        return;
    }

    variantModel_->setVariantInfo(
        layoutComboBox->currentData(LayoutInfoRole).value<FcitxQtLayoutInfo>());
    variantComboBox->setCurrentIndex(0);
}

void LayoutSelector::fetchLayoutFinished(QDBusPendingCallWatcher *watcher) {
    loadingCounter_ -= 1;
    watcher->deleteLater();
    QDBusPendingReply<FcitxQtLayoutInfoList> reply = *watcher;
    if (reply.isError()) {
        return;
    }
    QSet<QString> languages;
    auto layoutInfo = reply.value();
    for (const auto &layout : layoutInfo) {
        for (const auto &language : layout.languages()) {
            languages << language;
        }
        for (const auto &variant : layout.variants()) {
            for (const auto &language : variant.languages()) {
                languages << language;
            }
        }
    }
    QStringList languageList;
    for (const auto &language : languages) {
        languageList << language;
    }
    languageList.sort();
    languageComboBox->clear();
    languageComboBox->addItem(i18n("Any language"), "");
    for (const auto &language : languageList) {
        QString languageName = module_->iso639().query(language);
        if (languageName.isEmpty()) {
            languageName = language;
        } else {
            languageName = i18n("%1 (%2)", languageName, language);
        }
        languageComboBox->addItem(languageName, language);
    }
    languageComboBox->setCurrentIndex(0);
    layoutModel_->setLayoutInfo(std::move(layoutInfo));
    if (!loadingCounter_ && !preSelectLayout_.isEmpty()) {
        setLayout(preSelectLayout_, preSelectVariant_);
    }
}

void LayoutSelector::variantComboBoxChanged() {
    auto layout = layoutComboBox->currentData().toString();
    auto variant = variantComboBox->currentData().toString();
    if (layout.isEmpty()) {
        keyboardLayoutWidget_->setVisible(false);
    } else {
        keyboardLayoutWidget_->setKeyboardLayout(layout, variant);
        keyboardLayoutWidget_->setVisible(true);
    }
}

} // namespace kcm
} // namespace fcitx

#include "layoutselector.moc"
