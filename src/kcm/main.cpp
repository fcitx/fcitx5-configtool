/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "main.h"
#include "addonmodel.h"
#include "config.h"
#include "dbusprovider.h"
#include "imconfig.h"
#include "layoutprovider.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QDBusArgument>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QString>
#include <QStringLiteral>
#include <QVariantMap>
#include <QWindowList>
#include <Qt>
#include <QtContainerFwd>
#include <QtGlobal>
#include <fcitx-utils/key.h>
#include <fcitx-utils/misc.h>
#include <fcitx-utils/standardpaths.h>
#include <fcitxqtdbustypes.h>
#include <kquickconfigmodule.h>

namespace fcitx::kcm {

namespace {
QString maybeExtractExternalCommand(const QVariantMap &typeMap,
                                    const QString &baseTypeName) {

    if (!typeMap.contains(baseTypeName)) {
        return {};
    }
    auto list = typeMap.value(baseTypeName).toList();
    if (list.size() != 1) {
        return {};
    }
    auto option = list[0].toMap();
    if (option.value("type").toString() != "External") {
        return {};
    }

    auto properties = option.value("properties").toMap();

    if (properties.contains("LaunchSubConfig") &&
        properties.value("LaunchSubConfig").toString() == "True") {
        return {};
    }

    return properties.value("External").toString();
}
} // namespace

FcitxModule::FcitxModule(QObject *parent, const KPluginMetaData &metaData)
    : KQuickConfigModule(parent, metaData), dbus_(new DBusProvider(this)),
      imConfig_(new IMConfig(dbus_, IMConfig::Flatten, this)),
      layoutProvider_(new LayoutProvider(dbus_, this)),
      addonModel_(new FlatAddonModel(this)),
      addonProxyModel_(new AddonProxyModel(this)) {
    qmlRegisterAnonymousType<FilteredIMModel>("", 1);
    qmlRegisterAnonymousType<IMProxyModel>("", 1);
    qmlRegisterAnonymousType<LanguageModel>("", 1);

    addonProxyModel_->setSourceModel(addonModel_);
    addonProxyModel_->sort(0);

    connect(dbus_, &DBusProvider::availabilityChanged, this,
            &FcitxModule::handleAvailabilityChanged);

    connect(imConfig_, &IMConfig::changed, this,
            [this]() { setNeedsSave(true); });
    connect(addonModel_, &FlatAddonModel::changed, this,
            [this]() { setNeedsSave(true); });
    connect(dbus_, &DBusProvider::canRestartChanged, this,
            &FcitxModule::canRestartChanged);

    handleAvailabilityChanged(dbus_->available());

    xkbContext_.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));

    struct xkb_rule_names rule_names = {"evdev", "pc105", "us", "", ""};

    xkbKeymap_.reset(xkb_keymap_new_from_names(xkbContext_.get(), &rule_names,
                                               XKB_KEYMAP_COMPILE_NO_FLAGS));
    xkbState_.reset(xkb_state_new(xkbKeymap_.get()));

    connect(this, &KQuickConfigModule::pagePushed, this,
            [this](QQuickItem *page) {
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
        if (page && page->property("needsSave").isValid()) {
            QMetaObject::invokeMethod(page, "load", Qt::DirectConnection);
        }
    }
    setNeedsSave(false);
}

void FcitxModule::save() {
    imConfig_->save();
    for (const auto &page : pages_) {
        if (page && page->property("needsSave").isValid()) {
            QMetaObject::invokeMethod(page, "save", Qt::DirectConnection);
        }
    }
}

QVariant decomposeDBusVariant(const QVariant &v) {
    QVariantMap map;
    if (v.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(v);
        argument >> map;
    } else {
        return v;
    }
    for (auto &item : map) {
        item = decomposeDBusVariant(item);
    }
    return map;
}

QVariantList configTypeToVariant(const FcitxQtConfigType &type,
                                 QVariantMap &typeMap,
                                 const QMap<QString, FcitxQtConfigType> &types);

void configOptionToVariant(QVariantList &options,
                           const FcitxQtConfigOption &option,
                           QVariantMap &typeMap,
                           const QMap<QString, FcitxQtConfigType> &types) {
    if (types.contains(option.type())) {
        if (typeMap[option.type()].isNull()) {
            // Fill a dummy value first to avoid infinite recursion if bug
            // happens.
            typeMap[option.type()] = QVariantMap();
            typeMap[option.type()] =
                configTypeToVariant(types[option.type()], typeMap, types);
        }
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
                map["description"] = QStringLiteral("%1 > %2").arg(
                    option.description(), map["description"].toString());
                options.append(map);
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
            decomposeDBusVariant(option.defaultValue().variant());
        map["isSection"] = false;
        QVariantMap propertiesMap;
        for (auto p : fcitx::MakeIterRange(option.properties().keyValueBegin(),
                                           option.properties().keyValueEnd())) {
            propertiesMap[p.first] = decomposeDBusVariant(p.second);
        }
        map["properties"] = propertiesMap;
        options.append(map);
    }
}

QVariantList
configTypeToVariant(const FcitxQtConfigType &type, QVariantMap &typeMap,
                    const QMap<QString, FcitxQtConfigType> &types) {
    QVariantList options;
    for (const auto &option : type.options()) {
        configOptionToVariant(options, option, typeMap, types);
    }
    return options;
}

void FcitxModule::pushConfigPage(const QString &title, const QString &uri) {
    if (!dbus_->controller()) {
        return;
    }
    auto call = dbus_->controller()->GetConfig(uri);
    auto *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, uri, title](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                QDBusPendingReply<QDBusVariant, FcitxQtConfigTypeList> reply =
                    *watcher;
                if (!reply.isValid()) {
                    return;
                }
                auto configTypes = reply.argumentAt<1>();
                if (configTypes.empty()) {
                    return;
                }
                QVariantMap map;
                QVariantMap typeMap;
                QMap<QString, FcitxQtConfigType> types;
                map["uri"] = uri;
                map["rawValue"] =
                    decomposeDBusVariant(reply.argumentAt<0>().variant());
                QString baseTypeName = configTypes[0].name();
                map["typeName"] = baseTypeName;

                // Reserve the place for types.
                for (const auto &configType : configTypes) {
                    types[configType.name()] = configType;
                }
                for (const auto &configType : configTypes) {
                    if (typeMap[configType.name()].isNull()) {
                        typeMap[configType.name()] =
                            configTypeToVariant(configType, typeMap, types);
                    }
                }
                map["typeMap"] = typeMap;
                map["title"] = title;
                if (QString command =
                        maybeExtractExternalCommand(typeMap, baseTypeName);
                    !command.isEmpty()) {
                    launchExternal(command);
                } else {
                    push("ConfigPage.qml", map);
                }
            });
}

void FcitxModule::handleAvailabilityChanged(bool avail) {
    if (avail) {
        loadAddon();
    }
    Q_EMIT availabilityChanged(avail);
}

void FcitxModule::loadAddon() {
    if (!dbus_->controller()) {
        return;
    }
    auto call = dbus_->controller()->GetAddonsV2();
    auto *callwatcher = new QDBusPendingCallWatcher(call, this);
    connect(callwatcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<FcitxQtAddonInfoV2List> addons = *watcher;
                watcher->deleteLater();
                if (addons.isValid()) {
                    addonModel_->setAddons(addons.value());
                    addonProxyModel_->sort(0);
                }
            });
}

void FcitxModule::saveAddon() {
    if (!dbus_->controller()) {
        return;
    }
    FcitxQtAddonStateList list;
    for (const auto &enabled : addonModel_->enabledList()) {
        FcitxQtAddonState state;
        state.setUniqueName(enabled);
        state.setEnabled(true);
        list.append(state);
    }
    for (const auto &disabled : addonModel_->disabledList()) {
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

void FcitxModule::launchExternal(const QString &uri) {
    WId wid = 0;
    if (QGuiApplication::platformName() == "xcb") {
        auto *window = mainUi()->window();
        QWindow *actualWindow = window;
        if (window) {
            auto *renderWindow = QQuickRenderControl::renderWindowFor(window);
            if (renderWindow) {
                actualWindow = renderWindow;
            }
        }
        while (actualWindow && actualWindow->parent()) {
            actualWindow = actualWindow->parent();
        }
        wid = actualWindow ? actualWindow->winId() : 0;
    }
    launchExternalConfig(uri, wid);
}

void FcitxModule::grabKeyboard(QQuickItem *item) {
    item->window()->setKeyboardGrabEnabled(true);
    item->installEventFilter(this);
}

void FcitxModule::ungrabKeyboard(QQuickItem *item) {
    item->window()->setKeyboardGrabEnabled(false);
    item->removeEventFilter(this);
}

bool FcitxModule::eventFilter(QObject *, QEvent *event) {
    if (event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease ||
        event->type() == QEvent::ShortcutOverride) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        key_ = Key(static_cast<KeySym>(keyEvent->nativeVirtualKey()),
                   KeyStates(keyEvent->nativeModifiers()),
                   keyEvent->nativeScanCode());
    }
    return false;
}

QString FcitxModule::eventToString(bool keyCode) {
    Key key;
    if (QGuiApplication::platformName() == "xcb" ||
        QGuiApplication::platformName().startsWith("wayland")) {
        if (keyCode) {
            key = Key::fromKeyCode(key_.code(), key_.states());
        } else {
            key = key_.normalize();
        }
    }

    if (key.isValid()) {
        return QString::fromStdString(key.toString());
    }

    return QString();
}

QString FcitxModule::localizedKeyString(const QString &str) {
    Key key(str.toStdString());
    return QString::fromStdString(key.toString(KeyStringFormat::Localized));
}

void FcitxModule::saveConfig(const QString &uri, const QVariant &value) {
    if (!dbus_->controller()) {
        return;
    }
    auto map = value.value<QVariantMap>();
    QDBusVariant var(QVariant::fromValue(map));
    dbus_->controller()->SetConfig(uri, var);
}

QQuickItem *FcitxModule::pageNeedsSave(int idx) {
    if (auto page = pages_.value(idx)) {
        auto value = page->property("needsSave");
        if (value.isValid() && value.toBool()) {
            return page;
        }
    }
    return nullptr;
}

void FcitxModule::defaults() {
    for (const auto &page : pages_) {
        if (page) {
            QMetaObject::invokeMethod(page, "defaults", Qt::DirectConnection);
        }
    }
}

void FcitxModule::runFcitx() {
    QProcess::startDetached(
        QString::fromStdString(StandardPaths::fcitxPath("bindir", "fcitx5")),
        QStringList());
}

void FcitxModule::fixLayout() {
    const auto &imEntries = imConfig_->imEntries();
    if (imEntries.size() > 0 &&
        imEntries[0].key() !=
            QStringLiteral("keyboard-%0").arg(imConfig_->defaultLayout()) &&
        imEntries[0].key().startsWith("keyboard-")) {
        auto layoutString = imEntries[0].key().mid(9);
        imConfig_->setDefaultLayout(layoutString);
    }
}

void FcitxModule::fixInputMethod() {
    auto imname = QStringLiteral("keyboard-%0").arg(imConfig_->defaultLayout());
    FcitxQtStringKeyValue imEntry;
    int i = 0;
    auto imEntries = imConfig_->imEntries();
    for (; i < imEntries.size(); i++) {
        if (imEntries[i].key() == imname) {
            imEntry = imEntries[i];
            imEntries.removeAt(i);
            break;
        }
    }
    if (i == imEntries.size()) {
        imEntry.setKey(imname);
    }
    imEntries.push_front(imEntry);
    imConfig_->setIMEntries(imEntries);
    imConfig_->emitChanged();
}

} // namespace fcitx::kcm

K_PLUGIN_FACTORY_WITH_JSON(KCMFcitxFactory, "kcm_fcitx5.json",
                           registerPlugin<fcitx::kcm::FcitxModule>();)

#include "main.moc"
