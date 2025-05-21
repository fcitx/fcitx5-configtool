/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "migratorfactory.h"
#include "migrator.h"
#include "migratorfactory_p.h"
#include "migratorfactoryplugin.h"
#include <QDir>
#include <QFileInfo>
#include <QLatin1String>
#include <QLibrary>
#include <QObject>
#include <QPluginLoader>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>
#include <fcitx-utils/standardpaths.h>
#include <filesystem>
#include <memory>
#include <vector>

namespace fcitx {

MigratorFactoryPrivate::MigratorFactoryPrivate(MigratorFactory *factory)
    : QObject(factory), q_ptr(factory) {}

MigratorFactoryPrivate::~MigratorFactoryPrivate() {}

void MigratorFactoryPrivate::scan() {
    for (const auto &staticPlugin : QPluginLoader::staticPlugins()) {
        auto metadata = staticPlugin.metaData();
        if (metadata.value("IID") !=
            QLatin1String(FcitxMigratorFactoryInterface_iid)) {
            continue;
        }
        auto pluginMetadata = metadata.value("MetaData").toObject();
        auto addon = metadata.value("addon").toVariant().toString();
        if (auto *plugin = qobject_cast<FcitxMigratorFactoryPlugin *>(
                staticPlugin.instance())) {
            plugins_.emplace_back(plugin, addon);
        }
    }

    auto libraries = StandardPaths::global().locate(
        StandardPathsType::Addon, "qt" QT_STRINGIFY(QT_VERSION_MAJOR),
        [](const std::filesystem::path &path) {
            QString filePath(QString::fromStdString(path.string()));
            return QLibrary::isLibrary(filePath);
        },
        StandardPathsMode::System);

    for (const auto &[_, filePath] : libraries) {
        auto *loader =
            new QPluginLoader(QString::fromStdString(filePath.string()), this);
        if (loader->metaData().value("IID") !=
            QLatin1String(FcitxMigratorFactoryInterface_iid)) {
            delete loader;
            continue;
        }
        auto metadata = loader->metaData().value("MetaData").toObject();
        auto addon = metadata.value("addon").toVariant().toString();
        if (auto *plugin = qobject_cast<FcitxMigratorFactoryPlugin *>(
                loader->instance())) {
            plugins_.emplace_back(plugin, addon);
        } else {
            delete loader;
        }
    }
}

MigratorFactory::MigratorFactory(QObject *parent)
    : QObject(parent), d_ptr(new MigratorFactoryPrivate(this)) {
    Q_D(MigratorFactory);
    d->scan();
}

MigratorFactory::~MigratorFactory() {}

std::vector<std::unique_ptr<Migrator>>
MigratorFactory::list(const QSet<QString> &addons) const {
    Q_D(const MigratorFactory);
    std::vector<std::unique_ptr<Migrator>> result;
    for (const auto &[plugin, addon] : d->plugins_) {
        if (addon.isEmpty() || addons.contains(addon)) {
            if (auto *migrator = plugin->create()) {
                result.emplace_back(migrator);
            }
        }
    }
    return result;
}

} // namespace fcitx
