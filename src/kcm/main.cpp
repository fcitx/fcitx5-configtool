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
#include "main.h"
#include "logging.h"
#include "qtkeytrans.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <config.h>
#include <fcitx-utils/misc.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/stringutils.h>

namespace fcitx {

namespace kcm {

FcitxModule::FcitxModule(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args), dbus_(new DBusProvider(this)),
      imConfig_(new IMConfig(dbus_, IMConfig::Flatten, this)),
      layoutProvider_(new LayoutProvider(dbus_, this)),
      addonModel_(new FlatAddonModel(this)),
      addonProxyModel_(new AddonProxyModel(this)) {
    qmlRegisterAnonymousType<FilteredIMModel>("", 1);
    qmlRegisterAnonymousType<IMProxyModel>("", 1);
    qmlRegisterAnonymousType<LanguageModel>("", 1);

    KAboutData *about = new KAboutData(
        "kcm_fcitx5", i18n("Fcitx 5 Configuration Module"), PROJECT_VERSION,
        i18n("Configure Fcitx 5"), KAboutLicense::GPL_V2,
        i18n("Copyright 2017 Xuetian Weng"), QString(), QString(),
        "wengxt@gmail.com");

    about->addAuthor(i18n("Xuetian Weng"), i18n("Author"), "wengxt@gmail.com");
    setAboutData(about);
    addonProxyModel_->setSourceModel(addonModel_);
    addonProxyModel_->sort(0);

    connect(dbus_, &DBusProvider::availabilityChanged, this,
            &FcitxModule::handleAvailabilityChanged);

    connect(imConfig_, &IMConfig::changed, this,
            [this]() { setNeedsSave(true); });
    connect(addonModel_, &FlatAddonModel::changed, this,
            [this]() { setNeedsSave(true); });

    handleAvailabilityChanged(dbus_->available());

    xkbContext_.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));

    struct xkb_rule_names rule_names = {"evdev", "pc105", "us", "", ""};

    xkbKeymap_.reset(xkb_keymap_new_from_names(xkbContext_.get(), &rule_names,
                                               XKB_KEYMAP_COMPILE_NO_FLAGS));
    xkbState_.reset(xkb_state_new(xkbKeymap_.get()));

    connect(this, &ConfigModule::pagePushed, this, [this](QQuickItem *page) {
        pages_[currentIndex() + 1] = page;
        if (page->property("needsSave").isValid()) {
            connect(page, SIGNAL(needsSaveChanged()), this,
                    SLOT(pageNeedsSaveChanged()));
        }
    });
}

FcitxModule::~FcitxModule() {}

void FcitxModule::pageNeedsSaveChanged() { setNeedsSave(true); }

void FcitxModule::load() {
    imConfig_->load();
    for (const auto &page : pages_) {
        if (page) {
            QMetaObject::invokeMethod(page, "load", Qt::DirectConnection);
        }
    }
    setNeedsSave(false);
}

void FcitxModule::save() {
    imConfig_->save();
    for (const auto &page : pages_) {
        if (page) {
            QMetaObject::invokeMethod(page, "save", Qt::DirectConnection);
        }
    }
}

QVariant decompsoeDBusVariant(const QVariant &v) {
    QVariantMap map;
    if (v.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(v);
        argument >> map;
    } else {
        return v;
    }
    for (auto &item : map) {
        item = decompsoeDBusVariant(item);
    }
    return map;
}

void configOptionToVariant(QVariantList &options,
                           const FcitxQtConfigOption &option,
                           const QVariantMap &typeMap) {
    if (typeMap.contains(option.type())) {
        // Expand the nested type.
        // Add a section title.
        QVariantMap section;
        section["isSection"] = true;
        section["description"] = option.description();
        options.append(section);
        auto subOptions = typeMap[option.type()].value<QVariantList>();
        for (const auto &subOption : subOptions) {
            auto map = subOption.value<QVariantMap>();
            if (map["isSection"].toBool()) {
                options.append(subOption);
            } else {
                auto names = map["name"].toStringList();
                names.prepend(option.name());
                map["name"] = names;
                options.append(map);
            }
        }
    } else {
        QVariantMap map;
        map["name"] = QStringList() << option.name();
        map["description"] = option.description();
        map["type"] = option.type();
        map["defaultValue"] =
            decompsoeDBusVariant(option.defaultValue().variant());
        map["isSection"] = false;
        QVariantMap propertiesMap;
        for (auto p : fcitx::MakeIterRange(option.properties().keyValueBegin(),
                                           option.properties().keyValueEnd())) {
            propertiesMap[p.first] = decompsoeDBusVariant(p.second);
        }
        map["properties"] = propertiesMap;
        options.append(map);
    }
}

QVariantList configTypeToVariant(const FcitxQtConfigType &type,
                                 const QVariantMap &typeMap) {
    QVariantList options;
    for (const auto &option : type.options()) {
        configOptionToVariant(options, option, typeMap);
    }
    return options;
}

void FcitxModule::pushConfigPage(const QString &title, const QString &uri) {
    auto call = dbus_->controller()->GetConfig(uri);
    call.waitForFinished();
    if (!call.isValid()) {
        return;
    }
    auto configTypes = call.argumentAt<1>();
    if (configTypes.empty()) {
        return;
    }
    QVariantMap map;
    QVariantMap typeMap;
    map["uri"] = uri;
    map["rawValue"] = decompsoeDBusVariant(call.argumentAt<0>().variant());
    map["typeName"] = configTypes[0].name();

    // We do a revsere order.
    for (const auto &configType :
         MakeIterRange(configTypes.rbegin(), configTypes.rend())) {
        typeMap[configType.name()] = configTypeToVariant(configType, typeMap);
    }
    map["typeMap"] = typeMap;
    map["title"] = title;
    push("ConfigPage.qml", map);
}

} // namespace kcm
} // namespace fcitx

void fcitx::kcm::FcitxModule::handleAvailabilityChanged(bool avail) {
    if (avail) {
        loadAddon();
    }
    emit availabilityChanged(avail);
}

void fcitx::kcm::FcitxModule::loadAddon() {
    auto call = dbus_->controller()->GetAddons();
    auto callwatcher = new QDBusPendingCallWatcher(call, this);
    connect(callwatcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<FcitxQtAddonInfoList> addons = *watcher;
                watcher->deleteLater();
                if (addons.isValid()) {
                    addonModel_->setAddons(addons.value());
                }
            });
}

void fcitx::kcm::FcitxModule::saveAddon() {
    if (!dbus_->controller()) {
        return;
    }
    FcitxQtAddonStateList list;
    for (auto &enabled : addonModel_->enabledList()) {
        FcitxQtAddonState state;
        state.setUniqueName(enabled);
        state.setEnabled(true);
        list.append(state);
    }
    for (auto &disabled : addonModel_->disabledList()) {
        FcitxQtAddonState state;
        state.setUniqueName(disabled);
        state.setEnabled(false);
        list.append(state);
    }
    if (list.size()) {
        dbus_->controller()->SetAddonsState(list);
        loadAddon();
    }
}

void fcitx::kcm::FcitxModule::launchExternal(const QString &uri) {
    if (uri.startsWith("fcitx://config/addon/")) {
        auto wrapperPath =
            stringutils::joinPath(StandardPath::global().fcitxPath("libdir"),
                                  "fcitx5/libexec/fcitx5-qt5-gui-wrapper");
        QStringList args;
        if (QGuiApplication::platformName() == "xcb") {
            auto window = mainUi()->window();
            QWindow *actualWindow = window;
            if (window) {
                auto renderWindow =
                    QQuickRenderControl::renderWindowFor(window);
                if (renderWindow) {
                    actualWindow = renderWindow;
                }
            }
            while (actualWindow && actualWindow->parent()) {
                actualWindow = actualWindow->parent();
            }
            WId wid = actualWindow ? actualWindow->winId() : 0;
            if (wid) {
                args << "-w";
                args << QString::number(wid);
            }
        }
        args << uri;
        qCDebug(KCM_FCITX5) << "Launch: " << wrapperPath.data() << args;
        QProcess::startDetached(wrapperPath.data(), args);
    }
}

void fcitx::kcm::FcitxModule::grabKeyboard(QQuickItem *item) {
    item->window()->setKeyboardGrabEnabled(true);
}

void fcitx::kcm::FcitxModule::ungrabKeyboard(QQuickItem *item) {
    item->window()->setKeyboardGrabEnabled(false);
}

QString fcitx::kcm::FcitxModule::eventToString(int keyQt, int modifiers,
                                               quint32 nativeScanCode,
                                               bool keyCode) {
    int sym;
    unsigned int states;

    modifiers = modifiers & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);
    if (keyQt > 0) {
        if ((keyQt == Qt::Key_Backtab) && (modifiers & Qt::SHIFT)) {
            keyQt = Qt::Key_Tab;
        }
    } else {
        return QString();
    }

    if (fcitx::keyQtToSym(keyQt, Qt::KeyboardModifiers(modifiers), sym,
                          states)) {
        Key key;
        if (keyCode) {
            key = Key::fromKeyCode(nativeScanCode, KeyStates(states));
        } else {
            if (keyQt == Qt::Key_Shift || keyQt == Qt::Key_Super_L ||
                keyQt == Qt::Key_Alt || keyQt == Qt::Key_Control) {
                auto xkbsym =
                    xkb_state_key_get_one_sym(xkbState_.get(), nativeScanCode);
                if (keyQt == Qt::Key_Shift && xkbsym == XKB_KEY_Shift_R) {
                    sym = FcitxKey_Shift_R;
                }
                if (keyQt == Qt::Key_Super_L && xkbsym == XKB_KEY_Super_R) {
                    sym = FcitxKey_Super_R;
                }
                if (keyQt == Qt::Key_Alt && xkbsym == XKB_KEY_Alt_R) {
                    sym = FcitxKey_Alt_R;
                }
                if (keyQt == Qt::Key_Control && xkbsym == XKB_KEY_Control_R) {
                    sym = FcitxKey_Control_R;
                }
            }
            key = Key(static_cast<KeySym>(sym),
                      KeyStates(states) |
                          Key::keySymToStates(static_cast<KeySym>(sym)),
                      nativeScanCode);
        }
        return QString::fromStdString(key.toString());
    }

    return QString();
}

QString fcitx::kcm::FcitxModule::localizedKeyString(const QString &str) {
    Key key(str.toStdString());
    return QString::fromStdString(key.toString(KeyStringFormat::Localized));
}

void fcitx::kcm::FcitxModule::saveConfig(const QString &uri,
                                         const QVariant &value) {
    auto map = value.value<QVariantMap>();
    QDBusVariant var(QVariant::fromValue(map));
    auto call = dbus_->controller()->SetConfig(uri, var);
    call.waitForFinished();
}

QQuickItem *fcitx::kcm::FcitxModule::pageNeedsSave(int idx) {
    if (auto page = pages_.value(idx)) {
        auto value = page->property("needsSave");
        if (value.isValid() && value.toBool()) {
            return page;
        }
    }
    return nullptr;
}

void fcitx::kcm::FcitxModule::defaults() {
    for (const auto &page : pages_) {
        if (page) {
            QMetaObject::invokeMethod(page, "defaults", Qt::DirectConnection);
        }
    }
}

K_PLUGIN_FACTORY_WITH_JSON(KCMFcitxFactory, "kcm_fcitx5.json",
                           registerPlugin<fcitx::kcm::FcitxModule>();)

#include "main.moc"
