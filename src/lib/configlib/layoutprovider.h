//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _CONFIGLIB_LAYOUTPROVIDER_H_
#define _CONFIGLIB_LAYOUTPROVIDER_H_

#include "iso639.h"
#include "layoutmodel.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>
#include <fcitxqtdbustypes.h>

class QDBusPendingCallWatcher;

namespace fcitx {
namespace kcm {

class DBusProvider;
class LanguageFilterModel;
class LayoutInfoModel;
class VariantInfoModel;

class LayoutProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(
        fcitx::kcm::LanguageModel *languageModel READ languageModel CONSTANT)
    Q_PROPERTY(LanguageFilterModel *layoutModel READ layoutModel CONSTANT)
    Q_PROPERTY(LanguageFilterModel *variantModel READ variantModel CONSTANT)
public:
    LayoutProvider(DBusProvider *dbus, QObject *parent = nullptr);
    ~LayoutProvider();

    auto languageModel() const { return languageModel_; }
    auto layoutModel() const { return layoutFilterModel_; }
    auto variantModel() const { return variantFilterModel_; }

    Q_INVOKABLE int layoutIndex(const QString &layoutString);
    Q_INVOKABLE int variantIndex(const QString &layoutString);

    Q_INVOKABLE void setVariantInfo(const FcitxQtLayoutInfo &info) const {
        variantModel_->setVariantInfo(info);
    }

    Q_INVOKABLE QString layout(int layoutIdx, int variantIdx) const {
        auto layoutModelIndex = layoutFilterModel_->index(layoutIdx, 0);
        auto variantModelIndex = variantFilterModel_->index(variantIdx, 0);
        if (layoutModelIndex.isValid() && variantModelIndex.isValid()) {
            auto layout = layoutModelIndex.data(Qt::UserRole).toString();
            auto variant = variantModelIndex.data(Qt::UserRole).toString();
            if (layout.isEmpty()) {
                return QString();
            }
            if (variant.isEmpty()) {
                return layout;
            }
            return QString("%1-%2").arg(layout, variant);
        }
        return QString();
    }

    bool loaded() const { return loaded_; }

signals:
    void loadedChanged();

private slots:
    void availabilityChanged();
    void fetchLayoutFinished(QDBusPendingCallWatcher *watcher);

private:
    void setLoaded(bool loaded) {
        if (loaded != loaded_) {
            loaded_ = loaded;
            emit loadedChanged();
        }
    }

    DBusProvider *dbus_;
    bool loaded_ = false;
    LanguageModel *languageModel_;
    LayoutInfoModel *layoutModel_;
    VariantInfoModel *variantModel_;
    LanguageFilterModel *layoutFilterModel_;
    LanguageFilterModel *variantFilterModel_;
    Iso639 iso639_;
};

} // namespace kcm
} // namespace fcitx

#endif // _CONFIGLIB_LAYOUTPROVIDER_H_
