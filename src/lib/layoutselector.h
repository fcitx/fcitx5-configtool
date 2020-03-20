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
#ifndef _KCM_FCITX_LAYOUTWIDGET_H_
#define _KCM_FCITX_LAYOUTWIDGET_H_

#include "iso639.h"
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QWidget>
#include <fcitxqtdbustypes.h>

class QDBusPendingCallWatcher;

namespace Ui {
class LayoutSelector;
}

namespace fcitx {
namespace kcm {

class DBusProvider;
class KeyboardLayoutWidget;
class LanguageFilterModel;
class LayoutInfoModel;
class VariantInfoModel;

class LayoutSelector : public QWidget {
    Q_OBJECT
public:
    LayoutSelector(DBusProvider *dbus, QWidget *parent = nullptr);
    ~LayoutSelector();
    void setLayout(const QString &layout, const QString &variant);

    static QPair<QString, QString>
    selectLayout(QWidget *parent, DBusProvider *dbus, const QString &title,
                 const QString &layout, const QString &variant,
                 bool *ok = nullptr);

    QPair<QString, QString> layout() const;

private slots:
    void availabilityChanged();
    void languageComboBoxChanged();
    void layoutComboBoxChanged();
    void variantComboBoxChanged();
    void fetchLayoutFinished(QDBusPendingCallWatcher *watcher);

private:
    std::unique_ptr<Ui::LayoutSelector> ui_;
    DBusProvider *dbus_;
    KeyboardLayoutWidget *keyboardLayoutWidget_;
    LayoutInfoModel *layoutModel_;
    VariantInfoModel *variantModel_;
    LanguageFilterModel *layoutFilterModel_;
    LanguageFilterModel *variantFilterModel_;
    Iso639 iso639_;

    int loadingCounter_ = 0;
    QString preSelectLayout_;
    QString preSelectVariant_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_LAYOUTWIDGET_H_
