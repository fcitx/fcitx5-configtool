/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
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
#include <fcitx-utils/color.h>
#include <fcitx-utils/key.h>
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
    Q_PROPERTY(bool availability READ availability NOTIFY availabilityChanged)
public:
    FcitxModule(QObject *parent, const QVariantList &args);
    virtual ~FcitxModule() override;

    auto imConfig() const { return imConfig_; }
    auto layoutProvider() const { return layoutProvider_; }
    auto addonModel() const { return addonProxyModel_; }
    bool availability() const { return dbus_->available(); }

public Q_SLOTS:
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

    QColor parseColor(const QString &str) {
        Color color;
        try {
            color.setFromString(str.toStdString());
        } catch (...) {
        }
        QColor qcolor;
        qcolor.setRedF(color.redF());
        qcolor.setGreenF(color.greenF());
        qcolor.setBlueF(color.blueF());
        qcolor.setAlphaF(color.alphaF());
        return qcolor;
    }
    QString colorToString(const QColor &color) {
        Color fcitxColor;
        fcitxColor.setRedF(color.redF());
        fcitxColor.setGreenF(color.greenF());
        fcitxColor.setBlueF(color.blueF());
        fcitxColor.setAlphaF(color.alphaF());
        return QString::fromStdString(fcitxColor.toString());
    }

    void grabKeyboard(QQuickItem *item);
    void ungrabKeyboard(QQuickItem *item);

    bool eventFilter(QObject *watched, QEvent *event) override;

    QString eventToString(int key, int modifiers, quint32 nativeScanCode,
                          const QString &text, bool keyCode);
    QString localizedKeyString(const QString &key);

    void runFcitx();
    void fixLayout();
    void fixInputMethod();

Q_SIGNALS:
    void availabilityChanged(bool avail);

private Q_SLOTS:
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
    Key key_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX5_KCM_NG_MAIN_H_
