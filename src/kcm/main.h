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
#ifndef _KCM_FCITX5_KCM_NG_MAIN_H_
#define _KCM_FCITX5_KCM_NG_MAIN_H_

#include "addonmodel.h"
#include "dbusprovider.h"
#include "font.h"
#include "imconfig.h"
#include "model.h"
#include <KQuickAddons/ConfigModule>
#include <QFont>
#include <QMap>
#include <layoutprovider.h>
#include <xkbcommon/xkbcommon.h>

class BalooSettings;

namespace fcitx {

namespace kcm {

class FcitxModule : public KQuickAddons::ConfigModule {
    Q_OBJECT
    Q_PROPERTY(IMConfig *imConfig READ imConfig CONSTANT)
    Q_PROPERTY(LayoutProvider *layoutProvider READ layoutProvider CONSTANT)
    Q_PROPERTY(AddonProxyModel *addonModel READ addonModel CONSTANT)
public:
    FcitxModule(QObject *parent, const QVariantList &args);
    virtual ~FcitxModule() override;

    auto imConfig() const { return imConfig_; }
    auto layoutProvider() const { return layoutProvider_; }
    auto addonModel() const { return addonProxyModel_; }

public slots:
    void load() override;
    void save() override;
    void defaults() override;
    void loadAddon();
    void saveAddon();

    void pushConfigPage(const QString &title, const QString &uri);
    void launchExternal(const QString &uri);
    void saveConfig(const QString &uri, const QVariant &value);
    QQuickItem *pageNeedsSave(int idx);

    QFont parseFont(const QString &font) {
        return ::fcitx::kcm::parseFont(font);
    }
    QString fontToString(const QFont &font) {
        return ::fcitx::kcm::fontToString(font);
    }

    void grabKeyboard(QQuickItem *item);
    void ungrabKeyboard(QQuickItem *item);

    QString eventToString(int key, int modifiers, quint32 nativeScanCode,
                          bool keyCode);
    QString localizedKeyString(const QString &key);

signals:
    void availabilityChanged(bool avail);

private slots:
    void handleAvailabilityChanged(bool avail);
    void pageNeedsSaveChanged();

private:
    DBusProvider *dbus_;
    IMConfig *imConfig_;
    LayoutProvider *layoutProvider_;
    FlatAddonModel *addonModel_;
    AddonProxyModel *addonProxyModel_;
    QMap<int, QPointer<QQuickItem>> pages_;
    struct XKBStateDeleter {
        void operator()(struct xkb_state *state) const {
            return xkb_state_unref(state);
        }
    };
    struct XKBKeymapDeleter {
        void operator()(struct xkb_keymap *keymap) const {
            return xkb_keymap_unref(keymap);
        }
    };
    struct XKBContextDeleter {
        void operator()(struct xkb_context *context) const {
            return xkb_context_unref(context);
        }
    };
    using ScopedXKBState = std::unique_ptr<struct xkb_state, XKBStateDeleter>;
    using ScopedXKBKeymap =
        std::unique_ptr<struct xkb_keymap, XKBKeymapDeleter>;
    using ScopedXKBContext =
        std::unique_ptr<struct xkb_context, XKBContextDeleter>;
    ScopedXKBState xkbState_;
    ScopedXKBKeymap xkbKeymap_;
    ScopedXKBContext xkbContext_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_KCM_NG_MAIN_H_
